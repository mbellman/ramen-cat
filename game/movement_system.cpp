#include "movement_system.h"
#include "ui_system.h"
#include "collision.h"
#include "macros.h"

constexpr float FORCE_GRAVITY = 750.f;
constexpr float FORCE_WALL = 1000.f;

using namespace Gamma;

internal void resolveSingleCollision(GmContext* context, GameState& state, const Collision& collision, float dt) {
  auto& player = get_player();
  auto& plane = collision.plane;

  player.position = collision.point + plane.normal * player.scale.x;

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

    state.velocity.y = 0.f;
    state.lastSolidGroundPosition = player.position;
    state.lastTimeOnSolidGround = state.frameStartTime;
  } else if (plane.nDotU < -0.7f && state.velocity.y > 0.f) {
    // When hitting a downward-facing plane from the underside,
    // immediately negate any upward velocity.
    state.velocity.y = 0.f;
  } else {
    // Allow wall planes to exert a constant outward force
    state.velocity += plane.normal * FORCE_WALL * dt;
  }

  if (Gm_Absf(plane.nDotU) <= 0.7f) {
    state.lastWallBumpTime = state.frameStartTime;
    state.lastBumpedWallNormal = plane.normal;
  }
}

internal void resolveAllPlaneCollisions(GmContext* context, GameState& state, float dt) {
  START_TIMING("resolveAllPlaneCollisions");

  auto& player = get_player();
  float playerRadius = player.scale.x;
  bool isFalling = state.previousPlayerPosition.y - player.position.y > 0.f;

  // @optimize precalculate collision plane min/max y, check against
  // player y and skip collision detection if out of range
  // @todo implement world chunks + only consider collision planes local to the player
  for (auto& plane : state.collisionPlanes) {
    Vec3f lineStart = state.previousPlayerPosition;
    Vec3f lineEnd = player.position - plane.normal * playerRadius;
    auto collision = getLinePlaneCollision(lineStart, lineEnd, plane);

    if (collision.hit) {
      resolveSingleCollision(context, state, collision, dt);
    } else if (isFalling && plane.nDotU > 0.6f) {
      // @todo description
      Vec3f fallCollisionLineEnd = player.position - plane.normal * (playerRadius + 5.f);
      auto fallCollision = getLinePlaneCollision(player.position, fallCollisionLineEnd, plane);

      if (fallCollision.hit) {
        player.position = fallCollision.point + plane.normal * playerRadius;

        state.velocity.y = 0.f;
        state.lastSolidGroundPosition = player.position;
        state.lastTimeOnSolidGround = state.frameStartTime;
      }
    }
  }

  if (state.velocity.y == 0.f) {
    // Set the solid ground flag after all collisions are resolved.
    // Setting it within ground collision resolution can cause
    // subsequent ground collisions to trigger previous-position
    // reset behavior, causing the player to get stuck.
    state.isOnSolidGround = true;
  }

  LOG_TIME();
}

internal void resolveAllNpcCollisions(GmContext* context, GameState& state) {
  START_TIMING("resolveAllNpcCollisions");

  auto& player = get_player();

  // @todo define these constants elsewhere and use them in player/NPC generation
  const float NPC_RADIUS = 20.f;
  const float NPC_HEIGHT = 70.f;
  const float PLAYER_RADIUS = player.scale.x;

  const float distanceThreshold = NPC_RADIUS + PLAYER_RADIUS + 10.f;

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

      commit(player);

      break;
    } else if (
      xzDistance < distanceThreshold &&
      player.position.y < npcTop.y &&
      player.position.y > npcBottom.y
    ) {
      Vec3f xzNpcPosition = Vec3f(npc.position.x, player.position.y, npc.position.z);

      player.position = xzNpcPosition + xzNpcToPlayer.unit() * distanceThreshold;

      commit(player);

      break;
    }
  }

  LOG_TIME();
}

namespace MovementSystem {
  void handlePlayerMovementInput(GmContext* context, GameState& state, float dt) {
    if (UISystem::hasBlockingDialogue()) {
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

    if (state.velocity.y != 0.f) {
      // Reduce movement rate in midair
      rate *= 0.05f;
    } else if (state.velocity.xz().magnitude() > 600.f) {
      // Limit top xz speed
      rate = 0.f;
    }

    if (input.isKeyHeld(Key::W)) {
      state.velocity += forward * rate;
    }
    
    if (input.isKeyHeld(Key::S)) {
      state.velocity += forward.invert() * rate;
    }

    if (input.isKeyHeld(Key::A)) {
      state.velocity += left * rate;
    }
    
    if (input.isKeyHeld(Key::D)) {
      state.velocity += left.invert() * rate;
    }

    state.isPlayerMovingThisFrame = state.velocity != initialVelocity;

    // Handle jump/wall kick actions
    {
      if (input.didPressKey(Key::SPACE)) {
        if (state.velocity.y == 0.f) {
          // Regular jump (@todo use state.isOnSolidGround?)
          state.velocity.y = 500.f;
        } else {
          // If we press SPACE in mid-air, queue a wall kick action.
          // We'll determine whether it's appropriate to perform
          // a wall kick next.
          state.lastWallKickInputTime = state.frameStartTime;
        }
      }

      float timeSinceLastWallBump = state.frameStartTime - state.lastWallBumpTime;
      float timeSinceLastWallKickInput = state.frameStartTime - state.lastWallKickInputTime;
      float timeSinceLastWallKick = state.frameStartTime - state.lastWallKickTime;

      if (
        timeSinceLastWallBump < 0.2f &&
        timeSinceLastWallKickInput < 0.2f &&
        timeSinceLastWallKick > 0.3f
      ) {
        // Wall kick
        Vec3f wallPlaneVelocity = state.velocity.alignToPlane(state.lastBumpedWallNormal);
        Vec3f kickDirection = (state.lastBumpedWallNormal + Vec3f(0, 3.f, 0)).unit();

        state.velocity = wallPlaneVelocity + kickDirection * state.velocity.magnitude();
        state.lastWallKickTime = state.frameStartTime;
      }
    }

    LOG_TIME();
  }

  void handlePlayerMovementPhysics(GmContext* context, GameState& state, float dt) {
    auto& player = get_player();
    const float gravity = FORCE_GRAVITY * dt;

    // Handle gravity/velocity
    state.velocity.y -= gravity;
    player.position += state.velocity * dt;

    resolveAllPlaneCollisions(context, state, dt);
    resolveAllNpcCollisions(context, state);

    if (state.velocity.y == 0.f && !state.isPlayerMovingThisFrame) {
      const float slowdown = 5.f;

      state.velocity.x = Gm_Lerpf(state.velocity.x, 0.f, slowdown * dt);
      state.velocity.z = Gm_Lerpf(state.velocity.z, 0.f, slowdown * dt);
    }

    commit(player);
  }
}