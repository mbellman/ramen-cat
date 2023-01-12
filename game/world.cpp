#include "world.h"
#include "collisions.h"
#include "macros.h"

using namespace Gamma;

std::vector<MeshAsset> World::meshAssets = {
  {
    .name = "lamp",
    .defaultColor = Vec3f(1.f),
    .create = []() {
      return Mesh::Model("./game/assets/lamp.obj");
    },
    .attributes = {
      .texture = "./game/assets/lamp.png",
      .emissivity = 3.f
    }
  },
  {
    .name = "concrete-slab",
    .defaultColor = Vec3f(0.5f),
    .create = []() {
      return Mesh::Cube();
    },
    .attributes = {
      .texture = "./game/assets/concrete.png",
      .normalMap = "./game/assets/concrete-normals.png"
    }
  },
  {
    .name = "metal-panel",
    .defaultColor = Vec3f(0.5f),
    .create = []() {
      return Mesh::Cube();
    },
    .attributes = {
      .texture = "./game/assets/metal-panel.png",
      .normalMap = "./game/assets/metal-panel-normals.png"
    }
  },
  {
    .name = "floor-1",
    .defaultColor = Vec3f(1.f),
    .create = []() {
      return Mesh::Model("./game/assets/floor-1.obj");
    },
    .attributes = {
      .texture = "./game/assets/floor-1.png",
      .normalMap = "./game/assets/floor-1-normals.png",
    }
  },
  {
    .name = "wall-1",
    .defaultColor = Vec3f(1.f),
    .scalingFactor = Vec3f(1.f, 0, 1.f),
    .create = []() {
      return Mesh::Plane(2);
    },
    .attributes = {
      .texture = "./game/assets/wall-1.png",
      .normalMap = "./game/assets/wall-1-normals.png"
    }
  },
  {
    .name = "wall-2",
    .defaultColor = Vec3f(1.f),
    .scalingFactor = Vec3f(1.f, 0, 1.f),
    .create = []() {
      return Mesh::Plane(2);
    },
    .attributes = {
      .texture = "./game/assets/wall-2.png",
      .normalMap = "./game/assets/wall-2-normals.png"
    }
  },
  {
    .name = "sign-1",
    .defaultColor = Vec3f(1.f),
    .hitboxScale = Vec3f(0.1f, 0.5f, 1.f),
    .create = []() {
      return Mesh::Model("./game/assets/sign-1.obj");
    },
    .attributes = {
      .texture = "./game/assets/sign-1.png",
      .emissivity = 0.5f
    }
  },
  {
    .name = "door-1",
    .defaultColor = Vec3f(1.f),
    .hitboxScale = Vec3f(0.5f, 1.f, 0.1f),
    .create = []() {
      return Mesh::Model("./game/assets/door-1.obj");
    },
    .attributes = {
      .texture = "./game/assets/door-1.png",
      .emissivity = 0.25f
    }
  },
  {
    .name = "windows-1",
    .defaultColor = Vec3f(1.f),
    .hitboxScale = Vec3f(1.f, 1.f, 0.1f),
    .create = []() {
      return Mesh::Model("./game/assets/windows-1.obj");
    },
    .attributes = {
      .texture = "./game/assets/windows-1.png",
      .emissivity = 0.25f
    }
  },
  {
    .name = "corrugated-metal",
    .defaultColor = Vec3f(0.5f),
    .create = []() {
      return Mesh::Model("./game/assets/corrugated-metal.obj");
    }
  },
  {
    .name = "pipe-1",
    .defaultColor = Vec3f(1.f),
    .hitboxScale = Vec3f(0.3f, 1.f, 0.3f),
    .create = []() {
      return Mesh::Model("./game/assets/pipe-1.obj");
    },
    .attributes = {
      .texture = "./game/assets/pipe-1.png",
      .normalMap = "./game/assets/pipe-1-normals.png"
    }
  },
  {
    .name = "roof",
    .defaultColor = Vec3f(1.f),
    .create = []() {
      return Mesh::Model("./game/assets/roof.obj");
    },
    .attributes = {
      .texture = "./game/assets/roof.png"
    }
  },
  {
    .name = "staircase",
    .dynamic = true,
    .defaultColor = Vec3f(0, 1.f, 0),
    .create = []() {
      return Mesh::Cube();
    }
  }
};

