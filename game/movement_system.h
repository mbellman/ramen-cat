#pragma once

#include "Gamma.h"

#include "game.h"

namespace MovementSystem {
  void handlePlayerMovement(GmContext* context, GameState& state, float dt);
}