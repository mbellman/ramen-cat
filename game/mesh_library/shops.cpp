#include "Gamma.h"

#include "mesh_library/shops.h"

using namespace Gamma;

std::vector<MeshAsset> shop_meshes = {
  {
    .name = "shop-1",
    .hitboxScale = Vec3f(1.f, 0.3f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/shop-1.obj");
    },
    .attributes = {
      .texture = "./game/assets/shop-1.png"
    }
  },
  {
    .name = "ramen-stall",
    .create = []() {
      return Mesh::Model("./game/assets/ramen-stall.obj");
    },
    .attributes = {
      .texture = "./game/assets/wood-beam.png"
    }
  },
  {
    .name = "ramen-stall-2",
    .create = []() {
      return Mesh::Model("./game/assets/ramen-stall-2.obj");
    },
    .attributes = {
      .texture = "./game/assets/ramen-stall-2.png"
    }
  },
  {
    .name = "food-stall-1",
    .create = []() {
      return Mesh::Model("./game/assets/shops/food-stall-1.obj");
    },
    .attributes = {
      .texture = "./game/assets/wood-beam.png"
    },
    .pieces = {
      {
        .name = "food-stall-1-canopy",
        .create = []() {
          return Mesh::Model("./game/assets/shops/food-stall-1-canopy.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(1.f - Gm_Modf(source.position.x, 10.f) * 0.05f, 1.f, 1.f - Gm_Modf(source.position.z, 10.f) * 0.05f);
        },
        .attributes = {
          .type = MeshType::PRESET_ANIMATED,
          .animation = {
            .type = PresetAnimationType::CLOTH,
            .factor = 0.5f
          }
        }
      }
    }
  },

  // @todo move to procedural
  {
    .name = "ramen-bowl",
    .hitboxScale = Vec3f(1.f, 0.4f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/ramen-bowl.obj");
    },
    .attributes = {
      .texture = "./game/assets/ramen-bowl.png",
      .roughness = 0.4f
    }
  }
};