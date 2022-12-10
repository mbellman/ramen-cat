#include "movement_system.h"
#include "macros.h"

using namespace Gamma;

struct Plane {
  Vec3f p1, p2, p3, p4;
  Vec3f normal;
};

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

internal Collision getPlayerCollision(GmContext* context, GameState& state) {
  auto& player = getPlayer();
  
  // @temporary
  for (auto& platform : objects("platform")) {
    platform.color = Vec3f(1.f);

    commit(platform);
  }

  // @todo rewrite to check against game state collision planes
  for (auto& platform : objects("platform")) {
    auto& position = platform.position;
    auto& scale = platform.scale;

    // @todo cleanup
    Plane plane;

    plane.p1 = Vec3f(
      platform.position.x - platform.scale.x,
      platform.position.y + platform.scale.y,
      platform.position.z - platform.scale.z
    );

    plane.p2 = Vec3f(
      platform.position.x + platform.scale.x,
      platform.position.y + platform.scale.y,
      platform.position.z - platform.scale.z
    );

    plane.p3 = Vec3f(
      platform.position.x - platform.scale.x,
      platform.position.y + platform.scale.y,
      platform.position.z + platform.scale.z
    );

    plane.p4 = Vec3f(
      platform.position.x + platform.scale.x,
      platform.position.y + platform.scale.y,
      platform.position.z + platform.scale.z
    );

    plane.normal = Vec3f::cross(plane.p3 - plane.p1, plane.p2 - plane.p1).unit();

    Vec3f lineStart = player.position + plane.normal * player.scale.x;
    Vec3f lineEnd = player.position - plane.normal * player.scale.x;

    auto collision = getLinePlaneCollision(lineStart, lineEnd, plane);

    if (collision.hit) {
      // @temporary
      platform.color = Vec3f(0.3f, 1.f, 0.3f);
      commit(platform);

      return collision;
    }
  }

  return Collision();
}

internal void handleCollisions(GmContext* context, GameState& state) {
  auto& player = getPlayer();
  auto collision = getPlayerCollision(context, state);

  if (collision.hit) {
    Vec3f movementDelta = (player.position - state.previousPlayerPosition) * collision.plane.normal;

    player.position = collision.point + collision.plane.normal * player.scale.x;

    if (movementDelta.magnitude() > 1.f) {
      // @todo reflect velocity vector along the collision plane normal
      state.velocity.y *= -0.2f;
    } else {
      state.velocity.y = 0.f;
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

    auto moving = state.velocity != initialVelocity;

    if (input.isKeyHeld(Key::SPACE) && state.velocity.y == 0.f) {
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

    // @todo dampen velocity more robustly
    const float frictionCoefficient = 0.9f;

    if (state.velocity.y == 0.f) {
      state.velocity.x *= frictionCoefficient;
      state.velocity.z *= frictionCoefficient;
    }

    state.previousPlayerPosition = player.position;

    commit(player);
  }
}