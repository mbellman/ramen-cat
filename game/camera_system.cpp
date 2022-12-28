#include "camera_system.h"
#include "collision.h"
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
    START_TIMING("handleGameCamera");
  
    updateThirdPersonCameraRadius(state, dt);

    auto& input = getInput();
    auto& player = getPlayer();
    auto playerToCamera = state.camera3p.calculatePosition();
    auto targetCameraPosition = player.position + playerToCamera;

    // Control camera using mouse movements
    {
      // @todo Gm_IsWindowFocused()
      if (SDL_GetRelativeMouseMode()) {
        auto& delta = input.getMouseDelta();

        state.camera3p.azimuth -= delta.x / 1000.f;
        state.camera3p.altitude += delta.y / 1000.f;

        state.camera3p.limitAltitude(0.99f);
      }
    }

    // Reposition the camera if necessary to avoid clipping inside walls
    {
      for (auto& plane : state.collisionPlanes) {
        auto collision = getLinePlaneCollision(player.position, targetCameraPosition, plane);

        if (collision.hit) {
          auto playerToCollision = collision.point - player.position;

          targetCameraPosition = player.position + playerToCollision * 0.95f;
        }
      }
    }

    // Update the actual view camera position/orientation
    {
      if (context->scene.frame > 0) {
        getCamera().position = Vec3f::lerp(getCamera().position, targetCameraPosition, dt * 15.f);
      } else {
        getCamera().position = targetCameraPosition;
      }

      pointCameraAt(player);
    }

    LOG_TIME();
  }
}