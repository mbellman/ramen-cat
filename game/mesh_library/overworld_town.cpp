#include "Gamma.h"

#include "mesh_library/overworld_town.h"

using namespace Gamma;

std::vector<MeshAsset> overworld_town_meshes = {
  {
    .name = "town-house",
    .hitboxScale = Vec3f(1.f, 0.5f, 0.5f),
    .create = []() {
      return Mesh::Model("./game/assets/buildings/town-house-frame.obj");
    },
    .attributes = {
      .maxCascade = 4
    },
    .pieces = {
      {
        .name = "town-house-roof",
        .create = []() {
          return Mesh::Model("./game/assets/buildings/town-house-roof.obj");
        },
        .attributes = {
          .texture = "./game/assets/buildings/town-house-roof.png",
          .roughness = 0.4f
        }
      },
      {
        .name = "town-house-supports",
        .create = []() {
          return Mesh::Model("./game/assets/buildings/town-house-supports.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.8f, 0.6f, 0.4f);
        },
        .attributes = {
          .roughness = 0.4f
        }
      }
    }
  }
};