#include "movement_system.h"
#include "ui_system.h"
#include "entity_system.h"
#include "collisions.h"
#include "easing.h"
#include "game_constants.h"
#include "macros.h"

using namespace Gamma;

internal void resolveNewPositionFromCollision(const Collision& collision, Object& player) {
  player.position = collision.point + collision.plane.normal * PLAYER_RADIUS;
}

internal void resolveSingleCollision(GmContext* context, GameState& state, const Collision& collision, float dt) {
  auto& player = get_player();
  auto& plane = collision.plane;

  resolveNewPositionFromCollision(collision, player);

  if (plane.nDotU > 0.7f) {
    // If the collision plane normal points sufficiently upward,
    // treat it as a solid ground collision and stop falling

    if (state.isOnSolidGround) {
      // If we're already on solid ground, we don't
      // need to re-resolve the player position, so
      // set it back to the previous frame position.
      // Otherwise, the player gradually slides down
      // angled slopes, even without user input.
      player.position = state.previousPlayerPosition;
    }

    // Ensure that the current pitch is accurate when standing on
    // upward-facing planes.
    //
    // We recalculate player yaw/pitch based on motion, but when
    // jumping straight up, and then landing on a surface and stopping
    // immediately, pitch is not correctly reset based on the relationship
    // between the player direction and plane normal. Thus, we have to
    // perform this correction here.
    {
      // @hack invert() accounts for the z-inversion of the player model.
      // Perhaps this should be fixed!
      float dot = Vec3f::dot(player.rotation.getDirection().xz().invert(), plane.normal);

      state.currentPitch = Gm_Lerpf(state.currentPitch, dot, 10.f * dt);
    }

    state.velocity.y = 0.f;
    state.lastSolidGroundPosition = player.position;
    state.lastTimeOnSolidGround = get_scene_time();
  } else if (plane.nDotU < -0.7f && state.velocity.y > 0.f) {
    // When hitting a downward-facing plane from the underside,
    // immediately negate any upward velocity.
    state.velocity.y = 0.f;
  } else {
    // Allow wall planes to exert a constant outward force
    state.velocity += plane.normal * FORCE_WALL * dt;
  }

  if (Gm_Absf(plane.nDotU) <= 0.7f) {
    state.lastWallBumpNormal = plane.normal;

    if (!state.isOnSolidGround && time_since(state.lastWallBumpTime) > WALL_KICK_WINDOW_DURATION) {
      // Automatic wall kick
      state.lastWallBumpTime = get_scene_time();
      state.lastWallBumpVelocity = state.velocity;

      Vec3f wallPlaneVelocity = state.lastWallBumpVelocity.alignToPlane(state.lastWallBumpNormal);
      Vec3f kickDirection = (state.lastWallBumpNormal + Vec3f(0, 1.f, 0)).unit();

      state.velocity = wallPlaneVelocity + kickDirection * state.lastWallBumpVelocity.magnitude() * 0.75f;
      state.lastWallKickTime = get_scene_time();

      state.canPerformAirDash = true;
      state.canPerformWallKick = true;
    }
  }
}

internal void resolveAllPlaneCollisions(GmContext* context, GameState& state, float dt) {
  START_TIMING("resolveAllPlaneCollisions");

  auto& player = get_player();
  bool isFalling = state.previousPlayerPosition.y - player.position.y > 0.f;
  bool wasRecentlyOnSolidGround = time_since(state.lastTimeOnSolidGround) < 0.2f;
  bool didJustAirDash = time_since(state.lastAirDashTime) < 0.1f;
  bool didCollideWithSolidGround = false;

  // @todo implement world chunks + only consider collision planes local to the player
  for (auto& plane : state.collisionPlanes) {
    if (
      player.position.y > plane.maxY + PLAYER_RADIUS ||
      player.position.y < plane.minY - PLAYER_RADIUS
    ) {
      // Early out for collision planes not local to the player y position
      continue;
    }

    Vec3f lineStart = state.previousPlayerPosition;
    Vec3f lineEnd = player.position - plane.normal * PLAYER_RADIUS;
    auto collision = Collisions::getLinePlaneCollision(lineStart, lineEnd, plane);

    if (collision.hit) {
      resolveSingleCollision(context, state, collision, dt);

      if (collision.plane.nDotU > 0.7f) {
        didCollideWithSolidGround = true;
      }
    } else if (
      isFalling &&
      wasRecentlyOnSolidGround &&
      !didJustAirDash &&
      plane.nDotU > 0.6f
    ) {
      Vec3f fallCollisionLineEnd = player.position - plane.normal * 200.f;
      auto fallCollision = Collisions::getLinePlaneCollision(player.position, fallCollisionLineEnd, plane);

      if (fallCollision.hit) {
        resolveNewPositionFromCollision(fallCollision, player);

        state.velocity.y = 0.f;
        state.lastSolidGroundPosition = player.position;
        state.lastTimeOnSolidGround = get_scene_time();

        didCollideWithSolidGround = true;
      }
    }
  }

  // Set the solid ground flag after all collisions are resolved.
  // Setting it at ground collision resolution time can cause
  // subsequent ground collisions to trigger previous-position
  // reset behavior, causing the player to get stuck.
  state.isOnSolidGround = didCollideWithSolidGround;

  LOG_TIME();
}

