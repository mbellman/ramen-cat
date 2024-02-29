#include "Gamma.h"

#include "mesh_library/decorations.h"

using namespace Gamma;

std::vector<MeshAsset> decoration_meshes = {
  // @todo move to special.cpp
  {
    .name = "ocean-plane-piece",
    .hitboxScale = Vec3f(1.f, 0.1f, 1.f),
    .create = []() {
      return Mesh::Plane(2);
    },
    .attributes = {
      .type = MeshType::OCEAN,
      .ocean = {
        .turbulence = 0.1f
      }
    }
  },

  {
    .name = "vent-piece",
    .create = []() {
      return Mesh::Model({
        "./game/assets/decorations/vent-piece.obj",
        "./game/assets/decorations/vent-piece-lod.obj"
      });
    },
    .attributes = {
      .normals = "./game/assets/pipe-1-normals.png",
      .roughness = 0.2f
    }
  },
  {
    .name = "vent-corner",
    .create = []() {
      return Mesh::Model("./game/assets/decorations/vent-corner.obj");
    },
    .attributes = {
      .normals = "./game/assets/pipe-1-normals.png",
      .roughness = 0.2f
    }
  },
  {
    .name = "round-vent-piece",
    .create = []() {
      return Mesh::Model("./game/assets/decorations/round-vent-piece.obj");
    },
    .attributes = {
      .normals = "./game/assets/pipe-1-normals.png",
      .roughness = 0.4f
    }
  },
  {
    .name = "round-vent-corner",
    .create = []() {
      return Mesh::Model("./game/assets/decorations/round-vent-corner.obj");
    },
    .attributes = {
      .normals = "./game/assets/pipe-1-normals.png",
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
    .name = "vertical-banner",
    .hitboxScale = Vec3f(0.2f, 1.f, 0.05f),
    .create = []() {
      return Mesh::Model({
        "./game/assets/decorations/vertical-banner.obj",
        "./game/assets/decorations/vertical-banner-lod.obj"
      });
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .animation = {
        .type = PresetAnimationType::CLOTH,
        .factor = 6.f
      },
      .emissivity = 0.1f,
      .roughness = 0.4f
    }
  },
  {
    .name = "triangle-flag",
    .hitboxScale = Vec3f(0.7f, 1.f, 0.05f),
    .create = []() {
      return Mesh::Model("./game/assets/decorations/triangle-flag.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .animation = {
        .type = PresetAnimationType::CLOTH,
        .factor = 8.f
      },
      .emissivity = 0.1f,
      .roughness = 0.4f
    }
  },
  {
    .name = "circle-sign",
    .dynamic = true,
    .hitboxScale = Vec3f(1.f, 1.f, 0.2f),
    .create = []() {
      return Mesh::Model("./game/assets/decorations/circle-sign-board.obj");
    },
    .attributes = {
      .roughness = 0.2f
    },
    .pieces = {
      {
        .name = "circle-sign-frame",
        .create = []() {
          return Mesh::Model({
            "./game/assets/decorations/circle-sign-frame.obj",
            "./game/assets/decorations/circle-sign-frame-lod.obj"
          });
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = source.color;
        },
        .attributes = {
          .roughness = 0.2f
        }
      },
      {
        .name = "circle-sign-board",
        .create = []() {
          return Mesh::Model("./game/assets/decorations/circle-sign-board.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f::lerp(source.color.toVec3f(), Vec3f(1.f, 0.7f, 0.4f), 0.7f);
        }
      }
    }
  },
  {
    .name = "gate-sign",
    .hitboxScale = Vec3f(1.f, 0.5f, 0.05f),
    .create = []() {
      return Mesh::Model("./game/assets/decorations/gate-sign.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },
  {
    .name = "canopy",
    .hitboxScale = Vec3f(1.f, 0.2f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/decorations/canopy.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .maxCascade = 4,
      .animation = {
        .type = PresetAnimationType::CLOTH,
        .speed = 0.5f,
        .factor = 40.f
      }
    }
  },
  {
    .name = "flag-wire-spawn",
    .dynamic = true,
    .create = []() {
      return Mesh::Sphere(8);
    }
  },
  {
    .name = "flag-pivot",
    .create = []() {
      return Mesh::Model("./game/assets/decorations/flag-pivot.obj");
    }
  },
  {
    .name = "fish-kite",
    .moving = true,
    .hitboxScale = Vec3f(1.f, 0.5f, 0.5f),
    .create = []() {
      return Mesh::Model("./game/assets/decorations/fish-kite.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .animation = {
        .type = PresetAnimationType::CLOTH,
        .factor = 12.f
      },
      .emissivity = 0.3f
    },
    .pieces = {
      {
        .name = "fish-kite-fins",
        .moving = true,
        .create = []() {
          return Mesh::Model("./game/assets/decorations/fish-kite-fins.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = source.color.toVec3f() * 2.f;
        },
        .attributes = {
          .type = MeshType::PRESET_ANIMATED,
          .animation = {
            .type = PresetAnimationType::CLOTH,
            .factor = 20.f
          },
          .emissivity = 0.3f
        }
      }
    }
  },
  {
    .name = "flower-kite",
    .moving = true,
    .create = []() {
      return Mesh::Model("./game/assets/decorations/flower-kite.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .maxCascade = 4,
      .animation = {
        .type = PresetAnimationType::CLOTH,
        .factor = 20.f
      },
      .emissivity = 0.6f
    }
  },
  {
    .name = "large-kite",
    .create = []() {
      return Mesh::Model("./game/assets/decorations/large-kite.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .maxCascade = 4,
      .animation = {
        .type = PresetAnimationType::CLOTH,
        .speed = 0.5f,
        .factor = 30.f
      }
    }
  },
  {
    .name = "balloon-windmill",
    .moving = true,
    .create = []() {
      return Mesh::Model("./game/assets/decorations/balloon-windmill.obj");
    },
    .attributes = {
      .maxCascade = 4
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
    .name = "billboards-1",
    .create = []() {
      return Mesh::Model("./game/assets/texture-cube.obj");
    },
    .attributes = {
      .texture = "./game/assets/decorations/billboards-1.png",
      .useMipmaps = false
    }
  },
  {
    .name = "billboards-2",
    .create = []() {
      return Mesh::Model("./game/assets/texture-cube.obj");
    },
    .attributes = {
      .texture = "./game/assets/decorations/billboards-2.png",
      .useMipmaps = false
    }
  },
  {
    .name = "circle-billboard-1",
    .create = []() {
      return Mesh::Model("./game/assets/decorations/circle-billboard.obj");
    },
    .attributes = {
      .texture = "./game/assets/decorations/circle-billboard-1.png",
      .maxCascade = 4,
      .useMipmaps = false
    }
  },

  {
    .name = "circle-billboard-2",
    .create = []() {
      return Mesh::Model("./game/assets/decorations/circle-billboard.obj");
    },
    .attributes = {
      .texture = "./game/assets/decorations/circle-billboard-2.png",
      .useMipmaps = false
    }
  },
  {
    .name = "barred-window",
    .create = []() {
      return Mesh::Model("./game/assets/decorations/barred-window.obj");
    },
    .attributes = {
      .texture = "./game/assets/decorations/barred-window.png"
    }
  },
  {
    .name = "hanging-sign",
    .moving = true,
    .create = []() {
      return Mesh::Model("./game/assets/decorations/hanging-sign.obj");
    },
    .attributes = {
      .texture = "./game/assets/decorations/hanging-sign.png"
    }
  }
};