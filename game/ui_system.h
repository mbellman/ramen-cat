#pragma once

#include "Gamma.h"

#include "game.h"

namespace UISystem {
  void initializeUI(GmContext* context, GameState& state);
  void handleUI(GmContext* context, GameState& state, float dt);
}