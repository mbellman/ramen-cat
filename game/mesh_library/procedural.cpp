#include "Gamma.h"

#include "mesh_library/procedural.h"

using namespace Gamma;

std::vector<MeshAsset> procedural_meshes = {
  {
    .name = "onigiri-strip",
    .dynamic = true,
    .defaultColor = Vec3f(0.5f, 1.f, 0.5f),
    .create = []() {
      return Mesh::Cube();
    }
  },
  {
    .name = "onigiri-spawn",
    .dynamic = true,
    .defaultColor = Vec3f(0.5f, 1.f, 0.5f),
    .create = []() {
      return Mesh::Sphere(8);
    }
  },
  {
    .name = "nitamago-strip",
    .dynamic = true,
    .defaultColor = Vec3f(1.f, 1.f, 0.5f),
    .create = []() {
      return Mesh::Cube();
    }
  },
  {
    .name = "nitamago-spawn",
    .dynamic = true,
    .defaultColor = Vec3f(0.5f, 1.f, 0.5f),
    .create = []() {
      return Mesh::Sphere(8);
    }
  },
  {
    .name = "chashu-strip",
    .dynamic = true,
    .defaultColor = Vec3f(1.f, 0.6f, 0.2f),
    .create = []() {
      return Mesh::Cube();
    }
  },
  {
    .name = "chashu-spawn",
    .dynamic = true,
    .defaultColor = Vec3f(1.f, 0.6f, 0.2f),
    .create = []() {
      return Mesh::Sphere(8);
    }
  },
  {
    .name = "narutomaki-strip",
    .dynamic = true,
    .defaultColor = Vec3f(1.f, 0.5f, 1.f),
    .create = []() {
      return Mesh::Cube();
    }
  },
  {
    .name = "narutomaki-spawn",
    .dynamic = true,
    .defaultColor = Vec3f(0.5f, 1.f, 0.5f),
    .create = []() {
      return Mesh::Sphere(8);
    }
  },

  {
    .name = "plant-strip",
    .dynamic = true,
    .defaultColor = Vec3f(0.5f, 1.f, 0.6f),
    .create = []() {
      return Mesh::Cube();
    }
  },
  {
    .name = "concrete-stack",
    .dynamic = true,
    .create = []() {
      return Mesh::Cube();
    }
  },
  {
    .name = "mini-house",
    .dynamic = true,
    .defaultScale = Vec3f(180.f, 200.f, 100.f),
    .scalingFactor = Vec3f(0.f),
    .create = []() {
      return Mesh::Cube();
    }
  },
  {
    .name = "mini-house-double",
    .dynamic = true,
    .defaultScale = Vec3f(180.f, 200.f, 200.f),
    .scalingFactor = Vec3f(0.f),
    .create = []() {
      return Mesh::Cube();
    }
  },
  {
    .name = "wood-building-1",
    .hitboxScale = Vec3f(1.f, 0.3f, 0.5f),
    .create = []() {
      return Mesh::Model("./game/assets/buildings/wood-building-1.obj");
    },
    .attributes = {
      .texture = "./game/assets/wood-beam.png",
      .maxCascade = 4
    }
  },

  // Pots/planters/etc.
  {
    .name = "wood-planter",
    .hitboxScale = Vec3f(1.f, 0.2f, 0.2f),
    .create = []() {
      return Mesh::Model("./game/assets/plants/wood-planter.obj");
    },
    .attributes = {
      .texture = "./game/assets/wood-beam.png"
    }
  },
  {
    .name = "plant-pot",
    .create = []() {
      return Mesh::Model("./game/assets/decorations/plant-pot.obj");
    },
    .attributes = {
      .roughness = 0.9f
    }
  },
  {
    .name = "square-pot",
    .create = []() {
      return Mesh::Model("./game/assets/decorations/square-pot.obj");
    },
    .attributes = {
      .roughness = 0.7f
    }
  }
};

