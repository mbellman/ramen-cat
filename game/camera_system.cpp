#include "camera_system.h"
#include "macros.h"

using namespace Gamma;

internal void updateThirdPersonCameraRadius(ThirdPersonCamera& camera3p) {
  camera3p.radius = 300.f + 200.f * camera3p.altitude / Gm_HALF_PI;
}

namespace CameraSystem {
  void initializeGameCamera(GmContext* context, GameState& state) {
    state.camera3p.azimuth = Gm_PI + Gm_HALF_PI;
    state.camera3p.altitude = 0.f;

    updateThirdPersonCameraRadius(state.camera3p);

    getCamera().position = getPlayer().position + state.camera3p.calculatePosition();
  }

  void handleGameCamera(GmContext* context, GameState& state, float dt) {
    updateThirdPersonCameraRadius(state.camera3p);

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