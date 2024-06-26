#include "movement_system.h"
#include "ui_system.h"
#include "entity_system.h"
#include "collisions.h"
#include "easing.h"
#include "game_constants.h"
#include "macros.h"

using namespace Gamma;

internal bool isIssuingDirectionalInput(GmContext* context) {
  auto& input = get_input();

  return (
    input.isKeyHeld(Key::W) ||
    input.isKeyHeld(Key::A) ||
    input.isKeyHeld(Key::S) ||
    input.isKeyHeld(Key::D) ||
    input.getLeftStick().x != 0.f ||
    input.getLeftStick().y != 0.f
  );
}

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
      // @todo fix player model orientation
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

    if (time_since(state.lastTimeOnSolidGround) > 0.1f && time_since(state.lastWallBumpTime) > WALL_KICK_WINDOW_DURATION) {
      // Automatic wall kick
      // state.lastWallBumpTime = get_scene_time();
      // state.lastWallBumpVelocity = state.velocity;

      // Vec3f wallPlaneVelocity = state.lastWallBumpVelocity.alignToPlane(state.lastWallBumpNormal);
      // Vec3f kickDirection = (state.lastWallBumpNormal + Vec3f(0, 1.f, 0)).unit();

      // state.velocity = wallPlaneVelocity + kickDirection * state.lastWallBumpVelocity.magnitude() * 0.75f;
      // state.lastWallKickTime = get_scene_time();

      // state.canPerformAirDash = true;
      // state.canPerformWallKick = true;
    }
  }
}

