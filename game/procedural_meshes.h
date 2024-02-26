#pragma once

#include "Gamma.h"

#include "game.h"

namespace ProceduralMeshes {
  void buildWireFromStartToEnd(GmContext* context, const Gamma::Vec3f& start, const Gamma::Vec3f& end, const float scale, const Gamma::Vec3f& color);
  void rebuildProceduralMeshes(GmContext* context);
  void handleProceduralMeshes(GmContext* context, GameState& state, float dt);
}