#pragma once

#include "Gamma.h"
#include "game.h"

namespace EffectsSystem {
  void initializeGameEffects(GmContext* context, GameState& state);
  void handleGameEffects(GmContext* context, GameState& state, float dt);
}