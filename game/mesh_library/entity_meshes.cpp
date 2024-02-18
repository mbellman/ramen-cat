#include "Gamma.h"

#include "mesh_library/entity_meshes.h"

using namespace Gamma;

std::vector<MeshAsset> entity_meshes = {
  {
    .name = "air-dash-landing-point",
    .dynamic = true,
    .defaultColor = Vec3f(1.f, 0, 0),
    .defaultScale = Vec3f(60.f),
    .create = []() {
      return Mesh::Model("./game/assets/entities/target.obj");
    },
    .attributes = {
      .emissivity = 0.8f,
      .roughness = 0.2f,
    }
  },
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
    .attributes = {
      .texture = "./game/assets/wood-plank.png",
      .normals = "./game/assets/wood-plank-normals.png"
    },
    .pieces = {
      {
        .name = "jump-pad-platform",
        .moving = true,
        .create = []() {
          return Mesh::Model("./game/assets/entities/jump-pad.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.7f);
        },
        .attributes = {
          .texture = "./game/assets/wood-plank.png",
          .normals = "./game/assets/wood-plank-normals.png"
        }
      }
    }
  }
};