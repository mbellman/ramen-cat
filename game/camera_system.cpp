#include "camera_system.h"
#include "collision.h"
#include "macros.h"

using namespace Gamma;

internal float easeOut(float t) {
  return 1.f - powf(1.f - t, 5);
}

internal void updateThirdPersonCameraRadius(GameState& state, float dt) {
  state.camera3p.radius = 300.f + 200.f * state.camera3p.altitude / Gm_HALF_PI;

  if (state.isPlayerMovingThisFrame && state.camera3p.radius < 130.f) {
    state.camera3p.radius += 100.f * dt;
  }
}

internal void handleThirdPersonCameraOverrides(GmContext* context, GameState& state) {
  float t = (get_running_time() - state.cameraOverrideStartTime) / state.cameraOverrideDuration;

  if (state.cameraOverrideStartTime == 0.f) {
    // If we haven't started a camera transition yet,
    // don't do anything.
    return;
  }

  if (t > 1.f) {
    if (!state.useCameraOverride) {
      return;
    }

    t = 1.f;
  }

  auto tweenStart = state.sourceCameraState.camera3p;
  auto tweenEnd = state.targetCameraState.camera3p;

  state.camera3p.azimuth = Gm_LerpCircularf(tweenStart.azimuth, tweenEnd.azimuth, easeOut(t), Gm_PI);
  state.camera3p.altitude = Gm_Lerpf(tweenStart.altitude, tweenEnd.altitude, easeOut(t));
  state.camera3p.radius = Gm_Lerpf(tweenStart.radius, tweenEnd.radius, easeOut(t));
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
  handleThirdPersonCameraOverrides(context, state);

  auto& input = get_input();
  auto& player = get_player();
  auto lookAtPosition = CameraSystem::getLookAtTargetPosition(context, state);
  auto targetCameraPosition = lookAtPosition + state.camera3p.calculatePosition();

  // Control camera using mouse movements
  {
    // @todo Gm_IsWindowFocused()
    if (SDL_GetRelativeMouseMode() && !state.useCameraOverride) {
      auto& delta = input.getMouseDelta();

      state.camera3p.azimuth -= delta.x / 1000.f;
      state.camera3p.altitude += delta.y / 1000.f;

      state.camera3p.limitAltitude(0.99f);
    }

    state.camera3p.azimuth = Gm_Modf(state.camera3p.azimuth, Gm_TAU);
  }

  // Reposition the camera if necessary to avoid clipping inside walls
  {
    for (auto& plane : state.collisionPlanes) {
      auto collision = getLinePlaneCollision(lookAtPosition, targetCameraPosition, plane);

      // @todo if dot(camera - hit, plane.normal) > 0, don't do this
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
  float t = (get_running_time() - state.cameraOverrideStartTime) / state.cameraOverrideDuration;

  if (t > 1.f) {
    t = 1.f;
  }

  return Vec3f::lerp(tweenStart, tweenEnd, easeOut(t));
}