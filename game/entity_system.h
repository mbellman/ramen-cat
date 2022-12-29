#pragma once

#include "Gamma.h"

#include "game.h"

namespace EntitySystem {
  void initializeGameEntities(GmContext* context, GameState& state);
  void handleGameEntities(GmContext* context, GameState& state, float dt);
}