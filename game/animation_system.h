#pragma once

#include "Gamma.h"

#include "game.h"

namespace AnimationSystem {
  void initializePlayerRig(GmContext* context, GameState& state);
  void handleAnimations(GmContext* context, GameState& state, float dt);
}