#include "world.h"
#include "macros.h"

using namespace Gamma;

// @temporary
struct Platform {
  Vec3f position;
  Vec3f scale;
  Vec3f rotation;
  Vec3f color;
};

// @temporary
internal std::vector<std::vector<Vec3f>> platformPlanePoints = {
  // Top
  { Vec3f(-1.f, 1.f, -1.f ), Vec3f(-1.f, 1.f, 1.f), Vec3f(1.f, 1.f, 1.f), Vec3f(1.f, 1.f, -1.f) },
  // Left
  { Vec3f(-1.f, -1.f, -1.f), Vec3f(-1.f, -1.f, 1.f), Vec3f(-1.f, 1.f, 1.f), Vec3f(-1.f, 1.f, -1.f) },
  // Front
  { Vec3f(-1.f, -1.f, -1.f), Vec3f(-1.f, 1.f, -1.f), Vec3f(1.f, 1.f, -1.f), Vec3f(1.f, -1.f, -1.f) }
};

internal void setupCollisionPlane(Plane& plane) {
  plane.normal = Vec3f::cross(plane.p2 - plane.p1, plane.p3 - plane.p2).unit();

  plane.t1 = Vec3f::cross(plane.normal, plane.p2 - plane.p1);
  plane.t2 = Vec3f::cross(plane.normal, plane.p3 - plane.p2);
  plane.t3 = Vec3f::cross(plane.normal, plane.p4 - plane.p3);
  plane.t4 = Vec3f::cross(plane.normal, plane.p1 - plane.p4);

  plane.nDotU = Vec3f::dot(plane.normal, Vec3f(0, 1.f, 0));
}

internal void loadGameWorldData(GmContext* context, GameState& state) {
  using namespace std;

  auto worldData = Gm_LoadFileContents("./game/world_data.txt");
  auto lines = Gm_SplitString(worldData, "\n");

  // @temporary
  objects("platform").reset();

  // @temporary
  std::vector<Platform> platforms;
  Platform platform;

  // @temporary
  u32 i = 0;

  while (i < lines.size() - 1) {
    auto& line = lines[i];

    if (line[0] == '@') {
      // @todo check object name
    } else {
      auto parts = Gm_SplitString(line, ",");
      auto& platform = createObjectFrom("platform");

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

  // @temporary
  World::rebuildCollisionPlanes(context, state);
}

void World::initializeGameWorld(GmContext* context, GameState& state) {
  context->scene.zNear = 1.f;
  context->scene.zFar = 50000.f;

  addMesh("ocean", 1, Mesh::Plane(2));
  addMesh("ocean-floor", 1, Mesh::Plane(2));
  addMesh("platform", 1000, Mesh::Cube());
  addMesh("sphere", 1, Mesh::Sphere(18));

  loadGameWorldData(context, state);

  mesh("ocean")->type = MeshType::WATER;
  mesh("ocean")->canCastShadows = false;
  mesh("ocean-floor")->canCastShadows = false;

  auto& ocean = createObjectFrom("ocean");

  ocean.position = Vec3f(0, -2000.f, 0);
  ocean.scale = Vec3f(25000.f, 1.f, 25000.f);

  commit(ocean);

  auto& floor = createObjectFrom("ocean-floor");

  floor.position = Vec3f(0, -2500.f, 0);
  floor.scale = Vec3f(25000.f, 1.f, 25000.f);
  floor.color = Vec3f(0.5f, 0.7f, 0.9f);

  commit(floor);

  auto& player = createObjectFrom("sphere");

  player.scale = Vec3f(20.0f);
  player.position.y = 200.0f;
  player.color = Vec3f(1.f, 0.4f, 0.4f);

  commit(player);

  auto& light = createLight(LightType::DIRECTIONAL_SHADOWCASTER);

  light.direction = Vec3f(-0.2f, -1.f, -1.f);
  light.color = Vec3f(1.0f);
}

void World::rebuildCollisionPlanes(GmContext* context, GameState& state) {
  state.collisionPlanes.clear();

  // @temporary
  for (auto& platform : objects("platform")) {
    auto rotation = platform.rotation.toMatrix4f();

    for (auto& points : platformPlanePoints) {
      Plane plane;

      plane.p1 = platform.position + (rotation * (platform.scale * points[0])).toVec3f();
      plane.p2 = platform.position + (rotation * (platform.scale * points[1])).toVec3f();
      plane.p3 = platform.position + (rotation * (platform.scale * points[2])).toVec3f();
      plane.p4 = platform.position + (rotation * (platform.scale * points[3])).toVec3f();

      setupCollisionPlane(plane);

      state.collisionPlanes.push_back(plane);
    }
  }
}