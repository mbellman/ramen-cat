#include "game_meshes.h"

#include "mesh_library/overworld_town.h"
#include "mesh_library/overworld_city.h"
#include "mesh_library/overworld_stack.h"
#include "mesh_library/overworld_crossing.h"
#include "mesh_library/overworld_station.h"
#include "mesh_library/overworld_stairs.h"
#include "mesh_library/entities.h"
#include "mesh_library/plants.h"
#include "mesh_library/shops.h"
#include "mesh_library/characters.h"
#include "mesh_library/lights.h"
#include "mesh_library/decorations.h"
#include "mesh_library/spinners.h"
#include "mesh_library/transportation.h"
#include "mesh_library/uniques.h"
#include "mesh_library/procedural.h"
#include "mesh_library/teikoku.h"

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
      .maxCascade = 4,
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
      .type = MeshType::REFRACTIVE,
      .normals = "./game/assets/water-normals.png",
      .maxCascade = 4,
      .useMipmaps = false
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
   * Environmental details
   * ----------------------------
   */
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
      return Mesh::Model({
        "./game/assets/garden-terrain.obj",
        "./game/assets/garden-terrain-lod.obj"
      });
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
      .maxCascade = 4,
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
    .name = "tile-walkway",
    .create = []() {
      return Mesh::Cube();
    },
    .attributes = {
      .texture = "./game/assets/tile-walkway.png",
      .normals = "./game/assets/cobblestone-normals.png",
      .roughness = 0.7f,
      .useXzPlaneTexturing = true
    }
  },
  {
    .name = "metal-floor",
    .hitboxScale = Vec3f(1.f, 0.025f, 1.f),
    .create = []() {
      return Mesh::Plane(2);
    },
    .attributes = {
      .texture = "./game/assets/metal-floor.png",
      .normals = "./game/assets/metal-floor-normals.png",
      .roughness = 0.1f,
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
      return Mesh::Model({
        "./game/assets/japanese-roof.obj",
        "./game/assets/japanese-roof-lod.obj"
      });
    },
    .attributes = {
      .maxCascade = 4,
      .roughness = 0.6f
    }
  },
  {
    .name = "bathhouse-roof-segment",
    .defaultColor = Vec3f(0.43f,0.08f,0.08f),
    .defaultScale = Vec3f(100.f),
    .hitboxScale = Vec3f(1.f, 0.3f, 0.8f),
    .create = []() {
      return Mesh::Model({
        "./game/assets/bathhouse-roof-segment.obj",
        "./game/assets/bathhouse-roof-segment-lod.obj"
      });
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
      return Mesh::Model({
        "./game/assets/bathhouse-roof-corner.obj",
        "./game/assets/bathhouse-roof-corner-lod.obj",
      });
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
      .normals = "./game/assets/tiles-1-normals.png",
      .maxCascade = 4
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
      .maxCascade = 4,
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
    .name = "electrical-pole",
    .defaultScale = Vec3f(275.f),
    .hitboxScale = Vec3f(0.05f, 1.f, 0.15f),
    .create = []() {
      return Mesh::Model("./game/assets/electrical-pole.obj");
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
   * Stairs
   * ------
   */
  {
    .name = "metal-staircase",
    .dynamic = true,
    .defaultColor = Vec3f(0, 1.f, 0),
    .create = []() {
      return Mesh::Cube();
    }
  },
  {
    .name = "wood-staircase",
    .dynamic = true,
    .defaultColor = Vec3f(1.f, 0.8f, 0.6f),
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
};

// @todo remove + define these meshes as pieces for parent meshes (or move to procedural)
std::vector<MeshAsset> GameMeshes::dynamicMeshPieces = {
  {
    .name = "metal-stair-step",
    .maxInstances = 10000,
    .create = []() {
      return Mesh::Model("./game/assets/metal-stair-step.obj");
    },
    .attributes = {
      .texture = "./game/assets/stairs-1.png",
      .normals = "./game/assets/stairs-1-normals.png",
      .roughness = 0.1f
    }
  },
  {
    .name = "wood-stair-step",
    .maxInstances = 10000,
    .create = []() {
      return Mesh::Model("./game/assets/wood-stair-step.obj");
    },
    .attributes = {
      .texture = "./game/assets/wood-beam.png",
      .normals = "./game/assets/wood-beam-normals.png",
      .roughness = 0.7f
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
      .maxCascade = 4,
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
    .maxInstances = 10000,
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
        .factor = 1.5f
      },
    }
  },
};

std::vector<MeshAsset> GameMeshes::proceduralMeshParts = {};

void GameMeshes::loadAllMeshAssets() {
  // Store procedural mesh parts
  GameMeshes::proceduralMeshParts = procedural_mesh_parts;

  // Add assets from mesh library files
  auto& assets = GameMeshes::meshAssets;

  #define add_mesh_assets(list, elements) list.insert(list.end(), elements.begin(), elements.end())

  add_mesh_assets(assets, overworld_town_meshes);
  add_mesh_assets(assets, overworld_city_meshes);
  add_mesh_assets(assets, overworld_stack_meshes);
  add_mesh_assets(assets, overworld_crossing_meshes);
  add_mesh_assets(assets, overworld_station_meshes);
  add_mesh_assets(assets, overworld_stairs_meshes);
  add_mesh_assets(assets, entity_meshes);
  add_mesh_assets(assets, plant_meshes);
  add_mesh_assets(assets, shop_meshes);
  add_mesh_assets(assets, character_meshes);
  add_mesh_assets(assets, light_meshes);
  add_mesh_assets(assets, decoration_meshes);
  add_mesh_assets(assets, spinner_meshes);
  add_mesh_assets(assets, transportation_meshes);
  add_mesh_assets(assets, unique_meshes);
  add_mesh_assets(assets, procedural_meshes);
  add_mesh_assets(assets, teikoku_meshes);
}