internal void resolveAllNpcCollisions(GmContext* context, GameState& state) {
  START_TIMING("resolveAllNpcCollisions");

  auto& player = get_player();
  float distanceThreshold = NPC_RADIUS + PLAYER_RADIUS + 10.f;

  // @todo refactor cylinder collision behavior
  for (auto& npc : state.npcs) {
    Vec3f npcTop = npc.position + Vec3f(0, NPC_HEIGHT, 0);
    Vec3f npcBottom = npc.position - Vec3f(0, NPC_HEIGHT, 0);
    Vec3f xzNpcToPlayer = (player.position - npc.position).xz();
    Vec3f npcTopToPlayer = player.position - npcTop;
    float xzDistance = xzNpcToPlayer.magnitude();
    float topDistance = npcTopToPlayer.magnitude();

    if (
      player.position.y > npcTop.y &&
      topDistance < distanceThreshold
    ) {
      player.position = npcTop + npcTopToPlayer.unit() * distanceThreshold;

      break;
    } else if (
      xzDistance < distanceThreshold &&
      player.position.y < npcTop.y &&
      player.position.y > npcBottom.y
    ) {
      Vec3f xzNpcPosition = Vec3f(npc.position.x, player.position.y, npc.position.z);

      player.position = xzNpcPosition + xzNpcToPlayer.unit() * distanceThreshold;

      break;
    }
  }

  LOG_TIME();
}

internal void resolveAllHotAirBalloonCollisions(GmContext* context, GameState& state, float dt) {
  START_TIMING("resolveAllHotAirBalloonCollisions");

  auto& player = get_player();

  for (auto& balloon : objects("hot-air-balloon")) {
    float balloonRadius = balloon.scale.x;
    Vec3f balloonToPlayer = player.position - balloon.position;

    if (balloonToPlayer.magnitude() < PLAYER_RADIUS + balloonRadius) {
      Vec3f normalizedBalloonToPlayer = balloonToPlayer.unit();

      player.position = balloon.position + normalizedBalloonToPlayer * (balloonRadius + PLAYER_RADIUS);

      state.velocity = Vec3f::reflect(state.velocity, normalizedBalloonToPlayer) * 1.2f;
      state.canPerformAirDash = true;

      break;
    }
  }

  LOG_TIME();
}

