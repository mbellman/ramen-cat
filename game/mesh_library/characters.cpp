#include "Gamma.h"

#include "mesh_library/characters.h"

using namespace Gamma;

std::vector<MeshAsset> character_meshes = {
  {
    .name = "person",
    .moving = true,
    .defaultScale = Vec3f(65.f),
    .hitboxScale = Vec3f(0.6f, 1.4f, 0.2f),
    .create = []() {
      return Mesh::Model("./game/assets/person.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .animation = {
        .type = PresetAnimationType::NPC,
      },
      .emissivity = 0.5f,
      .roughness = 0.8f
    },
    .pieces = {
      {
        .name = "t-shirt",
        .create = []() {
          return Mesh::Model("./game/assets/characters/t-shirt.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(
            0.2f + 0.6f * Gm_Modf(source.position.x, 1.f),
            0.5f + 0.5f * Gm_Modf(source.position.x + source.position.z, 1.f),
            0.3f + 0.7f * Gm_Modf(source.position.z, 1.f)
          );
        },
        .attributes = {
          .type = MeshType::PRESET_ANIMATED,
          .animation = {
            .type = PresetAnimationType::NPC,
          },
          .emissivity = 0.1f,
          .roughness = 1.f
        }
      },
      {
        .name = "pants",
        .create = []() {
          return Mesh::Model("./game/assets/characters/pants.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.1f, 0.3f, 0.7f);
        },
        .attributes = {
          .type = MeshType::PRESET_ANIMATED,
          .animation = {
            .type = PresetAnimationType::NPC,
          },
          .emissivity = 0.1f,
          .roughness = 0.9f
        }
      },
      {
        .name = "hair",
        .create = []() {
          return Mesh::Model("./game/assets/characters/hair.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.8f, 0.5f, 0.2f) * Gm_Modf(source.position.x, 1.f);
        },
        .attributes = {
          .type = MeshType::PRESET_ANIMATED,
          .animation = {
            .type = PresetAnimationType::NPC,
          },
          .emissivity = 0.2f,
          .roughness = 0.5f
        }
      }
    }
  },

  {
    .name = "girl",
    .moving = true,
    .defaultScale = Vec3f(65.f),
    .hitboxScale = Vec3f(0.6f, 1.4f, 0.2f),
    .create = []() {
      return Mesh::Model("./game/assets/characters/girl.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .animation = {
        .type = PresetAnimationType::NPC,
      },
      .emissivity = 0.5f,
      .roughness = 0.8f
    },
    .pieces = {
      {
        .name = "tank-top",
        .create = []() {
          return Mesh::Model("./game/assets/characters/tank-top.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(
            0.2f + 0.6f * Gm_Modf(source.position.x, 1.f),
            0.5f + 0.5f * Gm_Modf(source.position.x + source.position.z, 1.f),
            0.3f + 0.7f * Gm_Modf(source.position.z, 1.f)
          );
        },
        .attributes = {
          .type = MeshType::PRESET_ANIMATED,
          .animation = {
            .type = PresetAnimationType::NPC,
          },
          .emissivity = 0.1f,
          .roughness = 1.f
        }
      },
      {
        .name = "girl-shorts",
        .create = []() {
          return Mesh::Model("./game/assets/characters/girl-shorts.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.05f, 0.1f, 0.5f);
        },
        .attributes = {
          .type = MeshType::PRESET_ANIMATED,
          .animation = {
            .type = PresetAnimationType::NPC,
          },
          .emissivity = 0.1f,
          .roughness = 0.9f
        }
      },
      {
        .name = "girl-hair",
        .create = []() {
          return Mesh::Model("./game/assets/characters/girl-hair.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.8f, 0.5f, 0.2f) * Gm_Modf(source.position.x, 1.f);
        },
        .attributes = {
          .type = MeshType::PRESET_ANIMATED,
          .animation = {
            .type = PresetAnimationType::NPC,
          },
          .emissivity = 0.2f,
          .roughness = 0.5f
        }
      }
    }
  },
};