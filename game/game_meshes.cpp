#include "game_meshes.h"

using namespace Gamma;

std::vector<MeshAsset> GameMeshes::meshAssets = {
  /**
   * Lights
   * ------
   */
  {
    .name = "lamp",
    .create = []() {
      return Mesh::Model("./game/assets/lamp.obj");
    },
    .attributes = {
      .texture = "./game/assets/lamp.png",
      .maxCascade = 2,
      .emissivity = 1.f
    }
  },
  {
    .name = "lantern",
    .moving = true,
    .create = []() {
      return Mesh::Model("./game/assets/lantern.obj");
    },
    .attributes = {
      .texture = "./game/assets/lantern.png",
      .emissivity = 0.5f,
      .useMipmaps = false
    }
  },
  {
    .name = "paper-lantern",
    .moving = true,
    .create = []() {
      return Mesh::Model("./game/assets/paper-lantern.obj");
    },
    .attributes = {
      .texture = "./game/assets/paper-lantern.png",
      .emissivity = 0.5f,
      .useMipmaps = false
    }
  },
  {
    .name = "small-light",
    .defaultColor = Vec3f(1.f, 0.9f, 0.7f),
    .hitboxScale = Vec3f(1.5f),
    .create = []() {
      return Mesh::Model("./game/assets/small-light.obj");
    },
    .attributes = {
      .maxCascade = 2,
      .emissivity = 1.f
    }
  },
  {
    .name = "japanese-lamppost",
    .hitboxScale = Vec3f(0.4f, 1.f, 0.25f),
    .create = []() {
      return Mesh::Model("./game/assets/japanese-lamppost.obj");
    },
    .attributes = {
      .texture = "./game/assets/japanese-lamppost.png"
    }
  },

  /**
   * Plants
   * ------
   */
  {
    .name = "weeds",
    .hitboxScale = Vec3f(1.f, 1.f, 0.2f),
    .create = []() {
      return Mesh::Model("./game/assets/weeds.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/weeds.png",
      .maxCascade = 2,
      .animation = {
        .type = PresetAnimationType::FLOWER
      },
      .emissivity = 0.1f,
      .roughness = 1.f
    }
  },
  {
    .name = "bush",
    .create = []() {
      return Mesh::Model("./game/assets/bush.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/bush.png",
      .animation = {
        .type = PresetAnimationType::FLOWER,
        .factor = 5.f
      },
      .useMipmaps = false
    }
  },
  {
    .name = "vines-horizontal",
    .hitboxScale = Vec3f(1.f, 0.3f, 0.3f),
    .create = []() {
      return Mesh::Model("./game/assets/vines-horizontal.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/vines-horizontal.png",
      .animation = {
        .type = PresetAnimationType::FLOWER,
        .factor = 5.f
      },
      .useMipmaps = false
    }
  },
  {
    .name = "vines-vertical",
    .hitboxScale = Vec3f(0.3f, 0.3f, 0.3f),
    .create = []() {
      return Mesh::Model("./game/assets/vines-vertical.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/vines-vertical.png",
      .animation = {
        .type = PresetAnimationType::FLOWER,
        .factor = 5.f
      },
      .useMipmaps = false
    }
  },
  {
    .name = "palm-trunk",
    .hitboxScale = Vec3f(0.1f, 1.f, 0.1f),
    .create = []() {
      return Mesh::Model("./game/assets/palm-trunk.obj");
    },
    .attributes = {
      .texture = "./game/assets/palm-trunk.png",
      .roughness = 0.8f
    }
  },
  {
    .name = "palm-leaf",
    .defaultRotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), Gm_HALF_PI),
    .hitboxScale = Vec3f(0.2f, 0.5f, 0.2f),
    .hitboxOffset = Vec3f(0, 1.f, 0.9),
    .create = []() {
      return Mesh::Model("./game/assets/palm-leaf.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/palm-leaf.png",
      .animation = {
        .type = PresetAnimationType::LEAF
      },
      .roughness = 0.8f
    }
  },

  /**
   * Animals
   */
  {
    .name = "seagull",
    .moving = true,
    .hitboxScale = Vec3f(1.4f, 0.2f, 1.1f),
    .create = []() {
      return Mesh::Model("./game/assets/seagull.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/seagull.png",
      .animation = {
        .type = PresetAnimationType::BIRD
      },
      .emissivity = 0.3f
    }
  },
  {
    .name = "bird-at-rest",
    .moving = true,
    .hitboxScale = Vec3f(0.4f, 0.5f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/bird-at-rest.obj");
    },
    .attributes = {
      .texture = "./game/assets/bird.png",
      .emissivity = 0.3f
    }
  },
  {
    .name = "bird-flying",
    .hitboxScale = Vec3f(0.4f, 0.5f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/bird-flying.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/bird.png",
      .animation = {
        .type = PresetAnimationType::BIRD,
        .speed = 5.f
      },
      .emissivity = 0.3f
    }
  },

  /**
   * Structures
   * ----------
   */
  {
    .name = "concrete-slab",
    .maxInstances = 10000,
    .create = []() {
      return Mesh::Cube();
    },
    .attributes = {
      .roughness = 0.9f
    }
  },
  {
    .name = "cobblestone",
    .hitboxScale = Vec3f(1.f, 0.025f, 1.f),
    .create = []() {
      return Mesh::Plane(2);
    },
    .attributes = {
      .texture = "./game/assets/cobblestone.png",
      .normals = "./game/assets/cobblestone-normals.png",
      .roughness = 0.7f,
      .useXzPlaneTexturing = true
    }
  },
  {
    .name = "building-1",
    .dynamic = true,
    .create = []() {
      return Mesh::Model("./game/assets/building-1.obj");
    },
    .attributes = {
      .roughness = 0.9f
    }
  },
  {
    .name = "building-arch",
    .hitboxScale = Vec3f(1.f, 1.f, 0.1f),
    .create = []() {
      return Mesh::Model("./game/assets/building-arch.obj");
    },
    .attributes = {
      .roughness = 0.3f
    }
  },
  {
    .name = "wood-structure",
    .create = []() {
      return Mesh::Model("./game/assets/wood-structure.obj");
    },
    .attributes = {
      .texture = "./game/assets/wood-beam.png",
      .roughness = 0.5f,
    }
  },
  {
    .name = "corrugated-roof",
    .create = []() {
      return Mesh::Model("./game/assets/corrugated-roof.obj");
    },
    .attributes = {
      .texture = "./game/assets/corrugated-roof.png",
      .normals = "./game/assets/corrugated-roof-normals.png",
      .roughness = 0.2f,
    }
  },
  {
    .name = "bridge-arch",
    .hitboxScale = Vec3f(1.f, 1.f, 0.2f),
    .create = []() {
      return Mesh::Model("./game/assets/bridge-arch.obj");
    },
    .attributes = {
      .roughness = 0.8f,
    }
  },

  {
    .name = "concrete-half-arch",
    .defaultColor = Vec3f(0.8f),
    .create = []() {
      return Mesh::Model("./game/assets/concrete-half-arch.obj");
    },
    .attributes = {
      .roughness = 0.9f
    }
  },
  {
    .name = "japanese-roof",
    .defaultColor = Vec3f(0.2f,0.5f,0.3f),
    .defaultScale = Vec3f(100.f),
    .hitboxScale = Vec3f(1.f, 0.05f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/japanese-roof.obj");
    },
    .attributes = {
      .roughness = 0.6f
    }
  },
  {
    .name = "bathhouse-roof-segment",
    .defaultColor = Vec3f(0.43f,0.08f,0.08f),
    .defaultScale = Vec3f(100.f),
    .hitboxScale = Vec3f(1.f, 0.3f, 0.8f),
    .create = []() {
      return Mesh::Model("./game/assets/bathhouse-roof-segment.obj");
    },
    .attributes = {
      .normals = "./game/assets/metal-guard-normals.png",
      .roughness = 0.8f
    }
  },
  {
    .name = "bathhouse-roof-corner",
    .defaultColor = Vec3f(0.43f,0.08f,0.08f),
    .defaultScale = Vec3f(100.f),
    .hitboxScale = Vec3f(0.7f, 0.25f, 0.7f),
    .create = []() {
      return Mesh::Model("./game/assets/bathhouse-roof-corner.obj");
    },
    .attributes = {
      .normals = "./game/assets/metal-guard-normals.png",
      .roughness = 0.8f
    }
  },
  {
    .name = "bathhouse-roof-spire",
    .defaultColor = Vec3f(0.8f,0.6f,0.2f),
    .defaultScale = Vec3f(100.f),
    .hitboxScale = Vec3f(1.f, 2.f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/bathhouse-roof-spire.obj");
    },
    .attributes = {
      .roughness = 0.3f
    }
  },
  {
    .name = "dome-roof",
    .hitboxScale = Vec3f(1.f, 0.5f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/dome-roof.obj");
    },
    .attributes = {
      .roughness = 0.1f
    }
  },
  {
    .name = "square-roof",
    .hitboxScale = Vec3f(1.f, 0.15f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/square-roof.obj");
    },
    .attributes = {
      .roughness = 0.2f
    }
  },
  {
    .name = "metal-panel",
    .create = []() {
      return Mesh::Cube();
    },
    .attributes = {
      .texture = "./game/assets/metal-panel.png",
      .normals = "./game/assets/metal-panel-normals.png",
      .roughness = 0.4f
    }
  },
  {
    .name = "metal-grate",
    .defaultScale = Vec3f(100.f),
    .hitboxScale = Vec3f(1.f, 0.0f, 1.f),
    .create = []() {
      return Mesh::Plane(2);
    },
    .attributes = {
      .texture = "./game/assets/metal-grate.png",
      .normals = "./game/assets/metal-grate-normals.png",
      .roughness = 0.1f
    }
  },
  {
    .name = "metal-guard",
    .defaultColor = Vec3f(0.333f),
    .hitboxScale = Vec3f(1.f, 0.2f, 0.05f),
    .create = []() {
      return Mesh::Model("./game/assets/metal-guard.obj");
    },
    .attributes = {
      .texture = "./game/assets/metal-guard.png",
      .normals = "./game/assets/metal-guard-normals.png",
      .roughness = 0.1f
    }
  },
  {
    .name = "ladder",
    .hitboxScale = Vec3f(0.45f, 1.f, 0.2f),
    .create = []() {
      return Mesh::Model("./game/assets/ladder.obj");
    },
    .attributes = {
      .texture = "./game/assets/metal-guard.png",
      .normals = "./game/assets/metal-guard-normals.png",
      .roughness = 0.1f
    }
  },
  {
    .name = "wood-beam",
    .hitboxScale = Vec3f(1.f, 0.025f, 0.1f),
    .create = []() {
      return Mesh::Model("./game/assets/wood-beam.obj");
    },
    .attributes = {
      .texture = "./game/assets/wood-beam.png",
      .roughness = 0.5f
    }
  },
  {
    .name = "wood-supports",
    .hitboxScale = Vec3f(0.5f, 1.7f, 0.15f),
    .create = []() {
      return Mesh::Model("./game/assets/wood-supports.obj");
    },
    .attributes = {
      .texture = "./game/assets/wood-beam.png",
      .roughness = 0.5f
    }
  },
  {
    .name = "windmill-base",
    .hitboxScale = Vec3f(0.5f, 1.f, 0.5f),
    .create = []() {
      return Mesh::Model("./game/assets/windmill-base.obj");
    },
    .attributes = {
      .texture = "./game/assets/wood-beam.png",
      .roughness = 0.7f
    }
  },
  {
    .name = "windmill-wheel",
    .moving = true,
    .defaultScale = Vec3f(75.f),
    .hitboxScale = Vec3f(1.f, 1.f, 0.1f),
    .create = []() {
      return Mesh::Model("./game/assets/windmill-wheel.obj");
    },
    .attributes = {
      .texture = "./game/assets/wood-beam.png"
    }
  },
  {
    .name = "floor-1",
    .create = []() {
      return Mesh::Model("./game/assets/texture-cube.obj");
    },
    .attributes = {
      .texture = "./game/assets/floor-1.png",
      .normals = "./game/assets/floor-1-normals.png"
    }
  },
  {
    .name = "tiles-1",
    .create = []() {
      return Mesh::Model("./game/assets/texture-cube.obj");
    },
    .attributes = {
      .texture = "./game/assets/tiles-1.png",
      .normals = "./game/assets/tiles-1-normals.png"
    }
  },
  {
    .name = "wall-1",
    .scalingFactor = Vec3f(1.f, 0, 1.f),
    .create = []() {
      return Mesh::Plane(2);
    },
    .attributes = {
      // .texture = "./game/assets/wall-1.png",
      // .normals = "./game/assets/wall-1-normals.png",
      .roughness = 0.1f
    }
  },
  {
    .name = "metal-sheet",
    .hitboxScale = Vec3f(0.5f, 0.6f, 0.025f),
    .create = []() {
      return Mesh::Model("./game/assets/metal-sheet.obj");
    },
    .attributes = {
      .texture = "./game/assets/metal-sheet.png",
      .normals = "./game/assets/metal-sheet-normals.png",
      .roughness = 0.1f
    }
  },
  {
    .name = "ac-unit",
    .hitboxScale = Vec3f(1.f, 0.6f, 0.5f),
    .create = []() {
      return Mesh::Model("./game/assets/ac-unit.obj");
    },
    .attributes = {
      .roughness = 0.4f
    }
  },
  {
    .name = "ac-fan",
    .moving = true,
    .defaultColor = Vec3f(0.1f),
    .hitboxScale = Vec3f(1.f, 1.f, 0.1f),
    .create = []() {
      return Mesh::Model("./game/assets/ac-fan.obj");
    },
    .attributes = {
      .roughness = 0.2f
    }
  },
  {
    .name = "kick-sign",
    .hitboxScale = Vec3f(1.f, 1.1f, 0.15f),
    .create = []() {
      return Mesh::Model("./game/assets/kick-sign.obj");
    },
    .attributes = {
      .texture = "./game/assets/kick-sign.png",
      .emissivity = 0.2f,
      .roughness = 0.4f
    }
  },
  {
    .name = "sign-1",
    .hitboxScale = Vec3f(0.1f, 0.5f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/sign-1.obj");
    },
    .attributes = {
      .texture = "./game/assets/sign-1.png",
      .emissivity = 0.5f
    }
  },
  {
    .name = "door-1",
    .hitboxScale = Vec3f(0.5f, 1.f, 0.1f),
    .create = []() {
      return Mesh::Model("./game/assets/door-1.obj");
    },
    .attributes = {
      .texture = "./game/assets/door-1.png",
      .emissivity = 0.25f
    }
  },
  {
    .name = "windows-1",
    .hitboxScale = Vec3f(1.f, 0.4f, 0.05f),
    .create = []() {
      return Mesh::Model("./game/assets/windows-1.obj");
    },
    .attributes = {
      .texture = "./game/assets/windows-1.png",
      .emissivity = 0.25f
    }
  },
  {
    .name = "window-1",
    .hitboxScale = Vec3f(0.8f, 1.f, 0.1f),
    .create = []() {
      return Mesh::Model("./game/assets/window-1.obj");
    },
    .attributes = {
      .texture = "./game/assets/window-1.png",
      .roughness = 0.4f
    }
  },
  {
    .name = "window-2",
    .hitboxScale = Vec3f(1.f, 1.f, 0.15f),
    .create = []() {
      return Mesh::Model("./game/assets/window-2.obj");
    },
    .attributes = {
      .texture = "./game/assets/window-2.png",
      .roughness = 0.4f
    }
  },
  {
    .name = "glass-window",
    .defaultColor = Vec3f(1.f),
    .create = []() {
      return Mesh::Cube();
    },
    .attributes = {
      .type = MeshType::REFRACTIVE,
      .normals = "./game/assets/glass-normals.png",
      .roughness = 0.9f
    } 
  },
  {
    .name = "flat-railing",
    .hitboxScale = Vec3f(1.f, 0.5f, 0.1f),
    .create = []() {
      return Mesh::Model("./game/assets/flat-railing.obj");
    },
    .attributes = {
      .texture = "./game/assets/flat-railing.png",
      .normals = "./game/assets/flat-railing-normals.png",
      .roughness = 0.3f,
      .useMipmaps = false,
      .useCloseTranslucency = true
    }
  },
  {
    .name = "wood-plank",
    .hitboxScale = Vec3f(0.2f, 0.05f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/wood-plank.obj");
    },
    .attributes = {
      .texture = "./game/assets/wood-plank.png",
      .normals = "./game/assets/wood-plank-normals.png",
      .roughness = 0.4f
    }
  },
  {
    .name = "pipe-1",
    .hitboxScale = Vec3f(0.3f, 1.f, 0.3f),
    .create = []() {
      return Mesh::Model("./game/assets/pipe-1.obj");
    },
    .attributes = {
      .texture = "./game/assets/pipe-1.png",
      .normals = "./game/assets/pipe-1-normals.png",
      .roughness = 0.1f
    }
  },
  {
    .name = "pipe-curve",
    .hitboxScale = Vec3f(0.3f, 0.5f, 0.6f),
    .create = []() {
      return Mesh::Model("./game/assets/pipe-curve.obj");
    },
    .attributes = {
      .texture = "./game/assets/pipe-1.png",
      .normals = "./game/assets/pipe-1-normals.png",
      .roughness = 0.1f
    }
  },
  {
    .name = "roof",
    .hitboxScale = Vec3f(1.f, 0.5f, 0.8f),
    .create = []() {
      return Mesh::Model("./game/assets/roof.obj");
    },
    .attributes = {
      .texture = "./game/assets/roof.png"
    }
  },
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
    .name = "ramen-lamp",
    .moving = true,
    .hitboxScale = Vec3f(0.7f, 1.2f, 0.7f),
    .create = []() {
      return Mesh::Model("./game/assets/ramen-lamp.obj");
    },
    .attributes = {
      .texture = "./game/assets/ramen-lamp.png",
      .useMipmaps = false
    }
  },

  {
    .name = "ramen-sign",
    .hitboxScale = Vec3f(1.f, 0.25f, 0.05f),
    .hitboxOffset = Vec3f(0, -1.f, 0),
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
    .name = "ramen-bowl",
    .hitboxScale = Vec3f(1.f, 0.4f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/ramen-bowl.obj");
    },
    .attributes = {
      .texture = "./game/assets/ramen-bowl.png",
      .roughness = 0.4f
    }
  },
  {
    .name = "hot-air-balloon",
    .moving = true,
    .defaultScale = Vec3f(200.f),
    .create = []() {
      return Mesh::Model("./game/assets/hot-air-balloon.obj");
    },
    .attributes = {
      .texture = "./game/assets/hot-air-balloon.png",
      .normals = "./game/assets/hot-air-balloon-normals.png",
      .emissivity = 0.2f,
      .roughness = 0.7f,
      .canCastShadows = false
    }
  },
  {
    .name = "electrical-pole",
    .defaultScale = Vec3f(275.f),
    .hitboxScale = Vec3f(0.05f, 1.f, 0.15f),
    .create = []() {
      return Mesh::Model("./game/assets/electrical-pole.obj");
    }
  },

  /**
   * Characters
   * ----------
   */
  {
    .name = "person",
    .moving = true,
    .defaultScale = Vec3f(65.f),
    .hitboxScale = Vec3f(0.6f, 1.4f, 0.2f),
    .create = []() {
      return Mesh::Model("./game/assets/person.obj");
    }
  },

  /**
   * Collectibles
   * ------------
   */
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
      .emissivity = 0.2f
    }
  },

  /**
   * Stairs
   * ------
   */
  {
    .name = "staircase",
    .dynamic = true,
    .defaultColor = Vec3f(0, 1.f, 0),
    .create = []() {
      return Mesh::Cube();
    }
  },
  {
    .name = "stairs-1",
    .create = []() {
      return Mesh::Model("./game/assets/stairs-1.obj");
    },
    .attributes = {
      .texture = "./game/assets/stairs-1.png",
      .normals = "./game/assets/stairs-1-normals.png",
      .roughness = 0.3f
    }
  },

  /**
   * Level-specific structures
   * ------------------------
   */
  {
    .name = "umimura-tree-base",
    .defaultScale = Vec3f(1000.f),
    .maxInstances = 2,
    .create = []() {
      return Mesh::Model("./game/assets/umimura-tree-base.obj");
    },
    .attributes = {
      .roughness = 0.2f
    }
  },
  {
    .name = "umimura-tree-branches",
    .defaultScale = Vec3f(1000.f),
    .maxInstances = 2,
    .create = []() {
      return Mesh::Model("./game/assets/umimura-tree-branches.obj");
    },
    .attributes = {
      .emissivity = 0.5f
    }
  }
};

std::vector<MeshAsset> GameMeshes::dynamicMeshPieces = {
  {
    .name = "stair-step",
    .maxInstances = 10000,
    .create = []() {
      return Mesh::Cube();
    }
  },
  {
    .name = "japanese-lamppost-light",
    .create = []() {
      return Mesh::Model("./game/assets/japanese-lamppost-light.obj");
    },
    .attributes = {
      .emissivity = 1.f
    }
  },
  {
    .name = "wire",
    .create = []() {
      return Mesh::Cube();
    },
    .attributes = {
      .roughness = 0.3f
    }
  },

  {
    .name = "building-1-body",
    .create = []() {
      return Mesh::Cube();
    }
  },
  {
    .name = "building-1-frame",
    .create = []() {
      return Mesh::Model("./game/assets/building-1-frame.obj");
    },
    .attributes = {
      .texture = "./game/assets/building-1-frame.png"
    }
  }
};