std::vector<MeshAsset> World::dynamicMeshPieces = {
  {
    .name = "stair-step",
    .defaultColor = Vec3f(0.f),
    .maxInstances = 10000,
    .create = []() {
      return Mesh::Cube();
    }
  }
};

// @temporary
struct Platform {
  Vec3f position;
  Vec3f scale;
  Vec3f rotation;
  Vec3f color;
};

internal void loadCollisionPlanes(GmContext* context, GameState& state) {
  u64 start = Gm_GetMicroseconds();

  // @todo eventually store as binary data
  auto worldData = Gm_LoadFileContents("./game/data_collision_planes.txt");
  auto lines = Gm_SplitString(worldData, "\n");

  state.collisionPlanes.clear();
  objects("platform").reset();

  for (auto& line : lines) {
    if (line.size() == 0) {
      continue;
    }

    auto parts = Gm_SplitString(line, ",");
    auto& platform = create_object_from("platform");

    #define df(n) stof(parts[n])
    #define di(n) stoi(parts[n])

    platform.position = Vec3f(df(0), df(1), df(2));
    platform.scale = Vec3f(df(3), df(4), df(5));
    platform.rotation = Quaternion(df(6), df(7), df(8), df(9));
    platform.color = pVec4(di(10), di(11), di(12));

    commit(platform);

    Collisions::addObjectCollisionPlanes(platform, state.collisionPlanes);
  }

  Console::log("Loaded collision planes in", Gm_GetMicroseconds() - start, "us");
}

internal void loadWorldObjects(GmContext* context) {
  u64 start = Gm_GetMicroseconds();

  // @todo eventually store as binary data
  auto worldData = Gm_LoadFileContents("./game/data_world_objects.txt");
  auto lines = Gm_SplitString(worldData, "\n");

  std::string meshName;

  for (u32 i = 0; i < lines.size(); i++) {
    auto& line = lines[i];

    if (line.size() == 0) {
      continue;
    }

    if (line[0] == '@') {
      meshName = line.substr(1);
    } else {
      auto parts = Gm_SplitString(line, ",");
      auto& object = create_object_from(meshName);

      #define df(n) stof(parts[n])
      #define di(n) stoi(parts[n])

      object.position = Vec3f(df(0), df(1), df(2));
      object.scale = Vec3f(df(3), df(4), df(5));
      object.rotation = Quaternion(df(6), df(7), df(8), df(9));
      object.color = pVec4(di(10), di(11), di(12));

      commit(object);
    }
  }

  Console::log("Loaded world objects in", Gm_GetMicroseconds() - start, "us");
}

internal void loadLights(GmContext* context) {
  u64 start = Gm_GetMicroseconds();

  // @todo eventually store as binary data
  auto worldData = Gm_LoadFileContents("./game/data_lights.txt");
  auto lines = Gm_SplitString(worldData, "\n");

  for (auto& line : lines) {
    if (line.size() == 0) {
      continue;
    }

    auto parts = Gm_SplitString(line, ",");
    auto& light = create_light(LightType::POINT);

    #define df(n) stof(parts[n])

    light.position = Vec3f(df(0), df(1), df(2));
    light.radius = df(3);
    light.color = Vec3f(df(4), df(5), df(6));
    light.power = df(7);
  }

  Console::log("Loaded lights in", Gm_GetMicroseconds() - start, "us");
}

