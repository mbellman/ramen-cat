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
  }
};