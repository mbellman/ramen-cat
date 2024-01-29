#pragma once

#include "mesh_library/mesh_asset.h"

namespace GameMeshes {
  extern std::vector<MeshAsset> meshAssets;
  extern std::vector<MeshAsset> dynamicMeshPieces;

  void loadAllMeshAssets();
}