#include "Gamma.h"

#include "game.h"

void initializeGame(GmContext* context, GameState& state) {
  using namespace Gamma;

  // Default camera control/window focus
  auto& input = context->scene.input;
  auto& camera = context->scene.camera;

  state.camera.radius = 100.f;
  state.camera.azimuth = Gm_PI + Gm_HALF_PI;
  state.camera.altitude = state.camera.radius / 300.f * Gm_HALF_PI * 0.5f;

  input.on<MouseMoveEvent>("mousemove", [&](const MouseMoveEvent& event) {
    if (SDL_GetRelativeMouseMode()) {
      state.camera.azimuth -= event.deltaX / 1000.f;
      state.camera.radius += float(event.deltaY) / 4.f;
      state.camera.radius = Gm_Clampf(state.camera.radius, 50.f, 300.f);
      state.camera.altitude = state.camera.radius / 300.f * Gm_HALF_PI * 0.5f;
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

  // Default scene objects/lighting
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

  auto& light = createLight(LightType::SPOT_SHADOWCASTER);

  light.position = sphere.position + Vec3f(-30.0f, 30.0f, -30.0f);
  light.direction = sphere.position - light.position;
  light.color = Vec3f(1.0f, 0.9f, 0.2f);
  light.radius = 500.0f;

  camera.position = Vec3f(-100.0f, 75.0f, -150.0f);

  Gm_PointCameraAt(context, sphere);
}

void updateGame(GmContext* context, GameState& state, float dt) {
  using namespace Gamma;

  auto& input = getInput();
  auto& sphere = objects("sphere")[0];

  Vec3f forward = getCamera().orientation.getDirection().xz();
  Vec3f left = getCamera().orientation.getLeftDirection().xz();

  if (input.isKeyHeld(Key::W)) {
    sphere.position += forward * 300.f * dt;
  } else if (input.isKeyHeld(Key::S)) {
    sphere.position += forward.invert() * 300.f * dt;
  } else if (input.isKeyHeld(Key::A)) {
    sphere.position += left * 300.f * dt;
  } else if (input.isKeyHeld(Key::D)) {
    sphere.position += left.invert() * 300.f * dt;
  }

  commit(sphere);

  getCamera().position = sphere.position + state.camera.calculatePosition();

  pointCameraAt(objects("sphere")[0]);
}