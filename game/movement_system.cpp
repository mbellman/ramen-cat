#include "movement_system.h"
#include "macros.h"

using namespace Gamma;

// @todo refactor to check against game state collision volumes
internal CollisionVolume* getCollision(GmContext* context, GameState& state) {
  auto& player = getPlayer();

  for (auto& platform : objects("platform")) {
    auto& position = platform.position;
    auto& scale = platform.scale;

    // @todo cleanup
    if (player.position.x + player.scale.x < position.x - scale.x || player.position.x - player.scale.x > position.x + scale.x) {
      continue;
    }

    if (player.position.z + player.scale.z < position.z - scale.z || player.position.z - player.scale.z > position.z + scale.z) {
      continue;
    }

    if (player.position.y + player.scale.y < position.y - scale.y || player.position.y - player.scale.y > position.y + scale.y) {
      continue;
    }

    // @todo return a pointer to a collision volume stored in game state
    auto* collision = new CollisionVolume();

    collision->corner1 = platform.position + platform.scale;
    collision->corner2 = platform.position - platform.scale;

    return collision;
  }

  return nullptr;
}

internal void handleCollisions(GmContext* context, GameState& state) {
  auto& player = getPlayer();
  auto* collision = getCollision(context, state);

  if (collision != nullptr) {
    float fallDelta = state.previousPlayerPosition.y - player.position.y;
    auto& corner1 = collision->corner1;
    auto& corner2 = collision->corner2;

    player.position.y = corner1.y + player.scale.y;

    if (state.previousPlayerPosition.y > corner1.y && fallDelta > 1.f) {
      state.velocity.y *= -0.2f;
    } else {
      state.velocity.y = 0.f;
    }

    // @temporary
    delete collision;
  }
}

namespace MovementSystem {
  void handlePlayerMovement(GmContext* context, GameState& state, float dt) {
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