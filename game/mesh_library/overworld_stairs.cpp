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
  },
  {
    .name = "concrete-b1",
    .create = []() {
      return Mesh::Model("./game/assets/buildings/concrete-b1.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },
  {
    .name = "round-tower",
    .dynamic = true,
    .create = []() {
      return Mesh::Model("./game/assets/buildings/round-tower-base.obj");
    },
    .attributes = {
      .maxCascade = 4
    },
    .pieces = {
      {
        .name = "round-tower-base",
        .create = []() {
          return Mesh::Model("./game/assets/buildings/round-tower-base.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(1.f, 0.8f, 0.6f);
        }
      },
      {
        .name = "round-tower-supports",
        .create = []() {
          return Mesh::Model("./game/assets/buildings/round-tower-supports.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(1.f, 0.6f, 0.2f);
        }
      },
      {
        .name = "round-tower-roof",
        .create = []() {
          return Mesh::Model("./game/assets/buildings/round-tower-roof.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.8f, 0.4f, 0.2f);
        }
      }
    }
  }
};