internal void resolveAllPlaneCollisions(GmContext* context, GameState& state, float dt) {
  START_TIMING("resolveAllPlaneCollisions");

  auto& player = get_player();
  bool wasRecentlyOnSolidGround = time_since(state.lastTimeOnSolidGround) < 0.2f;
  bool didJustJump = time_since(state.lastJumpTime) < 0.5f;
  bool didJustAirDash = time_since(state.lastAirDashTime) < 0.1f;
  bool resolvedCollisionWithSolidGround = false;
  float highestPlane = -Gm_FLOAT_MAX;

  auto toriiGatePlatformMeshIndex = mesh("torii-platform")->index;

  // @todo implement world chunks + only consider collision planes local to the player
  for (auto& plane : state.collisionPlanes) {
    if (
      player.position.y > plane.maxY + 2.f * PLAYER_RADIUS ||
      player.position.y < plane.minY - 2.f * PLAYER_RADIUS
    ) {
      // Early out for collision planes not local to the player y position
      continue;
    }

    if (!state.isInToriiGateZone && plane.sourceObjectRecord.meshIndex == toriiGatePlatformMeshIndex) {
      // Early out for torii gate platform collision planes when not in torii gate zones
      continue;
    }

    // @todo use distinct logic for floors (nDotU > 0.7f) to improve ground collision handling/slope transitions
    Vec3f lineStart = player.position - plane.normal * PLAYER_RADIUS;
    Vec3f lineEnd = player.position + plane.normal * PLAYER_RADIUS;
    auto collision = Collisions::getLinePlaneCollision(lineStart, lineEnd, plane);

    if (collision.hit) {
      // Resolve any standard motion collisions
      resolveSingleCollision(context, state, collision, dt);

      state.lastPlaneCollidedWith = plane;
      state.isDoingTargetedAirDash = false;

      if (collision.plane.nDotU > 0.7f) {
        resolvedCollisionWithSolidGround = true;
      }
    } else if (
      wasRecentlyOnSolidGround &&
      !resolvedCollisionWithSolidGround &&
      !didJustJump &&
      !didJustAirDash &&
      plane.nDotU > 0.7f &&
      (
        plane.nDotU == state.lastPlaneCollidedWith.nDotU ||
        (state.lastPlaneCollidedWith.minY - plane.minY) > 50.f
      )
    ) {
      // Snap the player to sloped floors
      Vec3f fallCollisionLineEnd = player.position - plane.normal * 200.f;
      auto fallCollision = Collisions::getLinePlaneCollision(player.position, fallCollisionLineEnd, plane);

      if (fallCollision.hit) {
        resolveNewPositionFromCollision(fallCollision, player);

        state.velocity.y = 0.f;
        state.lastSolidGroundPosition = player.position;
        state.lastTimeOnSolidGround = get_scene_time();
        state.lastPlaneCollidedWith = plane;
        state.isDoingTargetedAirDash = false;

        resolvedCollisionWithSolidGround = true;
      }
    }
  }

  // Set the solid ground flag after all collisions are resolved.
  // Setting it at ground collision resolution time can cause
  // subsequent ground collisions to trigger previous-position
  // reset behavior, causing the player to get stuck.
  state.isOnSolidGround = resolvedCollisionWithSolidGround;

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

internal void handleNormalMovementInput(GmContext* context, GameState& state, float dt) {
  auto& input = get_input();
  auto& player = get_player();
  auto& camera = get_camera();

  auto rate = 3000.f * dt;
  auto initialVelocity = state.velocity;

  Vec3f forward = camera.orientation.getDirection().xz().unit();
  Vec3f left = camera.orientation.getLeftDirection().xz().unit();
  Vec3f acceleration = Vec3f(0);

  if (state.velocity.y != 0.f) {
    // Reduce movement rate in midair
    rate *= 0.05f;
  }

  // Directional movement
  if (time_since(state.lastLedgeTurnaroundTime) > 0.2f && !state.isDoingTargetedAirDash) {
    // WASD
    {
      if (input.isKeyHeld(Key::W)) {
        acceleration += forward * rate;
      }

      if (input.isKeyHeld(Key::S)) {
        // @todo fix player model orientation
        acceleration += forward.invert() * rate;
      }

      if (input.isKeyHeld(Key::A)) {
        acceleration += left * rate;
      }

      if (input.isKeyHeld(Key::D)) {
        // @todo fix player model orientation
        acceleration += left.invert() * rate;
      }
    }

    // Analog sticks
    {
      acceleration += forward * input.getLeftStick().y * rate * -1.f;
      acceleration += left * input.getLeftStick().x * rate * -1.f;
    }
  }

  // Directional change handling/turn factor determination
  {
    float targetTurnFactor = 0.f;
    float xzSpeed = state.velocity.xz().magnitude();
    float xzAcceleration = acceleration.xz().magnitude();

    if (xzSpeed > 0.f && xzAcceleration > 0.f) {
      Vec3f unitXzVelocity = state.velocity.xz() / xzSpeed;
      Vec3f unitXzAcceleration = acceleration.xz() / xzAcceleration;
      Vec3f playerLeft = player.rotation.getLeftDirection();
      float directionChangeDot = Vec3f::dot(unitXzVelocity, unitXzAcceleration);

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

  // Limit horizontal speed
  {
    Vec3f horizontalVelocity = state.velocity.xz();

    float speedLimit = state.isOnSolidGround
      ? MAXIMUM_HORIZONTAL_GROUND_SPEED * (
          state.dashLevel == 1 ? DASH_LEVEL_1_SPEED_FACTOR :
          state.dashLevel == 2 ? DASH_LEVEL_2_SPEED_FACTOR :
          1.f
        )
      : MAXIMUM_HORIZONTAL_AIR_SPEED;

    if (
      horizontalVelocity.magnitude() > speedLimit &&
      !state.isDoingTargetedAirDash
    ) {
      Vec3f limitedHorizontalVelocity = horizontalVelocity.unit() * speedLimit;

      state.velocity.x = limitedHorizontalVelocity.x;
      state.velocity.z = limitedHorizontalVelocity.z;
    }
  }

  // Handle jump/wall kick/air dash actions
  {
    // @todo change the key binding for charged super jumps
    if (input.isKeyHeld(Key::CONTROL)) {
      state.superjumpChargeTime += dt;
    } else {
      state.superjumpChargeTime = 0.f;
    }

    if (!state.isNearActionableEntity && (
      input.didPressKey(Key::SPACE) ||
      input.didPressKey(Key::CONTROLLER_A)
    )) {
      float sceneTime = get_scene_time();

      if (state.isOnSolidGround) {
        // Jump
        float jumpFactor = (
          state.dashLevel == 1 ? 1.5f :
          state.dashLevel == 2 ? 2.f :
          1.f
        );

        if (
          !state.isNearJumpPad && (
            time_since(state.lastHardLandingTime) < SUPER_JUMP_WINDOW_DURATION ||
            time_since(state.lastAirDashTime) < SUPER_JUMP_WINDOW_DURATION ||
            state.superjumpChargeTime != 0.f
          )
        ) {
          // Super jump
          jumpFactor *= 2.f;

          if (state.superjumpChargeTime != 0.f) {
            // Charged super jump
            float alpha = 1.f + state.superjumpChargeTime / (state.superjumpChargeTime + 2.f);

            // @todo fix player model orientation
            state.velocity += player.rotation.getDirection().invert() * 200.f;
            state.superjumpChargeTime = 0.f;

            jumpFactor *= alpha;
          }

          state.lastHardLandingPosition = player.position;
          state.lastHardLandingTime = sceneTime;

          context->scene.fx.screenWarpTime = sceneTime;
        }

        if (state.isNearJumpPad) {
          // Jump pad
          jumpFactor = 6.f;

          state.velocity = Vec3f(0.f);
          state.lastJumpPadLaunchTime = sceneTime;

          // @note this is copied from the air dash spin code below;
          // we might want a dedicated effect for launch pad jumps
          state.lastAirDashTime = sceneTime;
          state.airDashSpinStartYaw = state.currentYaw;
          state.airDashSpinEndYaw = atan2(state.velocity.x, state.velocity.z) + Gm_PI;
          if (state.airDashSpinEndYaw - state.airDashSpinStartYaw < Gm_PI) state.airDashSpinEndYaw += Gm_TAU;

          context->scene.fx.screenWarpTime = sceneTime;
        }

        if (time_since(state.lastGroundPoundTime) < 0.5f) {
          jumpFactor *= 1.25f;

          state.velocity += camera.orientation.getDirection().xz() * 250.f;
          state.lastGroundPoundJumpTime = sceneTime;
          state.dashLandingJumpStartCameraAltitude = state.camera3p.altitude;
        }

        state.velocity.y = DEFAULT_JUMP_Y_VELOCITY * jumpFactor;
        state.lastJumpTime = sceneTime;

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
        state.lastWallKickTime = sceneTime;

        state.canPerformAirDash = true;
        state.canPerformWallKick = true;

        context->scene.fx.screenWarpTime = sceneTime;
      } else if (state.canPerformAirDash) {
        // Air dash
        Vec3f airDashDirection;

        if (state.hasAirDashTarget) {
          // Targeted air dash
          auto& target = objects("air-dash-target")[0];

          airDashDirection = (target.position - player.position).unit();

          state.isDoingTargetedAirDash = true;
          state.velocity = airDashDirection * MAXIMUM_HORIZONTAL_GROUND_SPEED * DASH_LEVEL_2_SPEED_FACTOR * 2.f;

          point_camera_at(target);
        } else if (time_since(state.lastJumpTime) > 1.f) {
          airDashDirection = camera.orientation.getDirection();

          state.velocity = airDashDirection * MAXIMUM_HORIZONTAL_GROUND_SPEED * DASH_LEVEL_2_SPEED_FACTOR * 2.f;
        } else {
          // Normal air dash
          airDashDirection = camera.orientation.getDirection();

          if (time_since(state.lastJumpTime) < 1.f) {
            airDashDirection += Vec3f(0, -2.f, 0);
            airDashDirection = airDashDirection.unit();

            state.lastGroundPoundTime = sceneTime;
          } else if (airDashDirection.y < 0.f) {
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
        }

        state.canPerformAirDash = false;
        state.canPerformWallKick = true;

        // @todo configure upper limit
        if (state.dashLevel < 2) {
          state.dashLevel = 2;

          if (state.dashLevel == 2) {
            state.lastDashLevel2Time = sceneTime;
          }
        }

        state.lastAirDashTime = sceneTime;
        state.airDashSpinStartYaw = state.currentYaw;
        state.airDashSpinEndYaw = atan2(airDashDirection.x, airDashDirection.z) + Gm_PI;
        if (state.airDashSpinEndYaw - state.airDashSpinStartYaw < Gm_PI) state.airDashSpinEndYaw += Gm_TAU;

        context->scene.fx.screenWarpTime = sceneTime;
      }
    }
  }

  // Movement state resets
  {
    auto isDashFlowerActive = state.lastDashFlowerCollectionTime != 0.f && time_since(state.lastDashFlowerCollectionTime) < 10.f;

    if (state.dashLevel > 0 && state.isOnSolidGround && !state.isMovingPlayerThisFrame && !isDashFlowerActive) {
      // Stop dashing when we cease movement while dashing along the ground
      state.dashLevel = 0;
    }

    if (state.isOnSolidGround && state.velocity.xz().magnitude() > 1.f) {
      // If we were on solid ground, but any movement
      // occurs along the xz plane, all bets are off!
      state.isOnSolidGround = false;
    }
  }
}

internal void handleGliderMovementInput(GmContext* context, GameState& state, float dt) {
  auto& input = get_input();
  auto& player = get_player();
  float targetTurnFactor = 0.f;
  float speed = state.velocity.magnitude();
  float speedFactor = speed / (speed + 500.f);

  if (input.isKeyHeld(Key::W)) {
    state.currentPitch += 2.f * dt;
  }

  if (input.isKeyHeld(Key::S)) {
    state.currentPitch -= 2.f * dt * speedFactor;
  }

  if (input.isKeyHeld(Key::A)) {
    state.currentYaw -= 2.f * dt;

    targetTurnFactor = -1.f;
  }

  if (input.isKeyHeld(Key::D)) {
    state.currentYaw += 2.f * dt;

    targetTurnFactor = 1.f;
  }

  state.turnFactor = Gm_Lerpf(state.turnFactor, targetTurnFactor, 2.f * dt);
  state.turnFactor = Gm_Clampf(state.turnFactor, -1.f, 1.f);

  // @todo move the below into handlePlayerMovementPhysics()
  float targetPitch = Gm_Lerpf(0.5f, -0.5f, speed / MAXIMUM_GLIDER_SPEED);

  state.currentPitch = Gm_Lerpf(state.currentPitch, targetPitch, 5.f * (1.f - speedFactor) * dt);

  // @todo fix player model orientation
  float azimuth = state.currentYaw - Gm_HALF_PI;
  float altitude = -state.currentPitch;
  float cosAltitude = cosf(altitude);

  Vec3f direction = Vec3f(
    // @todo fix player model orientation
    -1.f * cosAltitude * cosf(azimuth),
    sinf(altitude),
    cosAltitude * sinf(azimuth)
  );

  float velocityFactor = 0.1f + Vec3f::dot(direction, Vec3f(0, -1.f, 0));

  if (time_since(state.lastBoostRingLaunchTime) > 1.f) {
    state.velocity = direction * speed;
    state.velocity += direction * 10.f * velocityFactor * dt;

    // Use banking to determine additional velocity
    state.velocity += player.rotation.getLeftDirection().unit() * state.turnFactor * 10000.f * dt;
  }

  float newSpeed = state.velocity.magnitude();

  if (newSpeed > MAXIMUM_GLIDER_SPEED) {
    state.velocity *= (MAXIMUM_GLIDER_SPEED / speed);
  }
}

/**
 * @todo description
 */
internal bool willPlayerFallOffLedge(GmContext* context, GameState& state) {
  auto& player = get_player();
  auto start = player.position;
  auto end = player.position - Vec3f(0, PLAYER_RADIUS + 10.f, 0);

  for (auto& plane : state.collisionPlanes) {
    if (plane.minY > start.y) continue;
    if (plane.maxY < end.y) continue;

    auto collision = Collisions::getLinePlaneCollision(start, end, plane);

    if (collision.hit) {
      return false;
    }
  }

  return true;
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

    if (
      get_input().didPressKey(Key::SHIFT) &&
      !state.isOnSolidGround
    ) {
      state.isGliding = !state.isGliding;
      state.lastGliderChangeTime = get_scene_time();
    }

    if (state.isGliding) {
      handleGliderMovementInput(context, state, dt);
    } else {
      handleNormalMovementInput(context, state, dt);
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
      float boostRingDuration = state.isGliding ? GLIDER_BOOST_RING_DURATION : BOOST_RING_DURATION;
      bool didUseBoostRing = state.lastBoostRingLaunchTime != 0.f && time_since(state.lastBoostRingLaunchTime) < boostRingDuration;

      if (
        time_since(state.lastJumpTime) < 1.f &&
        state.velocity.y > 0.f && (
          input.isKeyHeld(Key::SPACE) ||
          input.isKeyHeld(Key::CONTROLLER_A)
        )
      ) {
        // Gain jump height when holding down the jump button
        auto alpha = 1.f - time_since(state.lastJumpTime);

        state.velocity.y += 750.f * alpha * dt;
      }

      if (!didUseBoostRing && !state.isDoingTargetedAirDash) {
        state.velocity.y -= gravity;
      }

      if (state.velocity.y < -3000.f) {
        // Terminal falling velocity
        state.velocity.y = -3000.f;
        state.velocity.x *= 1.f - 0.5f * dt;
        state.velocity.z *= 1.f - 0.5f * dt;
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

      if (state.isGliding) {
        // If the glider is active on solid ground, disable it
        state.isGliding = false;
        state.lastGliderChangeTime = get_scene_time();
      }

      if (initialVelocity.magnitude() > 550.f && initialVelocity.y < -200.f) {
        state.lastHardLandingPosition = player.position - Vec3f(0, PLAYER_RADIUS * 0.5f, 0);
        state.lastHardLandingVelocity = initialVelocity;
        state.lastHardLandingTime = get_scene_time();
      }
    }

    if (
      state.wasOnSolidGroundLastFrame &&
      !state.isOnSolidGround &&
      time_since(state.lastHardLandingTime) > 0.5f &&
      time_since(state.lastJumpTime) > 0.1f &&
      willPlayerFallOffLedge(context, state)
    ) {
      // Prevent the player from inadvertently walking off ledges
      player.position = state.lastSolidGroundPosition;

      // @todo description
      auto plane = state.lastPlaneCollidedWith;
      float max = Gm_FLOAT_MAX, dot;
      Vec3f tangent;
      Vec3f edge;

      if ((dot = Vec3f::dot(player.position - plane.p1, plane.t1)) < max) {
        tangent = plane.t1;
        edge = plane.p1 - plane.p2;
        max = dot;
      }

      if ((dot = Vec3f::dot(player.position - plane.p2, plane.t2)) < max) {
        tangent = plane.t2;
        edge = plane.p2 - plane.p3;
        max = dot;
      }

      if ((dot = Vec3f::dot(player.position - plane.p3, plane.t3)) < max) {
        tangent = plane.t3;
        edge = plane.p3 - plane.p4;
        max = dot;
      }

      if ((dot = Vec3f::dot(player.position - plane.p4, plane.t4)) < max) {
        tangent = plane.t4;
        edge = plane.p4 - plane.p1;
        max = dot;
      }

      if (Vec3f::dot(edge, state.velocity) < 0.f) {
        edge = edge.invert();
      }

      auto redirection = edge.unit() * 3.f + tangent.unit();
      auto redirectedVelocity = redirection.unit() * state.velocity.magnitude();

      state.velocity.x = redirectedVelocity.x;
      state.velocity.z = redirectedVelocity.z;
      state.lastLedgeTurnaroundTime = get_scene_time();
    }

    if (state.isOnSolidGround && !isIssuingDirectionalInput(context)) {
      if (lastSolidGroundXzDistance > 100.f) {
        // When landing from sufficiently long jumps, immediately reduce
        // the xz velocity to a fraction of the original to slow down quickly
        state.velocity.x *= 0.25f;
        state.velocity.z *= 0.25f;
      } else {
        // Gradually slow down along solid ground
        state.velocity.x *= (1.0f - 5.f * dt);
        state.velocity.z *= (1.0f - 5.f * dt);
      }
    }

    #if GAMMA_DEVELOPER_MODE
      // Display player motion vectors if dev tools are enabled
      mesh("motion_indicator")->disabled = !Gm_IsFlagEnabled(GammaFlags::ENABLE_DEV_TOOLS);

      if (Gm_IsFlagEnabled(GammaFlags::ENABLE_DEV_TOOLS)) {
        const float thickness = 3.f;

        // Velocity vectors
        auto& vx = objects("motion_indicator")[0];
        auto& vy = objects("motion_indicator")[1];
        auto& vz = objects("motion_indicator")[2];

        vx.position = player.position + Vec3f(state.velocity.x * 0.2f, 0, 0);
        vy.position = player.position + Vec3f(0, state.velocity.y * 0.2f, 0);
        vz.position = player.position + Vec3f(0, 0, state.velocity.z * 0.2f);

        vx.scale = Vec3f(state.velocity.x * 0.2f, thickness, thickness);
        vy.scale = Vec3f(thickness, state.velocity.y * 0.2f, thickness);
        vz.scale = Vec3f(thickness, thickness, state.velocity.z * 0.2f);

        vx.color = Vec3f(0, 0, 1.f);
        vy.color = Vec3f(0, 1.f, 0);
        vz.color = Vec3f(1.f, 0, 0);

        // Motion vector
        auto& motion = objects("motion_indicator")[3];
        auto yaw = atan2f(state.velocity.x, state.velocity.z);
        float pitch = atan2f(state.velocity.xz().magnitude(), state.velocity.y) + Gm_HALF_PI;

        motion.position = player.position + state.velocity * 0.2f;
        motion.scale = Vec3f(thickness, thickness, state.velocity.magnitude() * 0.2f);
        motion.rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), yaw) * Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), pitch);
        motion.color = Vec3f(0, 1.f, 1.f);

        commit(vx);
        commit(vy);
        commit(vz);
        commit(motion);
      }
    #endif
  }
}