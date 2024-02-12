#include "Gamma.h"

#include "mesh_library/entity_meshes.h"

using namespace Gamma;

std::vector<MeshAsset> entity_meshes = {
  {
    .name = "boost-pad",
    .hitboxScale = Vec3f(0.5f, 0.05f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/entities/boost-pad.obj");
    }
  }
};