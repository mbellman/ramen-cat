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
    }
  }
};