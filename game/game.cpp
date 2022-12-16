#include <string>

#include "Gamma.h"

#include "game.h"
#include "movement_system.h"
#include "camera_system.h"

#define internal static inline
#define getPlayer() objects("sphere")[0]

using namespace Gamma;

internal void initializeInputHandlers(GmContext* context, GameState& state) {
  auto& input = getInput();
  auto& camera = getCamera();

  input.on<MouseMoveEvent>("mousemove", [&](const MouseMoveEvent& event) {
    if (SDL_GetRelativeMouseMode()) {
      state.camera3p.azimuth -= event.deltaX / 1000.f;
      state.camera3p.altitude += event.deltaY / 1000.f;

      state.camera3p.limitAltitude(0.99f);
    }
  });

  input.on<MouseButtonEvent>("mousedown", [&](const MouseButtonEvent& event) {
    if (!SDL_GetRelativeMouseMode()) {
      SDL_SetRelativeMouseMode(SDL_TRUE);
    }
  });

  input.on<Key>("keyup", [&](Key key) {
    if (key == Key::ESCAPE) {
      SDL_SetRelativeMouseMode(SDL_FALSE);
    }

    if (key == Key::V) {
      if (Gm_IsFlagEnabled(GammaFlags::VSYNC)) {
        Gm_DisableFlags(GammaFlags::VSYNC);
      } else {
        Gm_EnableFlags(GammaFlags::VSYNC);
      }
    }
  });

  input.on<Key>("keystart", [&state, context](Key key) {
    if (key == Key::SPACE) {
      state.lastJumpInputTime = getRunningTime();
    }
  });
}

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

internal void loadWorldData(GmContext* context, GameState& state) {
  auto worldData = Gm_LoadFileContents("./game/world_data.txt");
  auto lines = Gm_SplitString(worldData, "\n");

  // @temporary
  objects("platform").reset();
  state.collisionPlanes.clear();

  // @temporary
  std::vector<Platform> platforms;
  Platform platform;

  // @temporary
  for (auto& line : lines) {
    if (line.size() == 0 || line[0] == '\n') {
      platforms.push_back(platform);

      platform.position = Vec3f(0.f);
      platform.scale = Vec3f(0.f);
      platform.rotation = Vec3f(0.f);
      platform.color = Vec3f(0.f);

      continue;
    }

    auto parts = Gm_SplitString(line, ":");
    auto label = parts[0];
    auto values = Gm_SplitString(parts[1], ",");

    Vec3f value = {
      std::stof(values[0]),
      std::stof(values[1]),
      std::stof(values[2])
    };

    if (label == "position") {
      platform.position = value;
    } else if (label == "scale") {
      platform.scale = value;
    } else if (label == "rotation") {
      platform.rotation = value;
    } else if (label == "color") {
      platform.color = value;
    }
  }

  // @temporary
  for (auto& [ position, scale, rotation, color ] : platforms) {
    auto& platform = createObjectFrom("platform");

    platform.position = position;
    platform.scale = scale;
    platform.rotation = rotation;
    platform.color = color;

    commit(platform);

    auto rotation = Matrix4f::rotation(platform.rotation);

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

internal void initializeGameScene(GmContext* context, GameState& state) {
  addMesh("ocean", 1, Mesh::Plane(2));
  addMesh("platform", 1000, Mesh::Cube());
  addMesh("sphere", 1, Mesh::Sphere(18));

  loadWorldData(context, state);

  Gm_WatchFile("./game/world_data.txt", [context, &state]() {
    u64 startMicroseconds = Gm_GetMicroseconds();

    loadWorldData(context, state);

    float totalMilliseconds = (Gm_GetMicroseconds() - startMicroseconds) / 1000.f;

    Console::log("Hot-reloaded world data in", totalMilliseconds, "ms");
  });

  mesh("ocean")->type = MeshType::WATER;

  auto& ocean = createObjectFrom("ocean");

  ocean.position = Vec3f(0, -2000.f, 0);
  ocean.scale = Vec3f(10000.f, 1.f, 10000.f);

  commit(ocean);

  auto& player = createObjectFrom("sphere");

  player.scale = Vec3f(20.0f);
  player.position.y = 200.0f;
  player.color = Vec3f(1.f, 0.4f, 0.4f);

  commit(player);

  auto& light = createLight(LightType::DIRECTIONAL_SHADOWCASTER);

  light.direction = Vec3f(0.5f, -1.f, -1.f);
  light.color = Vec3f(1.0f);

  state.previousPlayerPosition = player.position;
}

void initializeGame(GmContext* context, GameState& state) {
  Gm_EnableFlags(GammaFlags::VSYNC);

  initializeInputHandlers(context, state);
  initializeGameScene(context, state);

  CameraSystem::initializeGameCamera(context, state);
}

void updateGame(GmContext* context, GameState& state, float dt) {
  auto& player = getPlayer();

  // Track start-of-frame variables
  {
    state.frameStartTime = getRunningTime();
    state.isPlayerMovingThisFrame = false;
  }

  MovementSystem::handlePlayerMovementInput(context, state, dt);
  MovementSystem::handlePlayerMovementPhysics(context, state, dt);
  CameraSystem::handleGameCamera(context, state, dt);

  // Reset the player position after falling longer than 2 seconds
  {
    if (
      state.frameStartTime - state.lastTimeOnSolidGround > 2.f &&
      state.lastSolidGroundPosition.y - player.position.y > 1000.f
    ) {
      state.velocity = Vec3f(0.f);
      state.lastTimeOnSolidGround = state.frameStartTime;

      player.position = state.lastSolidGroundPosition;
    }
  }

  // Track end-of-frame variables
  {
    if (state.isOnSolidGround && state.velocity.xz().magnitude() > 1.f) {
      // If we were on solid ground, but any movement
      // occurs along the xz plane, all bets are off!
      state.isOnSolidGround = false;
    }

    state.previousPlayerPosition = player.position;
  }
}