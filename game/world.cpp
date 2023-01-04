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
      .emissivity = 5.f
    }
  },
  {
    .name = "da-vinci",
    .defaultColor = Vec3f(1.f),
    .create = []() {
      return Mesh::Model("./game/assets/da-vinci.obj");
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

internal void loadGameWorldData(GmContext* context, GameState& state) {
  u64 start = Gm_GetMicroseconds();

  // @todo eventually store as binary data
  auto worldData = Gm_LoadFileContents("./game/data_world.txt");
  auto lines = Gm_SplitString(worldData, "\n");

  // @temporary
  objects("platform").reset();

  // @temporary
  std::vector<Platform> platforms;
  Platform platform;

  // @temporary
  u32 i = 0;

  // @temporary
  while (i < lines.size() - 1) {
    auto& line = lines[i];

    if (line[0] == '@') {
      // @todo check object name
    } else {
      auto parts = Gm_SplitString(line, ",");
      auto& platform = create_object_from("platform");

      #define df(i) stof(parts[i])
      #define di(i) stoi(parts[i])

      platform.position = Vec3f(df(0), df(1), df(2));
      platform.scale = Vec3f(df(3), df(4), df(5));
      platform.rotation = Quaternion(df(6), df(7), df(8), df(9));
      platform.color = pVec4(di(10), di(11), di(12));

      commit(platform);
    }

    i++;
  }

  // Set up collision planes
  {
    state.collisionPlanes.clear();

    for (auto& platform : objects("platform")) {
      Collisions::addObjectCollisionPlanes(platform, state.collisionPlanes);
    }
  }

  Console::log("Loaded game world data in", Gm_GetMicroseconds() - start, "us");
}

void World::initializeGameWorld(GmContext* context, GameState& state) {
  context->scene.zNear = 5.f;
  context->scene.zFar = 50000.f;

  add_mesh("ocean", 1, Mesh::Plane(2));
  add_mesh("ocean-floor", 1, Mesh::Plane(2));
  add_mesh("platform", 1000, Mesh::Cube());
  add_mesh("npc", 100, Mesh::Cube());
  add_mesh("sphere", 1, Mesh::Sphere(18));

  mesh("ocean")->type = MeshType::WATER;
  mesh("ocean")->canCastShadows = false;
  mesh("ocean-floor")->canCastShadows = false;

  auto& ocean = create_object_from("ocean");

  ocean.position = Vec3f(0, -2000.f, 0);
  ocean.scale = Vec3f(50000.f, 1.f, 50000.f);

  commit(ocean);

  auto& floor = create_object_from("ocean-floor");

  floor.position = Vec3f(0, -2500.f, 0);
  floor.scale = Vec3f(50000.f, 1.f, 50000.f);
  floor.color = Vec3f(0.3f, 0.7f, 0.9f);

  commit(floor);

  auto& player = create_object_from("sphere");

  player.scale = Vec3f(20.0f);
  player.position.y = 200.0f;
  player.color = Vec3f(1.f, 0.4f, 0.4f);

  commit(player);

  auto& light = create_light(LightType::DIRECTIONAL_SHADOWCASTER);

  light.direction = Vec3f(-0.2f, -1.f, -1.f);
  light.color = Vec3f(1.0f);

  loadGameWorldData(context, state);
}