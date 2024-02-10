#include "Gamma.h"

#include "mesh_library/decorations.h"

using namespace Gamma;

std::vector<MeshAsset> decoration_meshes = {
  {
    .name = "vertical-banner",
    .hitboxScale = Vec3f(0.2f, 1.f, 0.05f),
    .create = []() {
      return Mesh::Model("./game/assets/decorations/vertical-banner.obj");
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
      return Mesh::Model("./game/assets/decorations/circle-sign-frame.obj");
    },
    .pieces = {
      {
        .name = "circle-sign-frame",
        .create = []() {
          return Mesh::Model("./game/assets/decorations/circle-sign-frame.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = source.color;
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
    .name = "balloon-windmill",
    .moving = true,
    .create = []() {
      return Mesh::Model("./game/assets/decorations/balloon-windmill.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  }
};