namespace MovementSystem {
  void handlePlayerMovementInput(GmContext* context, GameState& state, float dt) {
    #if GAMMA_DEVELOPER_MODE
      if (state.isFreeCameraMode) {
        return;
      }
    #endif

    if (
      UISystem::hasBlockingDialogue() ||
      EntitySystem::isInteractingWithEntity(context, state) ||
      state.gameStartTime == 0.f
    ) {
      return;
    }

    START_TIMING("handlePlayerMovementInput");

    auto& input = get_input();
    auto& player = get_player();
    auto& camera = get_camera();

    auto rate = 5000.f * dt;
    auto initialVelocity = state.velocity;

    Vec3f forward = camera.orientation.getDirection().xz().unit();
    Vec3f left = camera.orientation.getLeftDirection().xz().unit();
    Vec3f acceleration = Vec3f(0);

    if (state.velocity.y != 0.f) {
      // Reduce movement rate in midair
      rate *= 0.05f;
    }

    if (input.isKeyHeld(Key::W)) {
      acceleration += forward * rate;
    }

    if (input.isKeyHeld(Key::S)) {
      acceleration += forward.invert() * rate;
    }

    if (input.isKeyHeld(Key::A)) {
      acceleration += left * rate;
    }

    if (input.isKeyHeld(Key::D)) {
      acceleration += left.invert() * rate;
    }

    // Directional change handling/turn factor determination
    {
      float targetTurnFactor = 0.f;
      float xzSpeed = state.velocity.xz().magnitude();
      float xzAcceleration = acceleration.xz().magnitude();

      if (xzSpeed > 0.f && xzAcceleration > 0.f) {
        Vec3f unitXzAcceleration = acceleration.xz() / xzAcceleration;
        Vec3f playerLeft = player.rotation.getLeftDirection();
        float directionChangeDot = Vec3f::dot(state.velocity.xz() / xzSpeed, unitXzAcceleration);

        targetTurnFactor = (1.f - directionChangeDot) * (Vec3f::dot(unitXzAcceleration, playerLeft) > 0.f ? 1.f : -1.f);

        if (state.velocity.y != 0.f && time_since(state.lastWallKickTime) > 1.f) {
          // When midair, adjust acceleration in proportion to how much it resists
          // the current velocity. We want turning to be easier when airborne.
          float directionChangeFactor = 1.f - directionChangeDot;

          acceleration *= 1.f + directionChangeFactor * 3.f;
        }
      }

      state.turnFactor = Gm_Clampf(Gm_Lerpf(state.turnFactor, targetTurnFactor, 5.f * dt), -0.7f, 0.7f);
    }

    state.velocity += acceleration;
    state.isMovingPlayerThisFrame = state.velocity != initialVelocity;

    // Limit horizontal speed on ground
    {
      Vec3f horizontalVelocity = state.velocity.xz();

      float speedLimit = MAXIMUM_HORIZONTAL_GROUND_SPEED * (
        state.dashLevel == 1 ? DASH_LEVEL_1_SPEED_FACTOR :
        state.dashLevel == 2 ? DASH_LEVEL_2_SPEED_FACTOR :
        1.f
      );

      if (state.isOnSolidGround && horizontalVelocity.magnitude() > speedLimit) {
        Vec3f limitedHorizontalVelocity = horizontalVelocity.unit() * speedLimit;

        state.velocity.x = limitedHorizontalVelocity.x;
        state.velocity.z = limitedHorizontalVelocity.z;
      }
    }

    // Handle jump/wall kick/air dash actions
    {
      if (input.didPressKey(Key::SPACE)) {
        float time = get_scene_time();

        if (state.isOnSolidGround) {
          // Regular jump
          float jumpFactor = (
            state.dashLevel == 1 ? 1.5f :
            state.dashLevel == 2 ? 2.f :
            1.f
          );

          if (time_since(state.lastAirDashTime) < SUPER_JUMP_WINDOW_DURATION) {
            // Super jump
            jumpFactor *= 2.f;

            context->scene.fx.screenWarpTime = time;

            state.lastHardLandingPosition = player.position;
            state.lastHardLandingTime = time;
          }

          state.velocity.y = DEFAULT_JUMP_Y_VELOCITY * jumpFactor;
          state.lastJumpTime = time;

          state.isOnSolidGround = false;
          state.canPerformAirDash = true;

          // Make sure the player is off the ground plane
          // at the start of the jump to avoid a next-frame
          // collision snapping them back in place
          player.position.y += 2.f;
        } else if (state.canPerformWallKick && time_since(state.lastWallBumpTime) < WALL_KICK_WINDOW_DURATION) {
          // Wall kick
          Vec3f wallPlaneVelocity = state.lastWallBumpVelocity.alignToPlane(state.lastWallBumpNormal);
          Vec3f kickDirection = (state.lastWallBumpNormal + Vec3f(0, 1.f, 0)).unit();

          state.velocity = wallPlaneVelocity + kickDirection * state.lastWallBumpVelocity.magnitude();
          state.lastWallKickTime = time;

          state.canPerformAirDash = true;
          state.canPerformWallKick = true;

          context->scene.fx.screenWarpTime = time;
        } else if (state.canPerformAirDash) {
          // Air dash
          Vec3f airDashDirection = camera.orientation.getDirection();

          if (airDashDirection.y < 0.f) {
            airDashDirection = (airDashDirection * Vec3f(2.f, 1.f, 2.f)).unit();
          }

          state.velocity = airDashDirection * MAXIMUM_HORIZONTAL_GROUND_SPEED * (
            // Start dashing at level 1 speed
            state.dashLevel == 0 ? DASH_LEVEL_1_SPEED_FACTOR :
            // Start dashing at level 2 speed
            state.dashLevel == 1 ? DASH_LEVEL_2_SPEED_FACTOR :
            // Cap at speed level 2
            DASH_LEVEL_2_SPEED_FACTOR
          );

          state.canPerformAirDash = false;
          state.canPerformWallKick = true;

          // @todo configure upper limit
          if (state.dashLevel < 2) {
            state.dashLevel++;
          }

          state.lastAirDashTime = time;
          state.airDashSpinStartYaw = state.currentYaw;
          state.airDashSpinEndYaw = atan2(airDashDirection.x, airDashDirection.z) + Gm_PI;
          if (state.airDashSpinEndYaw - state.airDashSpinStartYaw < Gm_PI) state.airDashSpinEndYaw += Gm_TAU;

          context->scene.fx.screenWarpTime = time;
        }
      }
    }

    // Movement state resets
    {
      if (state.dashLevel > 0 && state.isOnSolidGround && !state.isMovingPlayerThisFrame) {
        // Stop dashing when we cease movement while dashing along the ground
        state.dashLevel = 0;
      }

      if (state.isOnSolidGround && state.velocity.xz().magnitude() > 1.f) {
        // If we were on solid ground, but any movement
        // occurs along the xz plane, all bets are off!
        state.isOnSolidGround = false;
      }
    }

    LOG_TIME();
  }

