#include "Gamma.h"

#include "mesh_library/procedural.h"

using namespace Gamma;

std::vector<MeshAsset> procedural_meshes = {
  {
    .name = "concrete-stack",
    .dynamic = true,
    .create = []() {
      return Mesh::Cube();
    }
  },
  {
    .name = "mini-house",
    .dynamic = true,
    .defaultScale = Vec3f(180.f, 200.f, 100.f),
    .scalingFactor = Vec3f(0.f),
    .create = []() {
      return Mesh::Cube();
    }
  },
  {
    .name = "mini-house-double",
    .dynamic = true,
    .defaultScale = Vec3f(180.f, 200.f, 200.f),
    .scalingFactor = Vec3f(0.f),
    .create = []() {
      return Mesh::Cube();
    }
  }
};