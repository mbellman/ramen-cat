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
  Vec3f color;
};

// @temporary
internal std::vector<Platform> platforms = {
  {
    .position = Vec3f(0, 0, 500.f),
    .scale = Vec3f(250.f, 50.f, 1500.f),
    .color = Vec3f(0.2f, 0.3f, 1.f)
  },
  {
    .position = Vec3f(-250.f, 0, 500.f),
    .scale = Vec3f(500.f, 50.f, 250.f),
    .color = Vec3f(0.2f, 0.3f, 1.f)
  },
  {
    .position = Vec3f(225.f, 500.f, 500.f),
    .scale = Vec3f(50.f, 1000.f, 1500.f),
    .color = Vec3f(1.f, 0.5, 0.2f)
  }
};

// @temporary
internal std::vector<std::vector<Vec3f>> platformPlanePoints = {
  // Top
  { Vec3f(-1.f, 1.f, -1.f ), Vec3f(-1.f, 1.f, 1.f), Vec3f(1.f, 1.f, 1.f), Vec3f(1.f, 1.f, -1.f) },
  // Left
  { Vec3f(-1.f, -1.f, -1.f), Vec3f(-1.f, -1.f, 1.f), Vec3f(-1.f, 1.f, 1.f), Vec3f(-1.f, 1.f, -1.f) }
};

internal void setupCollisionPlane(Plane& plane) {
  plane.normal = Vec3f::cross(plane.p2 - plane.p1, plane.p3 - plane.p2).unit();

  plane.t1 = Vec3f::cross(plane.normal, plane.p2 - plane.p1);
  plane.t2 = Vec3f::cross(plane.normal, plane.p3 - plane.p2);
  plane.t3 = Vec3f::cross(plane.normal, plane.p4 - plane.p3);
  plane.t4 = Vec3f::cross(plane.normal, plane.p1 - plane.p4);

  plane.nDotU = Vec3f::dot(plane.normal, Vec3f(0, 1.f, 0));
}

internal void initializeGameScene(GmContext* context, GameState& state) {
  addMesh("platform", (u16)platforms.size(), Mesh::Cube());
  addMesh("sphere", 1, Mesh::Sphere(18));

  for (auto& [ position, scale, color ] : platforms) {
    auto& platform = createObjectFrom("platform");

    platform.position = position;
    platform.position.y -= 10.f;
    platform.scale = scale;
    platform.color = color;

    commit(platform);

    // @temporary
    auto rotation = Matrix4f::rotation(platform.rotation);

    // @temporary
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

  auto& player = createObjectFrom("sphere");

  player.scale = Vec3f(20.0f);
  player.position.y = 200.0f;
  player.color = Vec3f(1.f, 0.4f, 0.4f);

  commit(player);

  auto& light = createLight(LightType::DIRECTIONAL_SHADOWCASTER);

  light.direction = Vec3f(0.5f, -1.f, -1.f);
  light.color = Vec3f(1.0f, 0.7f, 0.5f);

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

  // Handle input
  {
    MovementSystem::handlePlayerMovementInput(context, state, dt);

    if (state.isPlayerMovingThisFrame && state.camera3p.radius < 130.f) {
      state.camera3p.radius += 100.f * dt;
    }
  }

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