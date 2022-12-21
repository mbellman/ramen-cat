#pragma once

#include "Gamma.h"

#include "game.h"

namespace World {
  void initializeGameWorld(GmContext* context, GameState& state);
  void rebuildCollisionPlanes(GmContext* context, GameState& state);
}