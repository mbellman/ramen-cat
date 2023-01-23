#include "camera_system.h"
#include "collisions.h"
#include "macros.h"
#include "easing.h"

using namespace Gamma;

internal void updateThirdPersonCameraRadius(GameState& state, float dt) {
  if (state.cameraMode == CameraMode::FIRST_PERSON) {
    state.camera3p.radius = 5.f;

    return;
  }

  float baseRadius = state.cameraMode == CameraMode::NORMAL ? 300.f : 600.f;
  float altitudeRadius = 200.f * state.camera3p.altitude / Gm_HALF_PI;

  state.camera3p.radius = baseRadius + altitudeRadius;

  if (state.isMovingPlayerThisFrame && state.camera3p.radius < 130.f) {
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

internal void handleCameraOverride(GmContext* context, GameState& state) {
  float t = time_since(state.cameraOverrideStartTime) / state.cameraOverrideDuration;

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
  state.camera3p.azimuth = Gm_PI - Gm_HALF_PI;
  state.camera3p.altitude = 0.1f;

  updateThirdPersonCameraRadius(state, 0.f);

  get_camera().position = get_player().position + state.camera3p.calculatePosition();
}

void CameraSystem::handleGameCamera(GmContext* context, GameState& state, float dt) {
  START_TIMING("handleGameCamera");

  updateThirdPersonCameraRadius(state, dt);
  handleCameraOverride(context, state);

  auto& input = get_input();
  auto& player = get_player();
  auto lookAtPosition = getLookAtTargetPosition(context, state);
  auto targetCameraPosition = lookAtPosition + state.camera3p.calculatePosition();

  // Handle camera control inputs
  {
    if (Gm_IsWindowFocused() && !state.useCameraOverride) {
      auto& input = get_input();

      // Handle mouse movement
      {
        auto& delta = input.getMouseDelta();

        state.camera3p.azimuth -= delta.x / 1000.f;
        state.camera3p.altitude += delta.y / 1000.f;

        state.camera3p.limitAltitude(0.95f);
      }

      // Handle view mode changes
      {
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
            } else if (state.isOnSolidGround && !state.isMovingPlayerThisFrame) {
              state.cameraMode = CameraMode::FIRST_PERSON;
            }
          }
        } else if (state.cameraMode == CameraMode::FIRST_PERSON && state.isMovingPlayerThisFrame) {
          // Exit first-person mode whenever moving the player
          state.cameraMode = CameraMode::NORMAL;
        }
      }

      // Handle centering the camera behind the player
      {
        float targetAzimuth = state.camera3p.azimuth;

        if (state.direction.x != 0.f && state.direction.z != 0.f) {
          targetAzimuth = atan2f(state.direction.z, state.direction.x) + Gm_PI;
        }

        // @todo restore this when adding gamepad/analog controls
        // if (state.isMovingPlayerThisFrame) {
        //   state.camera3p.azimuth = Gm_LerpCircularf(state.camera3p.azimuth, targetAzimuth, 2.f * dt, Gm_PI);
        // }

        if (input.didPressKey(Key::SHIFT)) {
          state.camera3p.azimuth = targetAzimuth;

          if (state.camera3p.altitude > 0.3f) {
            state.camera3p.altitude = 0.3f;
          }
        }
      }
    }

    // Disable movement particles in first-person mode
    mesh("player-particle")->disabled = state.cameraMode == CameraMode::FIRST_PERSON;

    // Wrap the azimuth to [0, Gm_TAU]
    state.camera3p.azimuth = Gm_Modf(state.camera3p.azimuth, Gm_TAU);
  }

  // Reposition the camera if necessary to avoid clipping inside walls
  {
    auto& camera = get_camera();
    bool didRepositionCamera = false;

    for (auto& plane : state.collisionPlanes) {
      auto collision = Collisions::getLinePlaneCollision(lookAtPosition, targetCameraPosition, plane);
      auto cDotN = Vec3f::dot(targetCameraPosition - collision.point, plane.normal);

      if (collision.hit && cDotN < 0.f) {
        auto& collisionPlatform = *get_object_by_record(collision.plane.sourceObjectRecord);
        auto& scale = collisionPlatform.scale;
        auto matInverseRotation = collisionPlatform.rotation.toMatrix4f().inverse();
        auto collisionPlatformToTargetCamera = targetCameraPosition - collisionPlatform.position;
        auto target = (matInverseRotation * collisionPlatformToTargetCamera).toVec3f();

        if (
          // Check to see that the camera isn't currently being repositioned
          // before we do our inside-collision-platform check. If the camera
          // is already in a repositioned state, continue to reposition it.
          !state.isRepositioningCamera && (
          // Determine whether the target camera position would be outside
          // of the actual collision platform bounding box, and skip if so.
          // This way we can pan the camera 'around' collision platforms,
          // allowing for more freeform motion.
          target.x < -scale.x || target.x > scale.x ||
          target.y < -scale.y || target.y > scale.y ||
          target.z < -scale.z || target.z > scale.z
        )) {
          continue;
        }

        auto playerToCollision = collision.point - lookAtPosition;

        targetCameraPosition = lookAtPosition + playerToCollision * 0.9f;
        didRepositionCamera = true;
      }
    }

    state.isRepositioningCamera = didRepositionCamera;
  }

  // Update the actual view camera position/orientation
  {
    auto& camera = get_camera();

    if (context->scene.frame > 0) {
      camera.position = Vec3f::lerp(camera.position, targetCameraPosition, 15.f * dt);
    } else {
      camera.position = targetCameraPosition;
    }

    point_camera_at(lookAtPosition);
  }

  // Adjust the FOV when moving at higher speeds
  {
    const float BASE_FOV = 45.f;
    const float FOV_ADJUSTMENT_FACTOR = 20.f;

    auto& camera = get_camera();
    auto playerSpeed = state.velocity.magnitude() - 800.f;
    if (playerSpeed < 0.f) playerSpeed = 0.f;

    float targetFov = BASE_FOV + FOV_ADJUSTMENT_FACTOR * playerSpeed / (playerSpeed + 500.f);
    float alpha = Gm_Clampf(10.f * dt, 0.f, 1.f);

    camera.fov = Gm_Lerpf(camera.fov, targetFov, alpha);
  }

  use_frustum_culling({ "weeds", "lamp" });

  LOG_TIME();
}

void CameraSystem::setTargetCameraState(GmContext* context, GameState& state, const CameraState& cameraState) {
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