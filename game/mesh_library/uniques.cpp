#include "Gamma.h"

#include "mesh_library/uniques.h"

using namespace Gamma;

std::vector<MeshAsset> unique_meshes = {
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
    .maxInstances = 20,
    .create = []() {
      return Mesh::Model("./game/assets/sculpture-stand.obj");
    }
  },
  {
    .name = "umimura-sculpture-fan",
    .moving = true,
    .hitboxScale = Vec3f(1.f, 0.2f, 1.f),
    .maxInstances = 20,
    .create = []() {
      return Mesh::Model("./game/assets/sculpture-fan.obj");
    },
    .attributes = {
      .texture = "./game/assets/sculpture-fan.png"
    }
  },
  {
    .name = "big-tree-trunk",
    .hitboxScale = Vec3f(0.3f, 1.f, 0.3f),
    .maxInstances = 5,
    .create = []() {
      return Mesh::Model("./game/assets/uniques/big-tree-trunk.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },
  {
    .name = "water-tower-pipe",
    .hitboxScale = Vec3f(0.2f, 0.3f, 1.f),
    .maxInstances = 5,
    .create = []() {
      return Mesh::Model("./game/assets/uniques/water-tower-pipe.obj");
    },
    .attributes = {
      .maxCascade = 4,
      .roughness = 0.2f
    }
  },
  {
    .name = "sun-sign",
    .hitboxScale = Vec3f(1.f, 1.f, 0.1f),
    .maxInstances = 5,
    .create = []() {
      return Mesh::Model("./game/assets/uniques/sun-sign.obj");
    },
    .attributes = {
      .maxCascade = 4,
      .emissivity = 0.4f
    }
  },
  {
    .name = "moon-sign",
    .hitboxScale = Vec3f(1.f, 1.f, 0.1f),
    .maxInstances = 5,
    .create = []() {
      return Mesh::Model("./game/assets/uniques/moon-sign.obj");
    },
    .attributes = {
      .maxCascade = 4,
      .emissivity = 0.4f
    }
  },
  {
    .name = "lower-lake",
    .hitboxScale = Vec3f(1.f, 0.1f, 1.f),
    .maxInstances = 5,
    .create = []() {
      return Mesh::Model("./game/assets/uniques/lower-lake.obj");
    },
    .attributes = {
      .type = MeshType::OCEAN,
      .maxCascade = 4,
      .ocean = {
        .turbulence = 0.01f
      }
    },
    .pieces = {
      {
        .name = "lower-lakebed",
        .maxInstances = 5,
        .create = []() {
          return Mesh::Model("./game/assets/uniques/lower-lakebed.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.3f, 0.7f, 0.2f);
        },
        .attributes = {
          .roughness = 1.f
        }
      }
    }
  },
  {
    .name = "pink-palace",
    .maxInstances = 5,
    .create = []() {
      return Mesh::Model("./game/assets/uniques/pink-palace.obj");
    },
    .attributes = {
      .maxCascade = 4
    },
    .pieces = {
      {
        .name = "pink-palace-trim",
        .maxInstances = 5,
        .create = []() {
          return Mesh::Model("./game/assets/uniques/pink-palace-trim.obj");
        }
      },
      {
        .name = "pink-palace-top",
        .maxInstances = 5,
        .create = []() {
          return Mesh::Model("./game/assets/uniques/pink-palace-top.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(1.f, 0.8f, 0.7f);
        }
      },
      {
        .name = "pink-palace-roof",
        .maxInstances = 5,
        .create = []() {
          return Mesh::Model("./game/assets/uniques/pink-palace-roof.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.7f, 0.2f, 0.1f);
        }
      },
      {
        .name = "pink-palace-windows",
        .maxInstances = 5,
        .create = []() {
          return Mesh::Model("./game/assets/uniques/pink-palace-windows.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.5f, 0.75f, 1.f);
        },
        .attributes = {
          .roughness = 0.1f
        }
      }
    }
  },
  {
    .name = "landmark-sign",
    .maxInstances = 20,
    .create = []() {
      return Mesh::Model("./game/assets/uniques/landmark-sign-supports.obj");
    },
    .attributes = {
      .maxCascade = 4
    },
    .pieces = {
      {
        .name = "landmark-sign-roof",
        .maxInstances = 20,
        .create = []() {
          return Mesh::Model("./game/assets/uniques/landmark-sign-roof.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.1f, 0.3f, 0.6f);
        },
        .attributes = {
          .roughness = 0.3f
        }
      }
    }
  },
  {
    .name = "city-tree-trunk",
    .hitboxScale = Vec3f(0.2f, 1.f, 0.2f),
    .maxInstances = 5,
    .create = []() {
      return Mesh::Model("./game/assets/uniques/city-tree-trunk.obj");
    },
    .attributes = {
      .maxCascade = 4
    },
    .pieces = {
      {
        .name = "city-tree-branches",
        .create = []() {
          return Mesh::Model("./game/assets/uniques/city-tree-branches.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(1.f, 0.7f, 0.4f);
        }
      }
    }
  }
};