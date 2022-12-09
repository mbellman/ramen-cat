#pragma once

#include "Gamma.h"

#include "game.h"

namespace MovementSystem {
  void handlePlayerMovementInput(GmContext* context, GameState& state, float dt);
  void handlePlayerMovementPhysics(GmContext* context, GameState& state, float dt);
}