#include "Gamma.h"

#include "mesh_library/overworld_stairs.h"

using namespace Gamma;

std::vector<MeshAsset> overworld_stairs_meshes = {
  {
    .name = "concrete-rim",
    .create = []() {
      return Mesh::Model("./game/assets/buildings/concrete-rim.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  }
};