#pragma once

#include "Gamma.h"

#include "game.h"

namespace CameraSystem {
  void initializeGameCamera(GmContext* context, GameState& state);
  void handleGameCamera(GmContext* context, GameState& state, float dt);
  Gamma::Vec3f getLookAtTargetPosition(GmContext* context, GameState& state);
}