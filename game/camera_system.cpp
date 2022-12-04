#include "camera_system.h"
#include "macros.h"

using namespace Gamma;

internal void normalizeThirdPersonCamera(ThirdPersonCamera& camera3p) {
  constexpr static float MAX_RADIUS = 400.f;

  camera3p.radius = Gm_Clampf(camera3p.radius, 50.f, MAX_RADIUS);
  camera3p.altitude = std::powf(camera3p.radius / MAX_RADIUS, 3.f) * Gm_HALF_PI * 0.8f;
}

namespace CameraSystem {
  void initializeGameCamera(GmContext* context, GameState& state) {
    state.camera3p.radius = 100.f;
    state.camera3p.azimuth = Gm_PI + Gm_HALF_PI;

    normalizeThirdPersonCamera(state.camera3p);

    getCamera().position = getPlayer().position + state.camera3p.calculatePosition();
  }

  void handleGameCamera(GmContext* context, GameState& state, float dt) {
    normalizeThirdPersonCamera(state.camera3p);

    auto& player = getPlayer();

    getCamera().position = Vec3f::lerp(getCamera().position, player.position + state.camera3p.calculatePosition(), dt * 15.f);

    pointCameraAt(player);
  }
}