internal void rebuildDynamicStaircases(GmContext* context) {
  objects("stair-step").reset();

  const auto sidePoints = {
    Vec3f(1.f, 0, 0),
    Vec3f(-1.f, 0, 0),
    Vec3f(0, 1.f, 0),
    Vec3f(0, -1.f, 0),
    Vec3f(0, 0, 1.f),
    Vec3f(0, 0, -1.f)
  };

  std::vector<Vec3f> t_points;

  for (auto& staircase : objects("staircase")) {
    auto& scale = staircase.scale;
    auto rotation = staircase.rotation.toMatrix4f();
    Vec3f rotationDirectionXz = staircase.rotation.getDirection().xz();
    float yRotation = -1.f * atan2f(rotationDirectionXz.z, rotationDirectionXz.x);

    t_points.clear();

    // Determine the transformed staircase bounds
    {
      for (auto& point : sidePoints) {
        t_points.push_back(staircase.position + (rotation * (scale * point)).toVec3f());
      }
    }

    Vec3f start;
    Vec3f end;

    // Establish the start and end of the staircase
    {
      float furthest = 0.f;

      for (auto& t_point : t_points) {
        for (auto& t_point2 : t_points) {
          float distance = (t_point2 - t_point).magnitude();

          if (distance > furthest) {
            start = t_point;
            end = t_point2;

            furthest = distance;
          }
        }
      }
    }

    // Build the staircase steps
    {
      u32 totalSteps = u32((start - end).magnitude() / 30.f);

      for (u32 i = 0; i < totalSteps; i++) {
        auto& step = create_object_from("stair-step");

        step.position = Vec3f::lerp(start, end, i / float(totalSteps));
        step.color = Vec3f(0.f);
        step.rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), yRotation);

        // Scale the steps to the width of the staircase platform
        if (staircase.scale.x > staircase.scale.z) {
          step.scale = Vec3f(staircase.scale.z, 3.f, 10.f);
        } else {
          step.scale = Vec3f(10.f, 3.f, staircase.scale.x);
        }

        commit(step);
      }
    }
  }
}

void World::initializeGameWorld(GmContext* context, GameState& state) {
  context->scene.zNear = 5.f;
  context->scene.zFar = 50000.f;

  add_mesh("ocean", 1, Mesh::Disc(12));
  add_mesh("ocean-floor", 1, Mesh::Disc(12));
  add_mesh("platform", 1000, Mesh::Cube());
  add_mesh("npc", 100, Mesh::Cube());
  add_mesh("sphere", 1, Mesh::Sphere(18));

  mesh("ocean")->type = MeshType::WATER;
  mesh("ocean")->canCastShadows = false;
  mesh("ocean-floor")->canCastShadows = false;
  mesh("platform")->disabled = true;

  auto& ocean = create_object_from("ocean");

  ocean.position = Vec3f(0, -2000.f, 0);
  ocean.scale = Vec3f(50000.f, 1.f, 50000.f);

  commit(ocean);

  auto& oceanFloor = create_object_from("ocean-floor");

  oceanFloor.position = Vec3f(0, -2500.f, 0);
  oceanFloor.scale = Vec3f(50000.f, 1.f, 50000.f);
  oceanFloor.color = Vec3f(0.3f, 0.7f, 0.9f);

  commit(oceanFloor);

  auto& player = create_object_from("sphere");

  player.scale = Vec3f(20.0f);
  player.position = Vec3f(-30.f, 250.f, 2220.f);
  player.color = Vec3f(1.f, 0.4f, 0.4f);

  commit(player);

  auto& light = create_light(LightType::DIRECTIONAL_SHADOWCASTER);

  light.direction = Vec3f(-0.2f, -1.f, -1.f);
  light.color = Vec3f(1.0f);
  light.serializable = false;

  // Create meshes
  {
    for (auto& asset : World::meshAssets) {
      add_mesh(asset.name, asset.maxInstances, asset.create());

      // @todo handle all mesh attributes
      mesh(asset.name)->texture = asset.attributes.texture;
      mesh(asset.name)->normalMap = asset.attributes.normalMap;
      mesh(asset.name)->emissivity = asset.attributes.emissivity;
    }

    for (auto& asset : World::dynamicMeshPieces) {
      add_mesh(asset.name, asset.maxInstances, asset.create());

      // @todo handle all mesh attributes
    }
  }

  // Load world data
  {
    loadCollisionPlanes(context, state);
    loadWorldObjects(context);
    loadLights(context);

    World::rebuildDynamicMeshes(context);
  }
}

void World::rebuildDynamicMeshes(GmContext* context) {
  // Hide dynamic mesh placeholders
  for (auto& asset : World::meshAssets) {
    if (asset.dynamic) {
      mesh(asset.name)->disabled = true;
    }
  }

  // Show dynamic mesh pieces
  for (auto& asset : World::dynamicMeshPieces) {
    mesh(asset.name)->disabled = false;
  }

  rebuildDynamicStaircases(context);
}