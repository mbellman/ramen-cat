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
      state.camera3p.radius += float(event.deltaY) / 4.f;
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
    Vec3f(-100.f, 0, 0),
    Vec3f(250.f, 50.f, 200.f),
    Vec3f(0.2f, 0.3f, 1.f)
  },
  {
    Vec3f(150.f, 0.f, 300.f),
    Vec3f(50.f, 300.f, 1000.f),
    Vec3f(1.f, 0.5, 0.2f)
  },
  {
    Vec3f(-50.f, 0.f, 1500.f),
    Vec3f(200.f, 50.f, 250.f),
    Vec3f(0.5f, 1.f, 0.5f)
  }
};

// @temporary
internal std::vector<std::vector<Vec3f>> platformPlanePoints = {
  // Top
  { Vec3f(-1.f, 1.f, -1.f ), Vec3f(1.f, 1.f, -1.f), Vec3f(-1.f, 1.f, 1.f), Vec3f(1.f, 1.f, 1.f) },
  // Left
  { Vec3f(-1.f, -1.f, -1.f), Vec3f(-1.f, 1.f, -1.f), Vec3f(-1.f, -1.f, 1.f), Vec3f(-1.f, 1.f, 1.f) }
};

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

    for (auto& points : platformPlanePoints) {
      Plane plane;

      plane.p1 = platform.position + platform.scale * points[0];
      plane.p2 = platform.position + platform.scale * points[1];
      plane.p3 = platform.position + platform.scale * points[2];
      plane.p4 = platform.position + platform.scale * points[3];

      plane.normal = Vec3f::cross(plane.p3 - plane.p1, plane.p2 - plane.p1).unit();

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

internal void beginFrame(GameState& state) {
  state.frameStartTime = Gm_GetMicroseconds();
  state.isPlayerMovingThisFrame = false;
}

internal void handleInput(GmContext* context, GameState& state, float dt) {
  MovementSystem::handlePlayerMovementInput(context, state, dt);

  if (state.isPlayerMovingThisFrame && state.camera3p.radius < 130.f) {
    state.camera3p.radius += 100.f * dt;
  }
}

internal void checkAndResetPositionOnFall(GmContext* context, GameState& state) {
  auto& player = getPlayer();

  if (
    state.frameStartTime - state.lastTimeOnSolidGround > 2000000 &&
    state.lastSolidGroundPosition.y - player.position.y > 1000.f
  ) {
    state.velocity = Vec3f(0.f);
    state.lastTimeOnSolidGround = state.frameStartTime;

    player.position = state.lastSolidGroundPosition;
  }
}

void initializeGame(GmContext* context, GameState& state) {
  Gm_EnableFlags(GammaFlags::VSYNC);

  initializeInputHandlers(context, state);
  initializeGameScene(context, state);

  CameraSystem::initializeGameCamera(context, state);

  state.lastTimeOnSolidGround = Gm_GetMicroseconds();
  state.lastWallBumpTime = Gm_GetMicroseconds();
}

void updateGame(GmContext* context, GameState& state, float dt) {
  beginFrame(state);
  handleInput(context, state, dt);

  MovementSystem::handlePlayerMovementPhysics(context, state, dt);
  CameraSystem::handleGameCamera(context, state, dt);

  checkAndResetPositionOnFall(context, state);
}