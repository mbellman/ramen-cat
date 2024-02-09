#pragma once

#include "Gamma.h"

#include "game.h"

namespace VehicleSystem {
  void rebuildVehicleTracks(GmContext* context, GameState& state);
  void handleVehicles(GmContext* context, GameState& state, float dt);
}