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
    .name = "b4",
    .hitboxScale = Vec3f(0.8f, 1.f, 0.8f),
    .create = []() {
      return Mesh::Model("./game/assets/buildings/b4-levels.obj");
    },
    .attributes = {
      .maxCascade = 4,
      .roughness = 0.2f
    },
    .pieces = {
      {
        .name = "b4-columns",
        .create = []() {
          return Mesh::Model("./game/assets/buildings/b4-columns.obj");
        }
      },
      {
        .name = "b4-windows",
        .create = []() {
          return Mesh::Model("./game/assets/buildings/b4-windows.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.4f, 0.7f, 1.f);
        },
        .attributes = {
          .roughness = 0.1f
        }
      }
    }
  },
  {
    .name = "b5",
    .hitboxScale = Vec3f(0.6f, 1.f, 0.6f),
    .create = []() {
      return Mesh::Model("./game/assets/buildings/b5-base.obj");
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
  {
    .name = "runway",
    .create = []() {
      return Mesh::Model("./game/assets/decorations/runway.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  }
};