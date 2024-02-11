#include "Gamma.h"

#include "mesh_library/procedural.h"

using namespace Gamma;

std::vector<MeshAsset> procedural_meshes = {
  {
    .name = "plant-strip",
    .dynamic = true,
    .defaultColor = Vec3f(0.5f, 1.f, 0.6f),
    .create = []() {
      return Mesh::Cube();
    }
  },
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
  },
  {
    .name = "wood-building-1",
    .hitboxScale = Vec3f(1.f, 0.3f, 0.5f),
    .create = []() {
      return Mesh::Model("./game/assets/buildings/wood-building-1.obj");
    },
    .attributes = {
      .texture = "./game/assets/wood-beam.png",
      .maxCascade = 4
    }
  }
};

std::vector<MeshAsset> procedural_mesh_parts = {
  // plant-strip
  {
    .name = "p_shrub",
    .maxInstances = 10000,
    .create = []() {
      return Mesh::Model("./game/assets/plants/shrub.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/plants/shrub.png",
      .animation = {
        .type = PresetAnimationType::FLOWER,
        .factor = 6.f
      },
      .roughness = 1.f
    }
  },
  {
    .name = "p_shrub-2",
    .maxInstances = 10000,
    .create = []() {
      return Mesh::Model("./game/assets/plants/shrub.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/plants/shrub-2.png",
      .animation = {
        .type = PresetAnimationType::FLOWER,
        .factor = 6.f
      },
      .roughness = 1.f
    }
  },
  {
    .name = "p_banana-plant",
    .hitboxScale = Vec3f(1.f, 0.3f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/plants/banana-plant.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/plants/banana-plant.png",
      .maxCascade = 4,
      .animation = {
        .type = PresetAnimationType::LEAF,
        .factor = 3.f
      },
      .roughness = 0.4f
    }
  },
  {
    .name = "p_weeds",
    .hitboxScale = Vec3f(1.f, 1.f, 0.2f),
    .create = []() {
      return Mesh::Model("./game/assets/weeds.obj");
    },
    .attributes = {
      .type = MeshType::PRESET_ANIMATED,
      .texture = "./game/assets/weeds.png",
      .maxCascade = 2,
      .animation = {
        .type = PresetAnimationType::FLOWER,
        .factor = 3.f
      },
      .emissivity = 0.1f,
      .roughness = 1.f
    }
  },

  // concrete-stack
  {
    .name = "p_concrete",
    .maxInstances = 10000,
    .create = []() {
      return Mesh::Cube();
    },
    .attributes = {
      .maxCascade = 4,
      .roughness = 0.9f
    }
  },

  // mini-house/mini-house-double
  {
    .name = "p_mini-house",
    .create = []() {
      return Mesh::Cube();
    },
    .attributes = {
      .maxCascade = 4,
      .roughness = 0.7f
    }
  },
  {
    .name = "p_mini-house-roof",
    .create = []() {
      return Mesh::Model({
        "./game/assets/japanese-roof.obj",
        "./game/assets/japanese-roof-lod.obj"
      });
    },
    .attributes = {
      .maxCascade = 4,
      .roughness = 0.3f
    }
  },
  {
    .name = "p_mini-house-wood-beam",
    .maxInstances = 10000,
    .create = []() {
      return Mesh::Model("./game/assets/procedural/mini-house-wood-beam.obj");
    },
    .attributes = {
      .texture = "./game/assets/wood-beam.png",
      .roughness = 0.5f
    }
  },
  {
    .name = "p_mini-house-window",
    .create = []() {
      return Mesh::Model("./game/assets/procedural/mini-house-window.obj");
    },
    .attributes = {
      .texture = "./game/assets/windows-1.png",
      .roughness = 0.5f
    }
  },
  {
    .name = "p_mini-house-board",
    .maxInstances = 10000,
    .create = []() {
      return Mesh::Plane(2);
    },
    .attributes = {
      .texture = "./game/assets/wood-beam.png",
      .normals = "./game/assets/wood-beam-normals.png",
      .roughness = 0.5f
    }
  },

  // balloon-windmill
  {
    .name = "p_balloon-windmill-blades",
    .create = []() {
      return Mesh::Model("./game/assets/decorations/balloon-windmill-blades.obj");
    },
    .attributes = {
      .maxCascade = 4
    }
  }
};