std::vector<MeshAsset> procedural_mesh_parts = {
  // Collectables
  {
    .name = "onigiri",
    .moving = true,
    .defaultScale = Vec3f(40.f),
    .hitboxScale = Vec3f(0.7f, 1.f, 0.5f),
    .create = []() {
      return Mesh::Model("./game/assets/onigiri.obj");
    },
    .attributes = {
      .texture = "./game/assets/onigiri.png",
      .emissivity = 0.3f,
      .useMipmaps = false
    }
  },
  {
    .name = "nitamago",
    .moving = true,
    .defaultScale = Vec3f(40.f),
    .hitboxScale = Vec3f(0.8f, 0.8f, 0.6f),
    .create = []() {
      return Mesh::Model("./game/assets/nitamago.obj");
    },
    .attributes = {
      .texture = "./game/assets/nitamago.png",
      .emissivity = 0.3f,
      .useMipmaps = false
    }
  },
  {
    .name = "chashu",
    .moving = true,
    .defaultScale = Vec3f(40.f),
    .hitboxScale = Vec3f(0.8f, 0.8f, 0.2f),
    .create = []() {
      return Mesh::Model("./game/assets/chashu.obj");
    },
    .attributes = {
      .texture = "./game/assets/chashu.png",
      .emissivity = 0.3f,
      .useMipmaps = false
    }
  },
  {
    .name = "narutomaki",
    .moving = true,
    .defaultScale = Vec3f(40.f),
    .hitboxScale = Vec3f(0.8f, 0.8f, 0.2f),
    .create = []() {
      return Mesh::Model("./game/assets/collectibles/narutomaki.obj");
    },
    .attributes = {
      .texture = "./game/assets/collectibles/narutomaki.png",
      .emissivity = 0.3f,
      .useMipmaps = false
    }
  },

  /**
   * Effects
   * -------
   */
  {
    .name = "p_flower-spawn",
    .create = []() {
      return Mesh::Model("./game/assets/fx/flower-petals.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .animation = {
        .type = PresetAnimationType::LEAF
      },
      .emissivity = 0.3f
    }
  },
  {
    .name = "p_flower-center",
    .create = []() {
      return Mesh::Model("./game/assets/fx/flower-center.obj");
    },
    .attributes = {
      .emissivity = 0.3f
    }
  },
  {
    .name = "p_flower-leaves",
    .create = []() {
      return Mesh::Model("./game/assets/fx/flower-leaves.obj");
    },
    .attributes = {
      .emissivity = 0.3f
    }
  },

  /**
   * Procedural plants
   * -----------------
   */
  {
    .name = "p_croton",
    .hitboxScale = Vec3f(1.f, 0.3f, 1.f),
    .create = []() {
      return Mesh::Model(
        "./game/assets/plants/croton.obj"
      );
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/plants/croton.png",
      .animation = {
        .type = PresetAnimationType::LEAF,
        .factor = 3.f
      },
      .roughness = 0.3f
    }
  },
  {
    .name = "p_shrub",
    .maxInstances = 10000,
    .create = []() {
      return Mesh::Model({
        "./game/assets/plants/shrub.obj",
        "./game/assets/plants/shrub-lod.obj"
      });
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/plants/shrub.png",
      .animation = {
        .type = PresetAnimationType::FLOWER,
        .factor = 6.f
      },
      .roughness = 1.f
    }
  },
  {
    .name = "p_shrub-2",
    .maxInstances = 10000,
    .create = []() {
      return Mesh::Model({
        "./game/assets/plants/shrub.obj",
        "./game/assets/plants/shrub-lod.obj"
      });
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/plants/shrub-2.png",
      .animation = {
        .type = PresetAnimationType::FLOWER,
        .factor = 6.f
      },
      .roughness = 1.f
    }
  },
  {
    .name = "p_banana-plant",
    .hitboxScale = Vec3f(1.f, 0.3f, 1.f),
    .create = []() {
      return Mesh::Model({
        "./game/assets/plants/banana-plant.obj",
        "./game/assets/plants/banana-plant-lod.obj"
      });
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/plants/banana-plant.png",
      .animation = {
        .type = PresetAnimationType::LEAF,
        .factor = 3.f
      },
      .roughness = 0.4f
    }
  },
  {
    .name = "p_weeds",
    .hitboxScale = Vec3f(1.f, 1.f, 0.2f),
    .create = []() {
      return Mesh::Model("./game/assets/weeds.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/weeds.png",
      .maxCascade = 2,
      .animation = {
        .type = PresetAnimationType::FLOWER,
        .factor = 3.f
      },
      .emissivity = 0.1f,
      .roughness = 1.f
    }
  },
  {
    .name = "p_small-leaves",
    .maxInstances = 10000,
    .create = []() {
      return Mesh::Model("./game/assets/plants/small-leaves.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/plants/small-leaves.png",
      .maxCascade = 2,
      .animation = {
        .type = PresetAnimationType::FLOWER,
        .factor = 6.f
      }
    }
  },
  {
    .name = "p_small-flower",
    .maxInstances = 10000,
    .create = []() {
      return Mesh::Model("./game/assets/plants/small-flower.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/plants/small-flower.png",
      .maxCascade = 2,
      .animation = {
        .type = PresetAnimationType::FLOWER,
        .factor = 4.f
      }
    }
  },
  {
    .name = "p_small-cosmo",
    .maxInstances = 10000,
    .create = []() {
      return Mesh::Model("./game/assets/plants/small-flower.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/plants/small-cosmo.png",
      .maxCascade = 2,
      .animation = {
        .type = PresetAnimationType::FLOWER,
        .factor = 4.f
      }
    }
  },
  {
    .name = "p_bamboo",
    .create = []() {
      return Mesh::Model("./game/assets/plants/bamboo-plant.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/plants/bamboo-plant.png",
      .maxCascade = 2,
      .animation = {
        .type = PresetAnimationType::FLOWER,
        .factor = 4.f
      }
    }
  },

  // concrete-stack
  {
    .name = "p_concrete",
    .maxInstances = 10000,
    .create = []() {
      return Mesh::Cube();
    },
    .attributes = {
      .maxCascade = 4,
      .roughness = 0.9f
    }
  },

  // mini-house/mini-house-double
  {
    .name = "p_mini-house",
    .create = []() {
      return Mesh::Cube();
    },
    .attributes = {
      .maxCascade = 4,
      .roughness = 0.7f
    }
  },
  {
    .name = "p_mini-house-roof",
    .create = []() {
      return Mesh::Model({
        "./game/assets/japanese-roof.obj",
        "./game/assets/japanese-roof-lod.obj"
      });
    },
    .attributes = {
      .maxCascade = 4,
      .roughness = 0.3f
    }
  },
  {
    .name = "p_mini-house-wood-beam",
    .maxInstances = 10000,
    .create = []() {
      return Mesh::Model("./game/assets/procedural/mini-house-wood-beam.obj");
    },
    .attributes = {
      .texture = "./game/assets/wood-beam.png",
      .roughness = 0.5f
    }
  },
  {
    .name = "p_mini-house-window",
    .create = []() {
      return Mesh::Model("./game/assets/procedural/mini-house-window.obj");
    },
    .attributes = {
      .texture = "./game/assets/windows-1.png",
      .roughness = 0.5f
    }
  },
  {
    .name = "p_mini-house-board",
    .maxInstances = 10000,
    .create = []() {
      return Mesh::Plane(2);
    },
    .attributes = {
      .texture = "./game/assets/wood-beam.png",
      .normals = "./game/assets/wood-beam-normals.png",
      .roughness = 0.5f
    }
  },

  // balloon-windmill
  {
    .name = "p_balloon-windmill-blades",
    .create = []() {
      return Mesh::Model("./game/assets/decorations/balloon-windmill-blades.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },

  // town-sign
  {
    .name = "p_town-sign-spinner",
    .moving = true,
    .create = []() {
      return Mesh::Model("./game/assets/decorations/town-sign-spinner.obj");
    }
  },

  // food stalls/shops
  {
    .name = "p_ramen-sign",
    .create = []() {
      return Mesh::Model("./game/assets/ramen-sign.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/ramen-sign.png",
      .animation = {
        .type = PresetAnimationType::CLOTH
      },
      .roughness = 1.f,
      .useMipmaps = false
    }
  },
  {
    .name = "p_ramen-bowl",
    .create = []() {
      return Mesh::Model("./game/assets/ramen-bowl.obj");
    },
    .attributes = {
      .texture = "./game/assets/ramen-bowl.png",
      .roughness = 0.4f
    }
  },
  {
    .name = "p_dishes-1",
    .create = []() {
      return Mesh::Model("./game/assets/shops/dishes-1.obj");
    },
    .attributes = {
      .roughness = 0.2f
    }
  },
  {
    .name = "p_dumplings-1",
    .create = []() {
      return Mesh::Model("./game/assets/shops/dumplings-1.obj");
    },
    .attributes = {
      .roughness = 0.6f
    }
  },
  {
    .name = "p_fish-1",
    .create = []() {
      return Mesh::Model("./game/assets/shops/fish-1.obj");
    },
    .attributes = {
      .roughness = 0.4f
    }
  },
  {
    .name = "p_meat-1",
    .create = []() {
      return Mesh::Model("./game/assets/shops/meat-1.obj");
    },
    .attributes = {
      .roughness = 0.5f
    }
  }
};