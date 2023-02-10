#pragma once

#include <functional>
#include <vector>

#include "Gamma.h"

#include "game.h"

namespace World {
  void initializeGameWorld(GmContext* context, GameState& state);
  void rebuildDynamicMeshes(GmContext* context);
}