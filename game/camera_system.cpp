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

void CameraSystem::initializeGameCamera(GmContext* context, GameState& state) {
  state.camera3p.azimuth = Gm_PI + Gm_HALF_PI;
  state.camera3p.altitude = 0.f;

  updateThirdPersonCameraRadius(state, 0.f);

  get_camera().position = get_player().position + state.camera3p.calculatePosition();
}

void CameraSystem::handleGameCamera(GmContext* context, GameState& state, float dt) {
  START_TIMING("handleGameCamera");

  updateThirdPersonCameraRadius(state, dt);

  auto& input = get_input();
  auto& player = get_player();
  auto lookAtPosition = CameraSystem::getLookAtTargetPosition(context, state);
  auto targetCameraPosition = lookAtPosition + state.camera3p.calculatePosition();

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
      auto collision = getLinePlaneCollision(lookAtPosition, targetCameraPosition, plane);

      if (collision.hit) {
        auto playerToCollision = collision.point - lookAtPosition;

        targetCameraPosition = lookAtPosition + playerToCollision * 0.95f;
      }
    }
  }

  // Update the actual view camera position/orientation
  {
    auto& camera = get_camera();

    if (context->scene.frame > 0) {
      camera.position = Vec3f::lerp(camera.position, targetCameraPosition, dt * 15.f);
    } else {
      camera.position = targetCameraPosition;
    }

    point_camera_at(lookAtPosition);
  }

  LOG_TIME();
}

Vec3f CameraSystem::getLookAtTargetPosition(GmContext* context, GameState& state) {
  auto& player = get_player();

  if (state.cameraOverrideStartTime == 0.f) {
    // If we haven't started a camera transition yet,
    // always use the player position.
    return player.position;
  }

  Vec3f tweenStart = state.sourceCameraState.lookAtTarget;
  Vec3f tweenEnd = state.useCameraOverride ? state.targetCameraState.lookAtTarget : player.position;
  float t = get_running_time() - state.cameraOverrideStartTime;

  if (t > 1.f) {
    t = 1.f;
  }

  return Vec3f::lerp(tweenStart, tweenEnd, t);
}