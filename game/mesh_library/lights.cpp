#include "Gamma.h"

#include "mesh_library/lights.h"

using namespace Gamma;

std::vector<MeshAsset> light_meshes = {
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
    .name = "wall-lamp",
    .create = []() {
      return Mesh::Model("./game/assets/decorations/wall-lamp.obj");
    },
    .attributes = {
      .roughness = 0.2f
    },
    .pieces = {
      {
        .name = "wall-lamp-bulb",
        .create = []() {
          return Mesh::Model("./game/assets/decorations/wall-lamp-bulb.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(1.f, 0.9f, 0.6f);
        },
      }
    }
  }
};