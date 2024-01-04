#pragma once

#include "Gamma.h"

#include "game.h"

namespace EntitySystem {
  void initializeGameEntities(GmContext* context, GameState& state);
  void handleGameEntities(GmContext* context, GameState& state, float dt);
  void handleOcean(GmContext* context);
  bool isInteractingWithEntity(GmContext* context, GameState& state);
}