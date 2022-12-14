#include "camera_system.h"
#include "collisions.h"
#include "macros.h"

using namespace Gamma;

internal float easeOut(float t) {
  return 1.f - powf(1.f - t, 5);
}

internal void updateThirdPersonCameraRadius(GameState& state, float dt) {
  if (state.cameraMode == CameraMode::FIRST_PERSON) {
    state.camera3p.radius = 5.f;

    return;
  }

  float baseRadius = state.cameraMode == CameraMode::NORMAL ? 300.f : 600.f;

  state.camera3p.radius = baseRadius + 200.f * state.camera3p.altitude / Gm_HALF_PI;

  if (state.isPlayerMovingThisFrame && state.camera3p.radius < 130.f) {
    state.camera3p.radius += 100.f * dt;
  }
}

internal Vec3f getLookAtTargetPosition(GmContext* context, GameState& state) {
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
  auto lookAtPosition = getLookAtTargetPosition(context, state);
  auto targetCameraPosition = lookAtPosition + state.camera3p.calculatePosition();

  // Handle camera control inputs
  {
    if (Gm_IsWindowFocused() && !state.useCameraOverride) {
      auto& delta = input.getMouseDelta();

      state.camera3p.azimuth -= delta.x / 1000.f;
      state.camera3p.altitude += delta.y / 1000.f;

      state.camera3p.limitAltitude(0.95f);
    }

    // Handle camera mode changes
    if (input.didMoveMouseWheel()) {
      if (input.getMouseWheelDirection() == MouseWheelEvent::DOWN) {
        // Zoom further out
        if (state.cameraMode == CameraMode::FIRST_PERSON) {
          state.cameraMode = CameraMode::NORMAL;
        } else {
          state.cameraMode = CameraMode::ZOOM_OUT;
        }
      } else {
        // Zoom further in
        if (state.cameraMode == CameraMode::ZOOM_OUT) {
          state.cameraMode = CameraMode::NORMAL;
        } else if (state.isOnSolidGround && !state.isPlayerMovingThisFrame) {
          state.cameraMode = CameraMode::FIRST_PERSON;
        }
      }
    } else if (state.cameraMode == CameraMode::FIRST_PERSON && state.isPlayerMovingThisFrame) {
      // Exit first-person mode whenever moving the player
      state.cameraMode = CameraMode::NORMAL;
    }

    // Disable movement particles in first-person mode
    mesh("player-particle")->disabled = state.cameraMode == CameraMode::FIRST_PERSON;

    // Wrap the azimuth to [0, Gm_TAU]
    state.camera3p.azimuth = Gm_Modf(state.camera3p.azimuth, Gm_TAU);
  }

  // Reposition the camera if necessary to avoid clipping inside walls
  {
    auto& camera = get_camera();

    for (auto& plane : state.collisionPlanes) {
      auto collision = Collisions::getLinePlaneCollision(lookAtPosition, targetCameraPosition, plane);
      auto cDotN = Vec3f::dot(targetCameraPosition - collision.point, plane.normal);

      if (collision.hit && cDotN < 0.f) {
        auto playerToCollision = collision.point - lookAtPosition;

        targetCameraPosition = lookAtPosition + playerToCollision * 0.9f;
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

void CameraSystem::setCameraStateOverride(GmContext* context, GameState& state, const CameraState& cameraState) {
  state.originalCameraState.camera3p = state.camera3p;
  state.originalCameraState.lookAtTarget = getLookAtTargetPosition(context, state);

  state.sourceCameraState = state.originalCameraState;
  state.targetCameraState = cameraState;

  state.useCameraOverride = true;
  state.cameraOverrideStartTime = state.frameStartTime;
}

void CameraSystem::restoreOriginalCameraState(GmContext* context, GameState& state) {
  state.sourceCameraState.camera3p = state.camera3p;
  state.sourceCameraState.lookAtTarget = getLookAtTargetPosition(context, state);
  
  state.targetCameraState = state.originalCameraState;

  state.cameraOverrideStartTime = state.frameStartTime;
  state.useCameraOverride = false;
}