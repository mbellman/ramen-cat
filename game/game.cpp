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

internal void initializeGameScene(GmContext* context, GameState& state) {
  addMesh("platform", 9, Mesh::Cube());
  addMesh("sphere", 1, Mesh::Sphere(18));

  for (u8 i = 0; i < 9; i++) {
    auto& platform = createObjectFrom("platform");

    platform.scale = Vec3f(50.f, 10.f, 50.f);

    platform.position.x = -200.f + 200.f * (i % 3);
    platform.position.z = -200.f + 200.f * (i / 3);
    platform.position.y -= 10.f;

    commit(platform);
  }

  auto& player = createObjectFrom("sphere");

  player.scale = 20.0f;
  player.position.y = 20.0f;
  player.color = Vec3f(1.f, 0.4f, 0.4f);

  commit(player);

  auto& light = createLight(LightType::DIRECTIONAL_SHADOWCASTER);

  light.direction = Vec3f(0.5f, -1.f, -1.f);
  light.color = Vec3f(1.0f, 0.6f, 0.2f);

  state.lastFrameY = player.position.y;
}

internal void handleMovementInput(GmContext* context, GameState& state, float dt) {
  auto& input = getInput();
  auto& player = getPlayer();

  auto rate = 5000.f * dt;
  auto initialVelocity = state.velocity;

  Vec3f forward = getCamera().orientation.getDirection().xz().unit();
  Vec3f left = getCamera().orientation.getLeftDirection().xz().unit();

  if (player.position.y > 20.f) {
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

  if (input.isKeyHeld(Key::SPACE) && player.position.y == 20.f) {
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