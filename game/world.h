#pragma once

#include <functional>
#include <vector>

#include "Gamma.h"

#include "game.h"

typedef std::function<Gamma::Mesh*()> MeshCreator;

struct MeshAsset {
  std::string name;
  bool flat = false;
  bool dynamic = false;
  Gamma::Vec3f defaultColor = Gamma::Vec3f(0, 0, 1.f);
  Gamma::Quaternion defaultRotation = Gamma::Quaternion(1.f, 0, 0, 0);
  MeshCreator create = nullptr;
  Gamma::MeshAttributes attributes;
};

namespace World {
  extern std::vector<MeshAsset> meshAssets;
  extern std::vector<MeshAsset> dynamicMeshPieces;

  void initializeGameWorld(GmContext* context, GameState& state);
  void rebuildDynamicMeshes(GmContext* context);
}