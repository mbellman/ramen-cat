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
  {
    .name = "storefront-1",
    .hitboxScale = Vec3f(1.f, 0.7f, 0.7f),
    .create = []() {
      return Mesh::Model("./game/assets/shops/storefront-1.obj");
    },
    .attributes = {
      .texture = "./game/assets/wood-beam.png",
      .maxCascade = 4
    },
    .pieces = {
      {
        .name = "storefront-1-pieces",
        .create = []() {
          return Mesh::Model("./game/assets/shops/storefront-1-pieces.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.6f);
        },
        .attributes = {
          .texture = "./game/assets/wood-beam.png"
        }
      },
      {
        .name = "storefront-1-cover",
        .create = []() {
          return Mesh::Model("./game/assets/shops/storefront-1-cover.obj");
        },
        .attributes = {
          .texture = "./game/assets/shops/storefront-1-cover.png"
        }
      }
    }
  },
  {
    .name = "storefront-2",
    .create = []() {
      return Mesh::Model("./game/assets/shops/storefront-2.obj");
    },
    .attributes = {
      .texture = "./game/assets/wood-beam.png",
      .maxCascade = 4
    },
    .pieces = {
      {
        .name = "storefront-2-canopy",
        .create = []() {
          return Mesh::Model("./game/assets/shops/storefront-2-canopy.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(
            0.3f + Gm_Modf(source.position.x, 0.7f),
            0.6f,
            1.f - Gm_Modf(source.position.z, 0.7f)
          );
        }
      },
      {
        .name = "storefront-2-flags",
        .create = []() {
          return Mesh::Model("./game/assets/shops/storefront-2-flags.obj");
        },
        .attributes = {
          .type = MeshType::PRESET_ANIMATED,
          .animation = {
            .type = PresetAnimationType::CLOTH
          }
        }
      }
    }
  },
  {
    .name = "storefront-3",
    .create = []() {
      return Mesh::Model("./game/assets/shops/storefront-3.obj");
    },
    .attributes = {
      .texture = "./game/assets/shops/red-wood.png",
    },
    .pieces = {
      {
        .name = "storefront-3-roof",
        .create = []() {
          return Mesh::Model("./game/assets/shops/storefront-3-roof.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.1f, 0.5f, 0.3f);
        },
        .attributes = {
          .roughness = 0.8f
        }
      },
      {
        .name = "storefront-3-flags",
        .create = []() {
          return Mesh::Model("./game/assets/shops/storefront-3-flags.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.6f, 0.8f, 1.f);
        },
        .attributes = {
          .type = MeshType::PRESET_ANIMATED,
          .animation = {
            .type = PresetAnimationType::CLOTH
          },
          .roughness = 0.8f
        }
      },
      {
        .name = "storefront-3-windows",
        .create = []() {
          return Mesh::Model("./game/assets/shops/storefront-3-windows.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.1f, 0.3f, 0.6f);
        },
        .attributes = {
          .roughness = 0.1f
        }
      }
    }
  },
  {
    .name = "storefront-4",
    .create = []() {
      return Mesh::Model("./game/assets/shops/storefront-4.obj");
    },
    .attributes = {
      .texture = "./game/assets/wood-beam.png",
    },
    .pieces = {
      {
        .name = "storefront-4-cover",
        .create = []() {
          return Mesh::Model("./game/assets/shops/storefront-4-cover.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.1f, 0.5f, 0.8f);
        },
        .attributes = {
          .roughness = 0.8f
        }
      },
      {
        .name = "storefront-4-flags",
        .create = []() {
          return Mesh::Model("./game/assets/shops/storefront-4-flags.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.6f, 1.f, 0.8f);
        },
        .attributes = {
          .type = MeshType::PRESET_ANIMATED,
          .animation = {
            .type = PresetAnimationType::CLOTH
          },
          .roughness = 0.8f
        }
      }
    }
  },
  {
    .name = "storefront-5",
    .create = []() {
      return Mesh::Model("./game/assets/shops/storefront-5.obj");
    },
    .attributes = {
      .texture = "./game/assets/wood-beam.png",
    },
    .pieces = {
      {
        .name = "storefront-5-cover",
        .create = []() {
          return Mesh::Model("./game/assets/shops/storefront-5-cover.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(1.f, 0.8f, 0.6f);
        },
        .attributes = {
          .roughness = 0.8f
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