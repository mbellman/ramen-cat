#include "Gamma.h"

#include "mesh_library/spinners.h"

using namespace Gamma;

std::vector<MeshAsset> spinner_meshes = {
  {
    .name = "windmill-wheel",
    .moving = true,
    .defaultScale = Vec3f(75.f),
    .hitboxScale = Vec3f(1.f, 1.f, 0.1f),
    .create = []() {
      return Mesh::Model("./game/assets/windmill-wheel.obj");
    },
    .attributes = {
      .texture = "./game/assets/wood-beam.png",
      .maxCascade = 4
    }
  },
  {
    .name = "windmill-wheel-2",
    .moving = true,
    .defaultScale = Vec3f(75.f),
    .hitboxScale = Vec3f(1.f, 1.f, 0.1f),
    .create = []() {
      return Mesh::Model("./game/assets/windmill-wheel-2.obj");
    },
    .attributes = {
      .texture = "./game/assets/wood-beam.png",
      .maxCascade = 4
    }
  },

  {
    .name = "water-wheel",
    .moving = true,
    .hitboxScale = Vec3f(0.2f, 1.f, 1.f),
    .maxInstances = 5,
    .create = []() {
      return Mesh::Model("./game/assets/uniques/water-wheel.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },
  {
    .name = "solar-turbine",
    .moving = true,
    .hitboxScale = Vec3f(1.f, 0.2f, 1.f),
    .maxInstances = 100,
    .create = []() {
      return Mesh::Model("./game/assets/decorations/solar-turbine.obj");
    },
    .attributes = {
      .texture = "./game/assets/decorations/solar-turbine.png",
      .maxCascade = 4,
      .roughness = 0.2f
    }
  },
  {
    .name = "generator",
    .hitboxScale = Vec3f(1.f, 0.1f, 1.f),
    .maxInstances = 100,
    .create = []() {
      return Mesh::Model("./game/assets/decorations/generator.obj");
    },
    .attributes = {
      .texture = "./game/assets/decorations/generator.png",
      .roughness = 0.2f
    },
    .pieces = {
      {
        .name = "generator-fan",
        .moving = true,
        .create = []() {
          return Mesh::Model("./game/assets/decorations/exhaust-fan.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.4f);
          piece.position -= piece.rotation.getUpDirection() * source.scale.y * 0.2f;
        },
        .attributes = {
          .roughness = 0.1f
        }
      }
    }
  }
};