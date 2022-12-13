#include "movement_system.h"
#include "macros.h"

using namespace Gamma;

struct Collision {
  Plane plane;
  Vec3f point;
  bool hit = false;
};

#define min(a, b) (a > b ? b : a)
#define max(a, b) (a > b ? a : b)

internal bool isInBetween(float n, float a, float b) {
  return n >= min(a, b) && n <= max(a, b);
}

internal Collision getLinePlaneCollision(const Vec3f& lineStart, const Vec3f& lineEnd, const Plane& plane) {
  Collision collision;
  Vec3f line = lineEnd - lineStart;

  if (Vec3f::dot(plane.normal, line) != 0.f) {
    float nDotP = Vec3f::dot(plane.normal, plane.p1);
    float length = (nDotP - Vec3f::dot(plane.normal, lineStart)) / Vec3f::dot(plane.normal, line);
    Vec3f point = lineStart + line * length;

    if (
      // If the point is on the line segment
      isInBetween(point.x, lineStart.x, lineEnd.x) &&
      isInBetween(point.y, lineStart.y, lineEnd.y) &&
      isInBetween(point.z, lineStart.z, lineEnd.z) &&
      // And the point is inside the plane area
      Vec3f::dot(point - plane.p1, plane.t1) >= 0.f &&
      Vec3f::dot(point - plane.p2, plane.t2) >= 0.f &&
      Vec3f::dot(point - plane.p3, plane.t3) >= 0.f &&
      Vec3f::dot(point - plane.p4, plane.t4) >= 0.f
    ) {
      collision.plane = plane;
      collision.point = point;
      collision.hit = true;
    }
  }

  return collision;
}

internal void resolveSingleCollision(GmContext* context, GameState& state, const Collision& collision) {
  auto& player = getPlayer();
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
  } else {
    // Adjust bounce friction based on whether we're falling,
    // or walking solid ground (e.g. running into a wall)
    Vec3f friction = state.velocity.y == 0.f ? Vec3f(0.5f) : Vec3f(0.8f, 1.f, 0.8f);

    state.velocity = Vec3f::reflect(state.velocity, plane.normal) * friction;
  }

  if (Gm_Absf(plane.nDotU) < 0.35f) {
    state.lastWallBumpTime = state.frameStartTime;
  }
}

internal void resolveAllCollisions(GmContext* context, GameState& state) {
  auto& player = getPlayer();
  float playerRadius = player.scale.x;
  bool isFalling = state.previousPlayerPosition.y - player.position.y > 0.f;

  for (auto& plane : state.collisionPlanes) {
    Vec3f lineStart = player.position + plane.normal * playerRadius;
    Vec3f lineEnd = player.position - plane.normal * playerRadius;
    auto collision = getLinePlaneCollision(lineStart, lineEnd, plane);

    if (collision.hit) {
      resolveSingleCollision(context, state, collision);
    } else if (isFalling && plane.nDotU > 0.6f) {
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
}

namespace MovementSystem {
  void handlePlayerMovementInput(GmContext* context, GameState& state, float dt) {
    auto& input = getInput();
    auto& player = getPlayer();

    auto rate = 5000.f * dt;
    auto initialVelocity = state.velocity;

    Vec3f forward = getCamera().orientation.getDirection().xz().unit();
    Vec3f left = getCamera().orientation.getLeftDirection().xz().unit();

    if (state.velocity.y != 0.f) {
      // Reduce movement rate in midair
      rate *= 0.05f;
    } else if (state.velocity.xz().magnitude() > 500.f) {
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

    // Handle jump actions
    {
      float wallKickTimeDelta = state.lastJumpInputTime - state.lastWallBumpTime;

      if (input.isKeyHeld(Key::SPACE) && (
        // Allow jumping if on solid ground (@todo use state.isOnSolidGround?)
        state.velocity.y == 0.f ||
        // Allow wall kicks
        wallKickTimeDelta > 0 && wallKickTimeDelta < 0.2f
      )) {
        state.velocity.y = 500.f;
      }
    }
  }

  void handlePlayerMovementPhysics(GmContext* context, GameState& state, float dt) {
    auto& player = getPlayer();
    const float gravity = 750.f * dt;

    // Handle gravity/velocity
    state.velocity.y -= gravity;
    player.position += state.velocity * dt;

    resolveAllCollisions(context, state);

    if (state.velocity.y == 0.f && !state.isPlayerMovingThisFrame) {
      const float slowdown = 5.f;

      state.velocity.x = Gm_Lerpf(state.velocity.x, 0.f, slowdown * dt);
      state.velocity.z = Gm_Lerpf(state.velocity.z, 0.f, slowdown * dt);
    }

    commit(player);
  }
}