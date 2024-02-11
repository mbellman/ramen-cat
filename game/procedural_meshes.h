#pragma once

#include "Gamma.h"

#include "game.h"

namespace ProceduralMeshes {
  void rebuildProceduralMeshes(GmContext* context);
  void handleProceduralMeshes(GmContext* context, GameState& state, float dt);
}