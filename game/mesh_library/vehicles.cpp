#include "Gamma.h"

#include "mesh_library/vehicles.h"

using namespace Gamma;

std::vector<MeshAsset> vehicle_meshes = {
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

  {
    .name = "cable-car-pole",
    .hitboxScale = Vec3f(0.2f, 1.f, 0.2f),
    .create = []() {
      return Mesh::Model("./game/assets/vehicles/cable-car-pole.obj");
    },
    .attributes = {
      .maxCascade = 4,
      .roughness = 0.2f
    }
  },

  // Vehicles
  {
    .name = "small-train",
    .moving = true,
    .create = []() {
      return Mesh::Model("./game/assets/vehicles/small-train.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  }
};