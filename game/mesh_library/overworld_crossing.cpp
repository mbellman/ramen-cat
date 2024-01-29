#include "Gamma.h"

#include "mesh_library/overworld_crossing.h"

using namespace Gamma;

std::vector<MeshAsset> overworld_crossing_meshes = {
  {
    .name = "bridge-1",
    .dynamic = true,
    .hitboxScale = Vec3f(0.7f, 0.7f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/buildings/bridge-1.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },
};