#include "Gamma.h"

#include "mesh_library/uniques.h"

using namespace Gamma;

std::vector<MeshAsset> unique_meshes = {
  {
    .name = "big-tree-trunk",
    .hitboxScale = Vec3f(0.3f, 1.f, 0.3f),
    .create = []() {
      return Mesh::Model("./game/assets/uniques/big-tree-trunk.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },
  {
    .name = "water-tower-pipe",
    .hitboxScale = Vec3f(0.2f, 0.3f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/uniques/water-tower-pipe.obj");
    },
    .attributes = {
      .maxCascade = 4,
      .roughness = 0.2f
    }
  },
  {
    .name = "lower-lake",
    .hitboxScale = Vec3f(1.f, 0.1f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/uniques/lower-lake.obj");
    },
    .attributes = {
      .type = MeshType::OCEAN
    }
  }
};