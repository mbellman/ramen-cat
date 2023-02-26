#include "camera_system.h"
#include "collisions.h"
#include "macros.h"
#include "easing.h"
#include "game_constants.h"

using namespace Gamma;

internal void updateThirdPersonCameraRadius(GmContext* context, GameState& state, float dt) {
  if (state.cameraMode == CameraMode::FIRST_PERSON) {
    state.camera3p.radius = CAMERA_FIRST_PERSON_RADIUS;

    return;
  }

  float baseRadius =
    state.cameraMode == CameraMode::NORMAL
      ? CAMERA_NORMAL_BASE_RADIUS
      : CAMERA_ZOOM_OUT_BASE_RADIUS;

  float altitudeRadius = CAMERA_RADIUS_ALTITUDE_MULTIPLIER * state.camera3p.altitude / Gm_HALF_PI;

  state.camera3p.radius = baseRadius + altitudeRadius;

  if (state.isMovingPlayerThisFrame && state.camera3p.radius < 130.f) {
    state.camera3p.radius += 100.f * dt;
  }

  if (
    state.canPerformWallKick &&
    state.lastWallBumpTime != 0.f &&
    time_since(state.lastWallBumpTime) <= WALL_KICK_WINDOW_DURATION
  ) {
    // Zoom in to the player when winding up a potential wall kick
    float calculatedRadius = state.camera3p.radius;
    float alpha = time_since(state.lastWallBumpTime) / WALL_KICK_WINDOW_DURATION;

    state.camera3p.radius = Gm_Lerpf(calculatedRadius, calculatedRadius * 0.6f, alpha);
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
  float t = time_since(state.cameraOverrideStartTime) / state.cameraOverrideDuration;

  if (t > 1.f) {
    t = 1.f;
  }

  return Vec3f::lerp(tweenStart, tweenEnd, easeOutQuint(t));
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

  state.camera3p.azimuth = Gm_LerpCircularf(tweenStart.azimuth, tweenEnd.azimuth, easeOutQuint(t), Gm_PI);
  state.camera3p.altitude = Gm_Lerpf(tweenStart.altitude, tweenEnd.altitude, easeOutQuint(t));
  state.camera3p.radius = Gm_Lerpf(tweenStart.radius, tweenEnd.radius, easeOutQuint(t));
}

void CameraSystem::initializeGameCamera(GmContext* context, GameState& state) {
  state.camera3p.azimuth = Gm_PI - Gm_HALF_PI;
  state.camera3p.altitude = 0.1f;

  updateThirdPersonCameraRadius(context, state, 0.f);

  get_camera().position = get_player().position + state.camera3p.calculatePosition();
}

void CameraSystem::handleGameCamera(GmContext* context, GameState& state, float dt) {
  if (state.gameStartTime == 0.f) {
    return;
  }

  START_TIMING("handleGameCamera");

  updateThirdPersonCameraRadius(context, state, dt);
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

    // Disable ground particles in first-person mode
    mesh("ground-particle")->disabled = state.cameraMode == CameraMode::FIRST_PERSON;

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
        auto target = matInverseRotation.transformVec3f(collisionPlatformToTargetCamera);

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
      #if GAMMA_DEVELOPER_MODE
        // Allow camera panning to behave at full speed, irrespective of game speed
        float alpha = 15.f * (dt / state.gameSpeed);
      #else
        float alpha = 15.f * dt;
      #endif

      camera.position = Vec3f::lerp(camera.position, targetCameraPosition, alpha);
    } else {
      camera.position = targetCameraPosition;
    }

    // @temporary
    float titleTransitionDuration = 3.f;

    if (time_since(state.gameStartTime) > titleTransitionDuration) {
      float alpha = state.useCameraOverride ? 1.f : Gm_Clampf(state.cameraLookAtSpeedFactor * dt);

      smoothly_point_camera_at(lookAtPosition, alpha);
    } else {
      // @temporary
      float alpha = easeInOutQuart(time_since(state.gameStartTime) / titleTransitionDuration);

      camera.position = Vec3f::lerp(CAMERA_TITLE_SCREEN_POSITION, targetCameraPosition, alpha);
      camera.orientation.yaw = Gm_Lerpf(Gm_PI * 0.6f, Gm_PI, alpha);
      camera.orientation.pitch = Gm_Lerpf(0.f, 0.1f, alpha);
      camera.rotation = camera.orientation.toQuaternion();
    }
  }

  // Adjust the FOV when moving at higher speeds
  {
    auto& camera = get_camera();
    float speedOverFovLimit = state.velocity.magnitude() - CAMERA_FOV_MINIMUM_SPEED;
    if (speedOverFovLimit < 0.f) speedOverFovLimit = 0.f;

    float fovSpeedFactor = speedOverFovLimit / (speedOverFovLimit + CAMERA_FOV_SPEED_INTERVAL);
    float targetFov = CAMERA_BASE_FOV + CAMERA_FOV_VELOCITY_MULTIPLIER * fovSpeedFactor;
    float alpha = Gm_Clampf(10.f * dt, 0.f, 1.f);

    if (state.dashLevel == 1) targetFov *= 1.1f;
    if (state.dashLevel == 2) targetFov *= 1.2f;

    camera.fov = Gm_Lerpf(camera.fov, targetFov, alpha);
  }

  LOG_TIME();
}

void CameraSystem::handleVisibilityCullingAndLevelsOfDetail(GmContext* context, GameState& state) {
  START_TIMING("handleVisibilityCullingAndLevelsOfDetail");

  use_frustum_culling({ "weeds", "lamp", "ladder", "ac-unit", "ac-fan" });

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