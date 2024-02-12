#include "Gamma.h"

#include "mesh_library/entity_meshes.h"

using namespace Gamma;

std::vector<MeshAsset> entity_meshes = {
  {
    .name = "boost-pad",
    .hitboxScale = Vec3f(0.5f, 0.2f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/entities/boost-pad.obj");
    }
  },
  {
    .name = "jump-pad",
    .hitboxScale = Vec3f(1.f, 0.2f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/entities/jump-pad-base.obj");
    },
    .pieces = {
      {
        .name = "jump-pad-platform",
        .create = []() {
          return Mesh::Model("./game/assets/entities/jump-pad.obj");
        }
      }
    }
  }
};