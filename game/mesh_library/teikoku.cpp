#include "Gamma.h"

#include "mesh_library/teikoku.h"

using namespace Gamma;

std::vector<MeshAsset> teikoku_meshes = {
  {
    .name = "green-curve-sign",
    .create = []() {
      return Mesh::Model("./game/assets/teikoku/green-curve-sign.obj");
    },
    .attributes = {
      .roughness = 0.9f
    }
  },
  {
    .name = "hanging-light",
    .create = []() {
      return Mesh::Model("./game/assets/teikoku/hanging-light.obj");
    },
    .attributes = {
      .roughness = 0.1f
    },
    .pieces = {
      {
        .name = "hanging-light-fan",
        .moving = true,
        .create = []() {
          return Mesh::Model("./game/assets/teikoku/hanging-light-fan.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.4f, 0.6f, 0.3f);
        },
        .attributes = {
          .roughness = 0.4f
        }
      }
    }
  },
  {
    .name = "blue-arch",
    .hitboxScale = Vec3f(1.f, 1.f, 0.2f),
    .create = []() {
      return Mesh::Model("./game/assets/teikoku/blue-arch.obj");
    },
    .attributes = {
      .roughness = 0.8f
    },
    .pieces = {
      {
        .name = "blue-arch-rim",
        .moving = true,
        .create = []() {
          return Mesh::Model("./game/assets/teikoku/blue-arch-rim.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.2f, 0.6f, 0.6f);
        },
        .attributes = {
          .roughness = 0.2f
        }
      }
    }
  }
};