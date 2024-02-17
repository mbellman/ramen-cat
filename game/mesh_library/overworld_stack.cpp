#include "Gamma.h"

#include "mesh_library/overworld_stack.h"

using namespace Gamma;

std::vector<MeshAsset> overworld_stack_meshes = {
  {
    .name = "cylinder-tower",
    .hitboxScale = Vec3f(0.1f, 1.f, 0.1f),
    .create = []() {
      return Mesh::Model("./game/assets/buildings/cylinder-tower.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },
  {
    .name = "cylinder-1",
    .create = []() {
      return Mesh::Model("./game/assets/buildings/cylinder-1.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },
  {
    .name = "cylinder-2",
    .create = []() {
      return Mesh::Model("./game/assets/buildings/cylinder-2.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },
  {
    .name = "dome-1",
    .hitboxScale = Vec3f(1.f, 0.2f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/buildings/dome-1.obj");
    },
    .attributes = {
      .maxCascade = 4,
      .roughness = 0.2f
    }
  },
  {
    .name = "dome-2",
    .hitboxScale = Vec3f(1.f, 0.2f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/buildings/dome-2.obj");
    },
    .attributes = {
      .maxCascade = 4,
      .roughness = 0.1f
    }
  },
};