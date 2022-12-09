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

internal std::vector<Platform> platforms = {
  {
    Vec3f(0.f),
    Vec3f(100.f, 50.f, 200.f),
    Vec3f(0.2f, 0.3f, 1.f)
  }
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
  }

  auto& player = createObjectFrom("sphere");

  player.scale = Vec3f(20.0f);
  player.position.y = 20.0f;
  player.color = Vec3f(1.f, 0.4f, 0.4f);

  commit(player);

  auto& light = createLight(LightType::DIRECTIONAL_SHADOWCASTER);

  light.direction = Vec3f(0.5f, -1.f, -1.f);
  light.color = Vec3f(1.0f, 0.7f, 0.5f);

  state.previousPlayerPosition = player.position;
}

internal void handleMovementInput(GmContext* context, GameState& state, float dt) {
  auto& input = getInput();
  auto& player = getPlayer();

  auto rate = 5000.f * dt;
  auto initialVelocity = state.velocity;

  Vec3f forward = getCamera().orientation.getDirection().xz().unit();
  Vec3f left = getCamera().orientation.getLeftDirection().xz().unit();

  if (state.velocity.y != 0.f) {
    // Reduce movement rate in midair
    rate *= 0.05f;
  }

  if (input.isKeyHeld(Key::W)) {
    state.velocity += forward * rate;
  }
  
  if (input.isKeyHeld(Key::S)) {
    state.velocity += forward.invert() * rate;
  }
  
  if (input.isKeyHeld(Key::A)) {
    state.velocity += left * rate;
  }
  
  if (input.isKeyHeld(Key::D)) {
    state.velocity += left.invert() * rate;
  }

  auto moving = state.velocity != initialVelocity;

  if (input.isKeyHeld(Key::SPACE) && state.velocity.y == 0.f) {
    state.velocity.y = 500.f;
  }
}

internal void handleInput(GmContext* context, GameState& state, float dt) {
  auto initialVelocity = state.velocity;

  handleMovementInput(context, state, dt);

  auto moving = initialVelocity != state.velocity;

  if (moving && state.camera3p.radius < 130.f) {
    state.camera3p.radius += 100.f * dt;
  }
}

void initializeGame(GmContext* context, GameState& state) {
  Gm_EnableFlags(GammaFlags::VSYNC);

  initializeInputHandlers(context, state);
  initializeGameScene(context, state);

  CameraSystem::initializeGameCamera(context, state);
}

void updateGame(GmContext* context, GameState& state, float dt) {
  handleInput(context, state, dt);

  MovementSystem::handlePlayerMovement(context, state, dt);
  CameraSystem::handleGameCamera(context, state, dt);
}