  void handlePlayerMovementPhysics(GmContext* context, GameState& state, float dt) {
    auto& input = get_input();
    auto& player = get_player();
    float gravity = FORCE_GRAVITY * dt;
    Vec3f initialVelocity = state.velocity;

    // Handle gravity/velocity
    {
      bool didLaunchFromRing = state.lastRingLaunchTime > 0.f && time_since(state.lastRingLaunchTime) < 1.f;

      if (
        time_since(state.lastJumpTime) < 1.f &&
        !input.isKeyHeld(Key::SPACE) &&
        state.velocity.y > 0.f &&
        !didLaunchFromRing
      ) {
        // When releasing the jump key shortly after a jump,
        // diminish y velocity more quickly to reduce the
        // total jump height.
        state.velocity.y *= 1.f - 2.f * dt;
      }
        
      if (!didLaunchFromRing) {
        state.velocity.y -= gravity;
      }

      player.position += state.velocity * dt;
    }

    // Track the xz distance traveled from the last solid ground position
    // *before* resolving collisions, so that any resolved ground collisions
    // won't reset it. We use this to determine ground friction afterward.
    float lastSolidGroundXzDistance = (state.lastSolidGroundPosition.xz() - player.position.xz()).magnitude();

    resolveAllPlaneCollisions(context, state, dt);
    resolveAllNpcCollisions(context, state);
    resolveAllHotAirBalloonCollisions(context, state, dt);

    if (state.isOnSolidGround) {
      // Reset the ability to perform wall kicks any time we're on solid ground.
      // Otherwise, jumping and landing keeps the flag set to true, and bumping
      // into a wall triggers the wall kick wind-up action.
      state.canPerformWallKick = false;

      if (initialVelocity.y < -550.f) {
        state.lastHardLandingPosition = player.position - Vec3f(0, PLAYER_RADIUS * 0.5f, 0);
        state.lastHardLandingTime = get_scene_time();
      }
    }

    if (state.isOnSolidGround && !state.isMovingPlayerThisFrame) {
      if (lastSolidGroundXzDistance > 100.f) {
        // When landing from sufficiently long jumps, immediately reduce
        // the xz velocity to a fraction of the original to slow down quickly
        state.velocity.x *= 0.25f;
        state.velocity.z *= 0.25f;
      } else {
        // Gradually slow down along solid ground
        state.velocity.x *= (1.0f - 7.f * dt);
        state.velocity.z *= (1.0f - 7.f * dt);
      }
    }
  }
}