#include "Gamma.h"

#include "mesh_library/entities.h"

using namespace Gamma;

std::vector<MeshAsset> entity_meshes = {
  {
    .name = "dash-flower",
    .moving = true,
    .create = []() {
      return Mesh::Model("./game/assets/collectibles/dash-flower.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/collectibles/dash-flower.png",
      .animation = {
        .type = PresetAnimationType::LEAF,
        .speed = 3.f,
        .factor = 2.f
      },
      .emissivity = 0.3f,
      .roughness = 0.2f,
      .useMipmaps = false
    }
  },
  {
    .name = "air-dash-landing-point",
    .dynamic = true,
    .defaultColor = Vec3f(1.f, 0, 0),
    .defaultScale = Vec3f(60.f),
    .hitboxScale = Vec3f(1.f, 0.2f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/entities/target.obj");
    },
    .attributes = {
      .emissivity = 0.8f,
      .roughness = 0.2f,
    }
  },
  {
    // @todo remove
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
  },
  {
    .name = "torii-platform",
    .moving = true,
    .create = []() {
      return Mesh::Cube();
    },
    .attributes = {
      .emissivity = 0.3f,
      .roughness = 0.2f
    }
  },
  {
    .name = "vending-machine",
    .hitboxScale = Vec3f(0.6f, 1.f, 0.4f),
    .create = []() {
      return Mesh::Model("./game/assets/entities/vending-machine.obj");
    },
    .attributes = {
      .texture = "./game/assets/entities/vending-machine.png",
      .emissivity = 0.3f
    }
  }
};