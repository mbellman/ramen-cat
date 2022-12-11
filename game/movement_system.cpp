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
    float d = Vec3f::dot(plane.normal, plane.p1);
    float t = (d - Vec3f::dot(plane.normal, lineStart)) / Vec3f::dot(plane.normal, line);
    Vec3f intersection = lineStart + line * t;

    if (
      // @todo this does not take multi-axis rotation into account
      isInBetween(intersection.x, plane.p1.x, plane.p4.x) &&
      isInBetween(intersection.y, plane.p1.y, plane.p4.y) &&
      isInBetween(intersection.z, plane.p1.z, plane.p4.z) &&
      isInBetween(intersection.x, lineStart.x, lineEnd.x) &&
      isInBetween(intersection.y, lineStart.y, lineEnd.y) &&
      isInBetween(intersection.z, lineStart.z, lineEnd.z)
    ) {
      collision.plane = plane;
      collision.point = intersection;
      collision.hit = true;
    }
  }

  return collision;
}

internal void handleCollisions(GmContext* context, GameState& state) {
  auto& player = getPlayer();

  for (auto& plane : state.collisionPlanes) {
    Vec3f lineStart = player.position + plane.normal * player.scale.x;
    Vec3f lineEnd = player.position - plane.normal * player.scale.x;
    auto collision = getLinePlaneCollision(lineStart, lineEnd, plane);

    if (collision.hit) {
      float uDotN = Vec3f::dot(Vec3f(0, 1.f, 0), collision.plane.normal);

      player.position = collision.point + collision.plane.normal * player.scale.x;

      if (uDotN  > 0.8f) {
        // If the collision plane normal points sufficiently upward,
        // treat it as a solid ground collision and stop falling
        state.velocity.y = 0.f;
        state.lastSolidGroundPosition = player.position;
        state.lastTimeOnSolidGround = state.frameStartTime;
      } else {
        // Adjust bounce friction based on whether we're falling,
        // or walking solid ground (e.g. running into a wall)
        Vec3f friction = state.velocity.y == 0.f ? Vec3f(0.5f) : Vec3f(0.8f, 1.f, 0.8f);

        state.velocity = Vec3f::reflect(state.velocity, collision.plane.normal) * friction;
      }

      if (uDotN  < 0.2f) {
        state.lastWallBumpTime = state.frameStartTime;
      }
    }
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

    if (input.isKeyHeld(Key::SPACE) && (
      state.velocity.y == 0.f ||
      state.frameStartTime - state.lastWallBumpTime < 150000
    )) {
      state.velocity.y = 500.f;
    }
  }

  void handlePlayerMovementPhysics(GmContext* context, GameState& state, float dt) {
    auto& player = getPlayer();
    const float gravity = 750.f * dt;

    // Handle gravity/velocity
    state.velocity.y -= gravity;
    player.position += state.velocity * dt;

    handleCollisions(context, state);

    if (state.velocity.y == 0.f && !state.isPlayerMovingThisFrame) {
      const float slowdown = 5.f;

      state.velocity.x = Gm_Lerpf(state.velocity.x, 0.f, slowdown * dt);
      state.velocity.z = Gm_Lerpf(state.velocity.z, 0.f, slowdown * dt);
    }

    state.previousPlayerPosition = player.position;

    commit(player);
  }
}