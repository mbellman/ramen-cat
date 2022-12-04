#pragma once

#include "Gamma.h"

#include "game.h"
#include "macros.h"

namespace CameraSystem {
  void initializeGameCamera(GmContext* context, GameState& state);
  void handleGameCamera(GmContext* context, GameState& state, float dt);
}