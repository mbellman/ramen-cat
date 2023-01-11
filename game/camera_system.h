#pragma once

#include "Gamma.h"

#include "game.h"

namespace CameraSystem {
  void initializeGameCamera(GmContext* context, GameState& state);
  void handleGameCamera(GmContext* context, GameState& state, float dt);
  void setTargetCameraState(GmContext* context, GameState& state, const CameraState& cameraState);
  void restoreOriginalCameraState(GmContext* context, GameState& state);
}