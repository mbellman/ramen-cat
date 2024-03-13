#include "Gamma.h"

#include "mesh_library/plants.h"

using namespace Gamma;

std::vector<MeshAsset> plant_meshes = {
  {
    .name = "small-leaves",
    .create = []() {
      return Mesh::Model("./game/assets/plants/small-leaves.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/plants/small-leaves.png",
      .maxCascade = 2,
      .animation = {
        .type = PresetAnimationType::FLOWER,
        .factor = 4.f
      }
    }
  },
  {
    .name = "small-flower",
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
        .type = PresetAnimationType::FLOWER,
        .factor = 3.f
      },
      .emissivity = 0.1f,
      .roughness = 1.f
    }
  },
  {
    .name = "bush",
    .hitboxScale = Vec3f(0.8f),
    .create = []() {
      return Mesh::Model({
        "./game/assets/bush.obj",
        "./game/assets/bush-lod.obj"
      });
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/bush.png",
      .maxCascade = 4,
      .animation = {
        .type = PresetAnimationType::FLOWER,
        .factor = 30.f
      },
      .roughness = 0.9f,
      .useMipmaps = false
    }
  },
  {
    .name = "shrub",
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
    .name = "sakura",
    .create = []() {
      return Mesh::Model("./game/assets/bush.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/sakura.png",
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
    .name = "flower",
    .hitboxScale = Vec3f(1.f, 0.5f, 1.f),
    .hitboxOffset = Vec3f(0, 0.25f, 0),
    .create = []() {
      return Mesh::Model("./game/assets/flower.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/flower.png",
      .animation = {
        .type = PresetAnimationType::FLOWER,
        .factor = 2.f
      }
    }
  },
  {
    .name = "tree-trunk",
    .hitboxScale = Vec3f(0.7f, 1.f, 0.7f),
    .create = []() {
      return Mesh::Model("./game/assets/tree-trunk.obj");
    },
    .attributes = {
      .roughness = 0.9f
    }
  },
  {
    .name = "tall-trunk",
    .hitboxScale = Vec3f(0.3f, 1.f, 0.3f),
    .create = []() {
      return Mesh::Model("./game/assets/plants/tall-trunk.obj");
    },
    .attributes = {
      .maxCascade = 4,
      .roughness = 0.9f
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
  {
    .name = "palm-leaves",
    .hitboxScale = Vec3f(1.f, 0.1f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/plants/palm-leaves.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/palm-leaf.png",
      .animation = {
        .type = PresetAnimationType::LEAF,
        .factor = 4.f
      },
      .roughness = 0.8f
    }
  },
  {
    .name = "japanese-tree",
    .create = []() {
      return Mesh::Model({
        "./game/assets/japanese-tree-trunk.obj",
        "./game/assets/japanese-tree-trunk-lod.obj"
      });
    },
    .attributes = {
      .maxCascade = 4
    },
    .pieces = {
      {
        .name = "japanese-tree-leaves",
        .create = []() {
          return Mesh::Model({
            "./game/assets/japanese-tree-leaves.obj",
            "./game/assets/japanese-tree-leaves-lod.obj"
          });
        },
        .rebuild = [](auto& source, auto& piece) {
          float g = Gm_Modf(piece.position.x, 10.f) / 10.f;
          float b = Gm_Modf(piece.position.z, 10.f) / 10.f;

          piece.color = Vec3f(0.1f, 0.4f * (0.5f + g * 0.5f), 0.2f * (0.5f + b * 0.5f));
        },
        .attributes = {
          .roughness = 0.9f
        }
      }
    }
  },
  {
    .name = "branch-1",
    .defaultColor = Vec3f(0.7f, 0.5f, 0.3f),
    .create = []() {
      return Mesh::Model({
        "./game/assets/branch-1.obj",
        "./game/assets/branch-1-lod.obj"
      });
    },
    .attributes = {
      .maxCascade = 4,
      .roughness = 0.9f
    }
  },
  {
    .name = "leaves",
    .hitboxScale = Vec3f(0.5f, 1.f, 0.3f),
    .create = []() {
      return Mesh::Model("./game/assets/plants/leaves.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/plants/leaves.png",
      .maxCascade = 4,
      .animation = {
        .type = PresetAnimationType::FLOWER,
        .factor = 15.f
      },
      .useMipmaps = false
    }
  },
  {
    .name = "flat-leaves",
    .hitboxScale = Vec3f(1.f, 1.f, 0.1f),
    .create = []() {
      return Mesh::Model("./game/assets/plants/flat-leaves.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/plants/leaves.png",
      .maxCascade = 4,
      .animation = {
        .type = PresetAnimationType::FLOWER,
        .factor = 15.f
      },
      .useMipmaps = false
    }
  },
  {
    .name = "hanging-leaves",
    .hitboxScale = Vec3f(0.9f, 0.2f, 0.9f),
    .create = []() {
      return Mesh::Model("./game/assets/plants/hanging-leaves.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/plants/leaves.png",
      .maxCascade = 4,
      .animation = {
        .type = PresetAnimationType::FLOWER,
        .factor = 10.f
      },
      .useMipmaps = false
    }
  },
  {
    .name = "hanging-sakura",
    .hitboxScale = Vec3f(0.9f, 0.2f, 0.9f),
    .create = []() {
      return Mesh::Model("./game/assets/plants/hanging-sakura.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/plants/hanging-sakura.png",
      .animation = {
        .type = PresetAnimationType::FLOWER,
        .factor = 16.f
      },
      .emissivity = 0.3f,
      .useMipmaps = false
    }
  },
  {
    .name = "bamboo",
    .hitboxScale = Vec3f(0.2f, 1.f, 0.2f),
    .create = []() {
      return Mesh::Model({
        "./game/assets/bamboo.obj",
        "./game/assets/bamboo-lod.obj"
      });
    },
    .attributes = {
      .maxCascade = 4
    }
  },
  {
    .name = "banana-plant",
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
      .maxCascade = 4,
      .animation = {
        .type = PresetAnimationType::LEAF,
        .factor = 5.f
      },
      .roughness = 0.4f
    }
  },
  {
    .name = "croton",
    .hitboxScale = Vec3f(1.f, 0.3f, 1.f),
    .create = []() {
      return Mesh::Model(
        "./game/assets/plants/croton.obj"
      );
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/plants/croton.png",
      .maxCascade = 4,
      .animation = {
        .type = PresetAnimationType::LEAF,
        .factor = 3.f
      },
      .roughness = 0.3f
    }
  },
  {
    .name = "hanging-ivy",
    .hitboxScale = Vec3f(0.3f, 1.f, 0.3f),
    .create = []() {
      return Mesh::Model("./game/assets/plants/hanging-ivy.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/plants/hanging-ivy.png",
      .maxCascade = 4,
      .animation = {
        .type = PresetAnimationType::FLOWER,
        .factor = 8.f
      },
      .roughness = 0.4f
    }
  },
};