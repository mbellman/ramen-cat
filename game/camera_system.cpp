#include "camera_system.h"
#include "macros.h"

using namespace Gamma;

internal void updateThirdPersonCameraRadius(GameState& state, float dt) {
  state.camera3p.radius = 300.f + 200.f * state.camera3p.altitude / Gm_HALF_PI;

  if (state.isPlayerMovingThisFrame && state.camera3p.radius < 130.f) {
    state.camera3p.radius += 100.f * dt;
  }
}

namespace CameraSystem {
  void initializeGameCamera(GmContext* context, GameState& state) {
    state.camera3p.azimuth = Gm_PI + Gm_HALF_PI;
    state.camera3p.altitude = 0.f;

    updateThirdPersonCameraRadius(state, 0.f);

    getCamera().position = getPlayer().position + state.camera3p.calculatePosition();
  }

  void handleGameCamera(GmContext* context, GameState& state, float dt) {
    updateThirdPersonCameraRadius(state, dt);

    auto& player = getPlayer();
    auto targetCameraPosition = player.position + state.camera3p.calculatePosition();

    if (context->scene.frame > 0) {
      getCamera().position = Vec3f::lerp(getCamera().position, targetCameraPosition, dt * 15.f);
    } else {
      getCamera().position = targetCameraPosition;
    }

    pointCameraAt(player);
  }
}