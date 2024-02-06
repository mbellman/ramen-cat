#include "mesh_library/overworld_city.h"

using namespace Gamma;

std::vector<MeshAsset> overworld_city_meshes = {
  {
    .name = "b1",
    .dynamic = true,
    .hitboxScale = Vec3f(1.f, 1.5f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/buildings/b1-base.obj");
    },
    .attributes = {
      .maxCascade = 4
    },
    .pieces = {
      {
        .name = "b1-base",
        .create = []() {
          return Mesh::Model("./game/assets/buildings/b1-base.obj");
        }
      },
      {
        .name = "b1-levels",
        .create = []() {
          return Mesh::Model({
            "./game/assets/buildings/b1-levels-hd.obj",
            "./game/assets/buildings/b1-levels.obj"
          });
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = source.color;
        },
        .attributes = {
          .useLowestLevelOfDetailForShadows = false
        }
      },
      {
        .name = "b1-windows",
        .create = []() {
          return Mesh::Model("./game/assets/buildings/b1-windows.obj");
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
    .name = "b2",
    .dynamic = true,
    .hitboxScale = Vec3f(0.9f, 1.2f, 0.8f),
    .create = []() {
      return Mesh::Model("./game/assets/buildings/b2-base.obj");
    },
    .attributes = {
      .maxCascade = 4
    },
    .pieces = {
      {
        .name = "b2-base",
        .create = []() {
          return Mesh::Model("./game/assets/buildings/b2-base.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f::lerp(source.color.toVec3f(), Vec3f(1.f), 0.5f);
        }
      },
      {
        .name = "b2-levels",
        .create = []() {
          return Mesh::Model({
            "./game/assets/buildings/b2-levels-hd.obj",
            "./game/assets/buildings/b2-levels.obj"
          });
        },
        .rebuild = [](auto& base, auto& piece) {
          piece.color = Vec3f(1.f);
        },
        .attributes = {
          .useLowestLevelOfDetailForShadows = false
        }
      },
      {
        .name = "b2-columns",
        .create = []() {
          return Mesh::Model("./game/assets/buildings/b2-columns.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = source.color;
        }
      },
      {
        .name = "b2-windows",
        .create = []() {
          return Mesh::Model("./game/assets/buildings/b2-windows.obj");
        },
        .rebuild = [](auto& base, auto& piece) {
          piece.color = Vec3f(0.5f, 0.75f, 1.f);
        },
        .attributes = {
          .roughness = 0.1f
        }
      }
    }
  },
  {
    .name = "b3",
    .dynamic = true,
    .hitboxScale = Vec3f(1.f, 1.2f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/buildings/b3-base.obj");
    },
    .attributes = {
      .maxCascade = 4
    },
    .pieces = {
      {
        .name = "b3-base",
        .create = []() {
          return Mesh::Model("./game/assets/buildings/b3-base.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f::lerp(source.color.toVec3f(), Vec3f(1.f), 0.5f);
        }
      },
      {
        .name = "b3-levels",
        .create = []() {
          return Mesh::Model("./game/assets/buildings/b3-levels.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(1.f);
        }
      },
      {
        .name = "b3-columns",
        .create = []() {
          return Mesh::Model("./game/assets/buildings/b3-columns.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = source.color;
        }
      }
    }
  },
  {
    .name = "wood-house",
    .dynamic = true,
    .create = []() {
      return Mesh::Model("./game/assets/buildings/wood-house-base.obj");
    },
    .attributes = {
      .maxCascade = 4
    },
    .pieces = {
      {
        .name = "wood-house-base",
        .create = []() {
          return Mesh::Model("./game/assets/buildings/wood-house-base.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.8f, 0.6f, 0.4f);
        }
      },
      {
        .name = "wood-house-roof",
        .create = []() {
          return Mesh::Model("./game/assets/buildings/wood-house-roof.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(1.f, 0.6f, 0.2f);
        }
      },
    }
  },
  {
    .name = "wood-awning",
    .hitboxScale = Vec3f(1.f, 1.2f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/buildings/wood-awning.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },
  {
    .name = "wood-tower",
    .defaultColor = Vec3f(1.f, 0.8f, 0.4f),
    .hitboxScale = Vec3f(1.f, 1.2f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/buildings/wood-tower.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },
  {
    .name = "wood-tower-top",
    .defaultColor = Vec3f(0.6f, 0.4f, 0.2f),
    .hitboxScale = Vec3f(1.f, 1.4f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/buildings/wood-tower-top.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },
  {
    .name = "wood-electrical-pole",
    .defaultColor = Vec3f(1.f, 0.8f, 0.4f),
    .hitboxScale = Vec3f(0.5f, 1.4f, 0.5f),
    .create = []() {
      return Mesh::Model("./game/assets/buildings/wood-electrical-pole.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },
  {
    .name = "wood-facade",
    .dynamic = true,
    .hitboxScale = Vec3f(1.f, 1.f, 0.1f),
    .create = []() {
      return Mesh::Model("./game/assets/buildings/wood-facade-base.obj");
    }
  },
  {
    .name = "wave-sign",
    .dynamic = true,
    .hitboxScale = Vec3f(1.f, 0.2f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/buildings/wave-sign.obj");
    }
  },
  {
    .name = "spiral-roof",
    .create = []() {
      return Mesh::Model("./game/assets/buildings/spiral-roof.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },
  {
    .name = "sign-roof",
    .dynamic = true,
    .create = []() {
      return Mesh::Model("./game/assets/buildings/sign-roof.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },
};