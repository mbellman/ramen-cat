#include "Gamma.h"

#include "mesh_library/uniques.h"

using namespace Gamma;

std::vector<MeshAsset> unique_meshes = {
  {
    .name = "big-tree-trunk",
    .hitboxScale = Vec3f(0.3f, 1.f, 0.3f),
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
    .create = []() {
      return Mesh::Model("./game/assets/uniques/water-tower-pipe.obj");
    },
    .attributes = {
      .maxCascade = 4,
      .roughness = 0.2f
    }
  },
  {
    .name = "lower-lake",
    .hitboxScale = Vec3f(1.f, 0.1f, 1.f),
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
        .create = []() {
          return Mesh::Model("./game/assets/uniques/lower-lakebed.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.3f, 0.8f, 0.2f);
        }
      }
    }
  },
  {
    .name = "pink-palace",
    .create = []() {
      return Mesh::Model("./game/assets/uniques/pink-palace.obj");
    },
    .attributes = {
      .maxCascade = 4
    },
    .pieces = {
      {
        .name = "pink-palace-trim",
        .create = []() {
          return Mesh::Model("./game/assets/uniques/pink-palace-trim.obj");
        }
      },
      {
        .name = "pink-palace-top",
        .create = []() {
          return Mesh::Model("./game/assets/uniques/pink-palace-top.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(1.f, 0.8f, 0.7f);
        }
      },
      {
        .name = "pink-palace-roof",
        .create = []() {
          return Mesh::Model("./game/assets/uniques/pink-palace-roof.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.7f, 0.2f, 0.1f);
        }
      }
    }
  }
};