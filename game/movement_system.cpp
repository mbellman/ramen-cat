#include "Gamma.h"

#include "movement_system.h"
#include "macros.h"

using namespace Gamma;

namespace MovementSystem {
  void handlePlayerMovement(GmContext* context, GameState& state, float dt) {
    auto& player = getPlayer();
    const float gravity = 750.f * dt;

    // Handle gravity/velocity
    state.velocity.y -= gravity;
    player.position += state.velocity * dt;

    // @todo resolve collisions more robustly
    if (player.position.y < 20.f) {
      float delta = state.lastFrameY - player.position.y;

      if (state.lastFrameY > 20.f && delta > 2.f) {
        player.position.y = 20.f;
        state.velocity.y *= -0.2f;
      } else {
        player.position.y = 20.f;
        state.velocity.y = 0.f;
      }
    }

    // @todo dampen velocity more robustly
    if (player.position.y == 20.f) {
      state.velocity.x *= 0.9f;
      state.velocity.z *= 0.9f;
    }

    state.lastFrameY = player.position.y;

    commit(player);
  }
}