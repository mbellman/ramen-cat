#include "Gamma.h"

#include "game.h"

#define internal static inline

using namespace Gamma;

internal void initializeInputHandlers(GmContext* context, GameState& state) {
  auto& input = context->scene.input;
  auto& camera = context->scene.camera;

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
  addMesh("plane", 1, Mesh::Plane(5));
  addMesh("sphere", 1, Mesh::Sphere(18));

  auto& plane = createObjectFrom("plane");
  auto& sphere = createObjectFrom("sphere");

  plane.scale = 1000.0f;

  sphere.scale = 20.0f;
  sphere.position.y = 20.0f;
  sphere.color = Vec3f(1.f, 0.4f, 0.4f);

  commit(plane);
  commit(sphere);

  auto& light = createLight(LightType::DIRECTIONAL_SHADOWCASTER);

  light.direction = Vec3f(0.5f, -1.f, -1.f);
  light.color = Vec3f(1.0f, 0.6f, 0.2f);
}

internal void normalizeThirdPersonCamera(ThirdPersonCamera& camera3p) {
  constexpr static float MAX_RADIUS = 400.f;

  camera3p.radius = Gm_Clampf(camera3p.radius, 50.f, MAX_RADIUS);
  camera3p.altitude = std::powf(camera3p.radius / MAX_RADIUS, 3.f) * Gm_HALF_PI * 0.8f;
}

internal void initializeCamera(GmContext* context, GameState& state) {
  state.camera3p.radius = 100.f;
  state.camera3p.azimuth = Gm_PI + Gm_HALF_PI;

  normalizeThirdPersonCamera(state.camera3p);

  auto& player = objects("sphere")[0];

  getCamera().position = player.position + state.camera3p.calculatePosition();
}

internal void handleInput(GmContext* context, GameState& state, float dt) {
  auto& input = getInput();
  auto& player = objects("sphere")[0];

  Vec3f forward = getCamera().orientation.getDirection().xz();
  Vec3f left = getCamera().orientation.getLeftDirection().xz();

  auto moving = true;

  if (input.isKeyHeld(Key::W)) {
    player.position += forward * 300.f * dt;
  } else if (input.isKeyHeld(Key::S)) {
    player.position += forward.invert() * 300.f * dt;
  } else if (input.isKeyHeld(Key::A)) {
    player.position += left * 300.f * dt;
  } else if (input.isKeyHeld(Key::D)) {
    player.position += left.invert() * 300.f * dt;
  } else {
    moving = false;
  }

  if (input.isKeyHeld(Key::SPACE) && state.velocity.y == 0.f) {
    state.velocity.y = 500.f;
  }

  if (moving && state.camera3p.radius < 130.f) {
    state.camera3p.radius += 100.f * dt;
  }
}

internal void handlePlayerMovement(GmContext* context, GameState& state, float dt) {
  state.velocity.y -= 750.f * dt;

  auto& player = objects("sphere")[0];

  player.position += state.velocity * dt;

  if (player.position.y < 20.f) {
    player.position.y = 20.f;
    state.velocity.y = 0.f;
  }

  commit(player);
}

internal void handlePlayerCamera(GmContext* context, GameState& state, float dt) {
  normalizeThirdPersonCamera(state.camera3p);

  auto& player = objects("sphere")[0];

  getCamera().position = Vec3f::lerp(getCamera().position, player.position + state.camera3p.calculatePosition(), dt * 15.f);

  pointCameraAt(player);
}

void initializeGame(GmContext* context, GameState& state) {
  Gm_EnableFlags(GammaFlags::VSYNC);

  initializeInputHandlers(context, state);
  initializeGameScene(context, state);
  initializeCamera(context, state);
}

void updateGame(GmContext* context, GameState& state, float dt) {
  handleInput(context, state, dt);
  handlePlayerMovement(context, state, dt);
  handlePlayerCamera(context, state, dt);
}