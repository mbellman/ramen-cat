#pragma once

#include <functional>
#include <string>
#include <vector>

#include "Gamma.h"

#include "game.h"

namespace World {
  void initializeGameWorld(GmContext* context, GameState& state);
  void rebuildDynamicMeshes(GmContext* context);
  void rebuildDynamicCollisionPlanes(GmContext* context, GameState& state);
  void loadLevel(GmContext* context, GameState& state, const std::string& levelName);
}