#pragma once

#include <functional>
#include <vector>

#include "Gamma.h"

#include "game.h"

typedef std::function<Gamma::Mesh*()> MeshCreator;

struct MeshAsset {
  std::string name;
  bool dynamic = false;
  Gamma::Vec3f defaultColor = Gamma::Vec3f(0, 0, 1.f);
  // @todo implement this
  Gamma::Quaternion defaultRotation = Gamma::Quaternion(1.f, 0, 0, 0);
  Gamma::Vec3f scalingFactor = Gamma::Vec3f(1.f);
  u16 maxInstances = 1000;
  MeshCreator create = nullptr;
  Gamma::MeshAttributes attributes;
};

namespace World {
  extern std::vector<MeshAsset> meshAssets;
  extern std::vector<MeshAsset> dynamicMeshPieces;

  void initializeGameWorld(GmContext* context, GameState& state);
  void rebuildDynamicMeshes(GmContext* context);
}