#include "Gamma.h"

#include "mesh_library/overworld_stairs.h"

using namespace Gamma;

std::vector<MeshAsset> overworld_stairs_meshes = {
  {
    .name = "concrete-rim",
    .create = []() {
      return Mesh::Model("./game/assets/buildings/concrete-rim.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },
  {
    .name = "exhaust-fan",
    .create = []() {
      return Mesh::Model("./game/assets/decorations/exhaust-fan-frame.obj");
    },
    .attributes = {
      .maxCascade = 4
    },
    .pieces = {
      {
        .name = "exhaust-fan-blades",
        .moving = true,
        .create = []() {
          return Mesh::Model("./game/assets/decorations/exhaust-fan.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.5f);
        },
        .attributes = {
          .roughness = 0.1f
        }
      }
    }
  },
  {
    .name = "concrete-b1",
    .hitboxScale = Vec3f(1.f, 1.f, 0.4f),
    .create = []() {
      return Mesh::Model("./game/assets/buildings/concrete-b1.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },
  {
    .name = "round-tower",
    .dynamic = true,
    .create = []() {
      return Mesh::Model("./game/assets/buildings/round-tower-base.obj");
    },
    .attributes = {
      .maxCascade = 4
    },
    .pieces = {
      {
        .name = "round-tower-base",
        .create = []() {
          return Mesh::Model("./game/assets/buildings/round-tower-base.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(1.f, 0.8f, 0.6f);
        }
      },
      {
        .name = "round-tower-supports",
        .create = []() {
          return Mesh::Model("./game/assets/buildings/round-tower-supports.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(1.f, 0.6f, 0.2f);
        }
      },
      {
        .name = "round-tower-roof",
        .create = []() {
          return Mesh::Model("./game/assets/buildings/round-tower-roof.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.8f, 0.4f, 0.2f);
        }
      }
    }
  },
  {
    .name = "bridge-2",
    .dynamic = true,
    .hitboxScale = Vec3f(0.3f, 0.2f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/buildings/bridge-2-floor.obj");
    },
    .attributes = {
      .maxCascade = 4
    },
    .pieces = {
      {
        .name = "bridge-2-floor",
        .create = []() {
          return Mesh::Model("./game/assets/buildings/bridge-2-floor.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(1.f, 0.6f, 0.2f);
        }
      },
      {
        .name = "bridge-2-railing",
        .create = []() {
          return Mesh::Model("./game/assets/buildings/bridge-2-railing.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.7f, 0.2f, 0.1f);
        }
      }
    }
  }
};