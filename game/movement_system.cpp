#include "movement_system.h"
#include "macros.h"

using namespace Gamma;

// @todo refactor to check against collision volumes
internal bool isPlayerOnPlatform(GmContext* context, GameState& state) {
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

    return true;
  }

  return false;
}

internal void handleCollisions(GmContext* context, GameState& state) {
  auto& player = getPlayer();

  // @todo handle collisions more robustly
  if (player.position.y < 20.f && isPlayerOnPlatform(context, state)) {
    float delta = state.previousPlayerPosition.y - player.position.y;

    if (state.previousPlayerPosition.y > 20.f && delta > 2.f) {
      player.position.y = 20.f;
      state.velocity.y *= -0.2f;
    } else {
      player.position.y = 20.f;
      state.velocity.y = 0.f;
    }
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

    if (player.position.y == 20.f) {
      state.velocity.x *= frictionCoefficient;
      state.velocity.z *= frictionCoefficient;
    }

    state.previousPlayerPosition = player.position;

    commit(player);
  }
}