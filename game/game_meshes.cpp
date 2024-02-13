#include "game_meshes.h"

#include "mesh_library/overworld_city.h"
#include "mesh_library/overworld_stack.h"
#include "mesh_library/overworld_crossing.h"
#include "mesh_library/overworld_station.h"
#include "mesh_library/overworld_stairs.h"
#include "mesh_library/entity_meshes.h"
#include "mesh_library/decorations.h"
#include "mesh_library/vehicles.h"
#include "mesh_library/uniques.h"
#include "mesh_library/procedural.h"

using namespace Gamma;

std::vector<MeshAsset> GameMeshes::meshAssets = {
  /**
   * Primitives
   * ----------
   */
  {
    .name = "sphere",
    .hitboxScale = Vec3f(0.8f),
    .create = []() {
      return Mesh::Sphere(12);
    },
    .attributes = {
      .maxCascade = 4
    }
  },
  {
    .name = "cube",
    .maxInstances = 10000,
    .create = []() {
      return Mesh::Cube();
    },
    .attributes = {
      .maxCascade = 4
    }
  },
  {
    .name = "cylinder",
    .create = []() {
      // @todo Mesh::Cylinder()
      return Mesh::Model("./game/assets/cylinder.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },

  /**
   * Environmental elements
   * ----------------------
   */
  {
    .name = "waterfall-1",
    .create = []() {
      return Mesh::Model("./game/assets/decorations/waterfall.obj");
    },
    .attributes = {
      // @todo create a new flowing water shader (or use scrolling textures for this one)
      .type = MeshType::REFRACTIVE,
      .maxCascade = 4
    }
  },

  /**
   * Spawns
   * ------
   */
  {
    .name = "petal-spawn",
    .dynamic = true,
    .defaultColor = Vec3f(1.f, 0.5f, 0.5f),
    .create = []() {
      return Mesh::Sphere(8);
    }
  },

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
  {
    .name = "streetlamp",
    .hitboxScale = Vec3f(0.4f, 1.f, 0.15f),
    .create = []() {
      return Mesh::Model("./game/assets/streetlamp.obj");
    },
    .attributes = {
      .roughness = 0.3f
    }
  },

  /**
   * Plants/environmental details
   * ----------------------------
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
    .name = "japanese-tree",
    .create = []() {
      return Mesh::Model("./game/assets/japanese-tree-trunk.obj");
    },
    .attributes = {
      .maxCascade = 4
    },
    .pieces = {
      {
        .name = "japanese-tree-leaves",
        .create = []() {
          return Mesh::Model("./game/assets/japanese-tree-leaves.obj");
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
      return Mesh::Model("./game/assets/leaves.obj");
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
    .name = "bamboo",
    .hitboxScale = Vec3f(0.2f, 1.f, 0.2f),
    .create = []() {
      return Mesh::Model("./game/assets/bamboo.obj");
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
        .factor = 10.f
      },
      .roughness = 0.4f
    }
  },
  {
    .name = "rock-1",
    .create = []() {
      return Mesh::Model("./game/assets/rock-1.obj");
    },
    .attributes = {
      .maxCascade = 4,
      .roughness = 0.8f
    }
  },
  {
    .name = "garden-terrain",
    .create = []() {
      return Mesh::Model("./game/assets/garden-terrain.obj");
    },
    .attributes = {
      .maxCascade = 4,
      .roughness = 1.f
    }
  },

  /**
   * Animals
   * -------
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
    .hitboxScale = Vec3f(0.2f, 0.25f, 0.5f),
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
    .hitboxScale = Vec3f(0.2f, 0.25f, 0.5f),
    .create = []() {
      return Mesh::Model("./game/assets/bird-flying.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/bird.png",
      .animation = {
        .type = PresetAnimationType::BIRD,
        .speed = 3.f,
        .factor = 0.75f
      },
      .emissivity = 0.3f
    }
  },
  {
    .name = "firefly-spawn",
    .dynamic = true,
    .defaultColor = Vec3f(0.7f, 1.f, 0.2f),
    .create = []() {
      return Mesh::Cube();
    },
    .attributes = {
      .emissivity = 1.f
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
      .maxCascade = 4,
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
    .name = "tiles",
    .hitboxScale = Vec3f(1.f, 0.025f, 1.f),
    .create = []() {
      return Mesh::Plane(2);
    },
    .attributes = {
      .texture = "./game/assets/tiles.png",
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
      .maxCascade = 4,
      .roughness = 0.9f
    }
  },
  {
    .name = "japanese-building",
    .create = []() {
      return Mesh::Model("./game/assets/japanese-building.obj");
    },
    .attributes = {
      .texture = "./game/assets/japanese-building.png",
      .roughness = 0.7f
    }
  },
  {
    .name = "yuki-building-1",
    .defaultColor = Vec3f(1.f, 0.9f, 0.8f),
    .hitboxScale = Vec3f(0.5f, 1.f, 0.5f),
    .create = []() {
      return Mesh::Model("./game/assets/yuki-building-1.obj");
    }
  },
  {
    .name = "yuki-building-2",
    .defaultColor = Vec3f(1.f, 0.9f, 0.8f),
    .hitboxScale = Vec3f(0.5f, 1.f, 0.5f),
    .create = []() {
      return Mesh::Model("./game/assets/yuki-building-2.obj");
    }
  },
  {
    .name = "yuki-building-3",
    .defaultColor = Vec3f(1.f, 0.9f, 0.8f),
    .hitboxScale = Vec3f(0.5f, 1.f, 0.5f),
    .create = []() {
      return Mesh::Model("./game/assets/yuki-building-3.obj");
    }
  },
  {
    .name = "torii-gate",
    .hitboxScale = Vec3f(1.f, 1.f, 0.15f),
    .maxInstances = 50,
    .create = []() {
      return Mesh::Model("./game/assets/torii-gate.obj");
    },
    .attributes = {
      .texture = "./game/assets/torii-gate.png",
      .maxCascade = 4,
      .roughness = 0.4f
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
      .maxCascade = 4,
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
      .maxCascade = 4,
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
      .maxCascade = 4,
      .roughness = 0.1f
    }
  },
  {
    .name = "dome-roof",
    .hitboxScale = Vec3f(1.f, 0.5f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/dome-roof.obj");
    },
    .attributes = {
      .maxCascade = 4,
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
      .maxCascade = 4,
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
    .name = "rock-wall",
    .hitboxScale = Vec3f(1.f, 1.f, 0.2f),
    .create = []() {
      return Mesh::Model("./game/assets/rock-wall.obj");
    },
    .attributes = {
      .texture = "./game/assets/rock-wall.png",
      .normals = "./game/assets/rock-wall-normals.png"
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
      .maxCascade = 4,
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
      .maxCascade = 4,
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
      .maxCascade = 4,
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
    .name = "wind-turbine-base",
    .hitboxScale = Vec3f(0.1f, 1.f, 0.1f),
    .create = []() {
      return Mesh::Model("./game/assets/wind-turbine-base.obj");
    },
    .attributes = {
      .roughness = 0.4f
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
    .name = "door",
    .hitboxScale = Vec3f(0.5f, 1.f, 0.1f),
    .create = []() {
      return Mesh::Model("./game/assets/door.obj");
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
    .name = "awning",
    .hitboxScale = Vec3f(0.9f, 0.2f, 0.4f),
    .create = []() {
      return Mesh::Model("./game/assets/awning.obj");
    },
    .attributes = {
      .texture = "./game/assets/awning.png",
      .maxCascade = 4
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
      return Mesh::Model({
        "./game/assets/hot-air-balloon.obj",
        "./game/assets/hot-air-balloon-lod.obj"
      });
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
   * Entities
   * --------
   */
  {
    .name = "boost-ring",
    .hitboxScale = Vec3f(1.f, 1.f, 0.1f),
    .create = []() {
      return Mesh::Model("./game/assets/ring.obj");
    },
    .attributes = {
      .roughness = 0.2f
    }
  },

  /**
   * Collectables
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
      .emissivity = 0.2f,
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
      .emissivity = 0.2f,
      .useMipmaps = false
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
  },
  {
    .name = "umimura-sculpture-stand",
    .hitboxScale = Vec3f(0.1f, 1.f, 0.1f),
    .maxInstances = 2,
    .create = []() {
      return Mesh::Model("./game/assets/sculpture-stand.obj");
    }
  },
  {
    .name = "umimura-sculpture-fan",
    .moving = true,
    .hitboxScale = Vec3f(1.f, 0.2f, 1.f),
    .maxInstances = 2,
    .create = []() {
      return Mesh::Model("./game/assets/sculpture-fan.obj");
    },
    .attributes = {
      .texture = "./game/assets/sculpture-fan.png"
    }
  }
};

// @todo remove + define these meshes as pieces for parent meshes
std::vector<MeshAsset> GameMeshes::dynamicMeshPieces = {
  {
    .name = "stair-step",
    .maxInstances = 10000,
    .create = []() {
      return Mesh::Model("./game/assets/stair-step.obj");
    },
    .attributes = {
      .texture = "./game/assets/stairs-1.png",
      .normals = "./game/assets/stairs-1-normals.png",
      .roughness = 0.3f
    }
  },

  {
    .name = "streetlamp-light",
    .create = []() {
      return Mesh::Cube();
    },
    .attributes = {
      .emissivity = 1.f
    }
  },
  {
    .name = "streetlamp-frame",
    .create = []() {
      return Mesh::Model("./game/assets/streetlamp-frame.obj");
    },
    .attributes = {
      .texture = "./game/assets/wood-beam.png"
    }
  },

  {
    .name = "wire",
    .maxInstances = 10000,
    .create = []() {
      return Mesh::Cube();
    },
    .attributes = {
      .roughness = 0.3f
    }
  },

  {
    .name = "ac-fan",
    .moving = true,
    .create = []() {
      return Mesh::Model("./game/assets/ac-fan.obj");
    },
    .attributes = {
      .roughness = 0.2f
    }
  },
  {
    .name = "wind-turbine",
    .moving = true,
    .create = []() {
      return Mesh::Model("./game/assets/wind-turbine.obj");
    },
    .attributes = {
      .roughness = 0.4f
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
  },

  {
    .name = "yuki-building-1-frame",
    .create = []() {
      return Mesh::Model("./game/assets/yuki-building-1-frame.obj");
    },
    .attributes = {
      .roughness = 0.8f
    }
  },

  {
    .name = "yuki-building-2-frame",
    .create = []() {
      return Mesh::Model("./game/assets/yuki-building-2-frame.obj");
    },
    .attributes = {
      .roughness = 0.8f
    }
  },

  {
    .name = "yuki-building-3-frame",
    .create = []() {
      return Mesh::Model("./game/assets/yuki-building-3-frame.obj");
    },
    .attributes = {
      .roughness = 0.8f
    }
  },

  // wood-facade
  {
    .name = "wood-facade-base",
    .create = []() {
      return Mesh::Model("./game/assets/buildings/wood-facade-base.obj");
    }
  },
  {
    .name = "wood-facade-cover",
    .create = []() {
      return Mesh::Model("./game/assets/buildings/wood-facade-cover.obj");
    }
  },

  // bridge-1
  {
    .name = "bridge-1-floor",
    .create = []() {
      return Mesh::Model("./game/assets/buildings/bridge-1-floor.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },
  {
    .name = "bridge-1-supports",
    .create = []() {
      return Mesh::Model("./game/assets/buildings/bridge-1-supports.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },
  {
    .name = "bridge-1-roof",
    .create = []() {
      return Mesh::Model("./game/assets/buildings/bridge-1-roof.obj");
    },
    .attributes = {
      .maxCascade = 4,
      .roughness = 0.3f
    }
  },

  // wave-sign
  {
    .name = "dynamic-wave-sign",
    .create = []() {
      return Mesh::Model("./game/assets/buildings/wave-sign.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },

  // sign-roof
  {
    .name = "sign-roof-supports",
    .create = []() {
      return Mesh::Model("./game/assets/buildings/sign-roof-supports.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },
  {
    .name = "sign-roof-signs",
    .create = []() {
      return Mesh::Model("./game/assets/buildings/sign-roof-signs.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },

  // particles
  {
    .name = "petal",
    .moving = true,
    .maxInstances = 0xffff,
    .create = []() {
      return Mesh::Model("./game/assets/petal.obj");
    },
    .attributes = {
      .maxCascade = 2,
      .emissivity = 0.8f
    }
  },

  // flag-wire-spawn
  {
    .name = "mini-flag",
    .create = []() {
      return Mesh::Model({
        "./game/assets/decorations/mini-flag.obj",
        "./game/assets/decorations/mini-flag-lod.obj"
      });
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .maxCascade = 4,
      .animation = {
        .type = PresetAnimationType::CLOTH,
        .factor = 3.f
      },
    }
  },
};

std::vector<MeshAsset> GameMeshes::proceduralMeshParts = {};

void GameMeshes::loadAllMeshAssets() {
  #define add_mesh_assets(list, elements) list.insert(list.end(), elements.begin(), elements.end())

  // Store procedural mesh parts
  GameMeshes::proceduralMeshParts = procedural_mesh_parts;

  // Add assets from mesh library files
  auto& assets = GameMeshes::meshAssets;

  add_mesh_assets(assets, overworld_city_meshes);
  add_mesh_assets(assets, overworld_stack_meshes);
  add_mesh_assets(assets, overworld_crossing_meshes);
  add_mesh_assets(assets, overworld_station_meshes);
  add_mesh_assets(assets, overworld_stairs_meshes);
  add_mesh_assets(assets, entity_meshes);
  add_mesh_assets(assets, decoration_meshes);
  add_mesh_assets(assets, vehicle_meshes);
  add_mesh_assets(assets, unique_meshes);
  add_mesh_assets(assets, procedural_meshes);
}