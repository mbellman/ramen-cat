#include "Gamma.h"

#include "mesh_library/transportation.h"

using namespace Gamma;

std::vector<MeshAsset> transportation_meshes = {
  // Vehicle track pivots
  {
    .name = "vehicle-stop",
    .dynamic = true,
    .defaultColor = Vec3f(1.f, 0, 0),
    .defaultScale = Vec3f(150.f),
    .create = []() {
      return Mesh::Sphere(8);
    }
  },
  {
    .name = "vehicle-spawn",
    .dynamic = true,
    .defaultColor = Vec3f(0, 1., 0),
    .defaultScale = Vec3f(150.f),
    .create = []() {
      return Mesh::Sphere(8);
    }
  },
  {
    .name = "vehicle-track",
    .dynamic = true,
    .defaultColor = Vec3f(0, 0, 1.f),
    .defaultScale = Vec3f(100.f),
    .create = []() {
      return Mesh::Sphere(8);
    }
  },

  // Structures/buildings relating to transportation
  {
    .name = "cable-car-pole",
    .hitboxScale = Vec3f(0.2f, 1.f, 0.2f),
    .create = []() {
      return Mesh::Model("./game/assets/transportation/cable-car-pole.obj");
    },
    .attributes = {
      .maxCascade = 4,
      .roughness = 0.2f
    }
  },

  {
    .name = "cable-car-station",
    .create = []() {
      return Mesh::Model("./game/assets/transportation/cable-car-station-walls.obj");
    },
    .attributes = {
      .texture = "./game/assets/transportation/cable-car-station-walls.png",
      .maxCascade = 4
    },
    .pieces = {
      {
        .name = "ccs-floor",
        .create = []() {
          return Mesh::Model("./game/assets/transportation/cable-car-station-floor.obj");
        },
        .attributes = {
          .texture = "./game/assets/transportation/cable-car-station-floor.png"
        }
      },
      {
        .name = "ccs-supports",
        .create = []() {
          return Mesh::Model("./game/assets/transportation/cable-car-station-supports.obj");
        },
        .attributes = {
          .texture = "./game/assets/transportation/cable-car-station-supports.png",
        }
      },
      {
        .name = "ccs-roof",
        .create = []() {
          return Mesh::Model("./game/assets/transportation/cable-car-station-roof.obj");
        },
        .rebuild = [](auto& source, auto& piece) {
          piece.color = Vec3f(0.7f, 0.5f, 0.3f);
        }
      }
    }
  },

  // Vehicles
  {
    .name = "small-train",
    .moving = true,
    .create = []() {
      return Mesh::Model("./game/assets/transportation/small-train.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  },
  {
    .name = "cable-car",
    .moving = true,
    .create = []() {
      return Mesh::Model("./game/assets/transportation/cable-car.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  }
};