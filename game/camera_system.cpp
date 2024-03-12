#include "camera_system.h"
#include "collisions.h"
#include "macros.h"
#include "easing.h"
#include "game_constants.h"
#include "gamma_flags.h"

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

  float speed = state.velocity.magnitude();

  // Extend the radius outward when moving faster
  baseRadius += 200.f * (speed / (speed + 500.f));

  if (state.dashLevel > 0) {
    // When dashing, reduce the radius
    baseRadius -= 150.f * (speed / (speed + 500.0f));
  }

  if (state.superjumpChargeTime != 0.f) {
    float alpha = state.superjumpChargeTime / (state.superjumpChargeTime + 2.f);

    baseRadius += 200.f * alpha;
  }

  float altitudeRadius = CAMERA_RADIUS_ALTITUDE_MULTIPLIER * state.camera3p.altitude / Gm_HALF_PI;

  state.camera3p.radius = baseRadius + altitudeRadius;
}

internal void updateThirdPersonCameraDirection(GmContext* context, GameState& state, float dt) {
  auto& input = get_input();

  // Handle camera orientation adjustments based on player/entity actions
  {
    // Dash landing jumps
    const float DASH_LANDING_JUMP_CAMERA_TRANSITION_TIME = 1.5f;

    if (
      state.lastDashLandingJumpTime != 0.f &&
      time_since(state.lastDashLandingJumpTime) < DASH_LANDING_JUMP_CAMERA_TRANSITION_TIME
    ) {
      auto alpha = easeOutQuint(time_since(state.lastDashLandingJumpTime) / DASH_LANDING_JUMP_CAMERA_TRANSITION_TIME);

      state.camera3p.altitude = Gm_Lerpf(state.dashLandingJumpStartCameraAltitude, Gm_PI * 0.4f, alpha);
    }

    // Launch pad jumps
    if (
      state.lastJumpPadLaunchTime != 0.f &&
      time_since(state.lastJumpPadLaunchTime) < 0.2f
    ) {
      auto alpha = time_since(state.lastJumpPadLaunchTime) / 0.2f;

      state.camera3p.altitude = Gm_Lerpf(state.camera3p.altitude, 0.f, alpha);
    }
  }

  if (
    !input.didPressKey(Key::SHIFT) &&
    (
      time_since(state.lastMouseMoveTime) < 5.f ||
      input.getRightStick().x != 0.f ||
      input.getRightStick().y != 0.f
    )
  ) {
    // Defer to manual camera control
    return;
  }


  // Tilt the camera when walking on sloped surfaces
  {
    if (state.isMovingPlayerThisFrame && state.isOnSolidGround) {
      auto& player = get_player();
      float targetAltitude = 0.2f;
      Vec3f motion = player.position - state.previousPlayerPosition;

      if (motion.magnitude() > 0.f) {
        targetAltitude += Gm_Absf(motion.unit().y);

        state.camera3p.altitude = Gm_Lerpf(state.camera3p.altitude, targetAltitude, 5.f * dt);
      }
    }
  }

  // Keep the camera centered behind the player
  {
    float targetAzimuth = state.camera3p.azimuth;

    if (state.direction.x != 0.f && state.direction.z != 0.f) {
      targetAzimuth = atan2f(state.direction.z, state.direction.x) + Gm_PI;
    }

    if ((state.isMovingPlayerThisFrame && state.isOnSolidGround) || state.isGliding) {
      state.camera3p.azimuth = Gm_LerpCircularf(state.camera3p.azimuth, targetAzimuth, 2.f * dt, Gm_PI);
    }

    if (input.didPressKey(Key::SHIFT)) {
      state.camera3p.azimuth = targetAzimuth;
    }
  }
}

internal Vec3f getLookAtTargetPosition(GmContext* context, GameState& state) {
  auto& camera = get_camera();
  auto& player = get_player();

  float currentCameraRadius = (camera.position - player.position).magnitude();
  auto playerUp = player.rotation.getUpDirection();
  float targetHeightAbovePlayer = state.cameraMode == CameraMode::FIRST_PERSON ? 20.f : 50.f;
  float lookAbovePlayerAdjustmentFactor = state.cameraMode == CameraMode::FIRST_PERSON ? 0.f : powf((1.f - state.camera3p.altitude - (-Gm_PI)) / Gm_TAU, 6.f);
  auto cameraUnitXzDirection = camera.orientation.getDirection().xz().unit();

  state.currentCameraLookatHeightAbovePlayer = targetHeightAbovePlayer + lookAbovePlayerAdjustmentFactor * 150.f;

  Vec3f defaultLookAtTarget = player.position + Vec3f(0, state.currentCameraLookatHeightAbovePlayer, 0);

  if (state.cameraMode == CameraMode::FIRST_PERSON) {
    defaultLookAtTarget += player.rotation.getDirection().invert().unit() * PLAYER_RADIUS * 0.8f;
  }

  if (state.cameraOverrideStartTime == 0.f) {
    // If we haven't started a camera transition yet,
    // use the default look-at target.
    return defaultLookAtTarget;
  }

  Vec3f tweenStart = state.sourceCameraState.lookAtTarget;
  Vec3f tweenEnd = state.useCameraOverride ? state.targetCameraState.lookAtTarget : defaultLookAtTarget;
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
  updateThirdPersonCameraDirection(context, state, 0.f);

  get_camera().position = get_player().position + state.camera3p.calculatePosition();
}

void CameraSystem::handleGameCamera(GmContext* context, GameState& state, float dt) {
  if (state.gameStartTime == 0.f) {
    return;
  }

  #if GAMMA_DEVELOPER_MODE
    if (state.isFreeCameraMode) {
      if (Gm_IsWindowFocused()) {
        auto& camera = get_camera();
        auto& input = get_input();
        auto& mouseDelta = input.getMouseDelta();
        auto speed = input.isKeyHeld(Key::SPACE) ? 200000.f : 10000.f;

        Gm_HandleFreeCameraMode(context, speed, dt);

        camera.orientation.yaw += mouseDelta.x / 1500.f;
        camera.orientation.pitch += mouseDelta.y / 1500.f;

        camera.rotation = camera.orientation.toQuaternion();
      }

      return;
    }
  #endif

  START_TIMING("handleGameCamera");

  updateThirdPersonCameraRadius(context, state, dt);
  updateThirdPersonCameraDirection(context, state, dt);
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

        state.camera3p.azimuth -= input.getRightStick().x * 5.f * dt;
        state.camera3p.altitude += input.getRightStick().y * 5.f * dt;

        state.camera3p.limitAltitude(0.9f);
      }

      // Handle view mode changes
      {
        // @todo use a different input for this
        // @todo handle first-person mode
        if (input.didPressKey(Key::CONTROLLER_Y)) {
          if (state.cameraMode == CameraMode::NORMAL) {
            state.cameraMode = CameraMode::ZOOM_OUT;
          } else {
            state.cameraMode = CameraMode::NORMAL;
          }
        }

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
    }

    // Wrap the azimuth to [0, Gm_TAU]
    state.camera3p.azimuth = Gm_Modf(state.camera3p.azimuth, Gm_TAU);
  }

  // @todo move to game_constants.h
  const float titleTransitionDuration = 3.f;

  // Reposition the camera if necessary to avoid clipping inside walls
  // @todo move this into its own function
  {
    auto& camera = get_camera();
    bool didRepositionCamera = false;
    bool isTitleScreenTransition = time_since(state.gameStartTime) < titleTransitionDuration;

    if (!isTitleScreenTransition) {
      for (auto& plane : state.collisionPlanes) {
        // Early out for collision planes not local to the look at/target camera positions
        if (plane.minY > lookAtPosition.y && plane.minY > targetCameraPosition.y) continue;
        if (plane.maxY < lookAtPosition.y && plane.maxY < targetCameraPosition.y) continue;

        auto collision = Collisions::getLinePlaneCollision(lookAtPosition, targetCameraPosition, plane);
        auto cDotN = Vec3f::dot(targetCameraPosition - collision.point, plane.normal);

        if (collision.hit && cDotN < 0.f) {
          auto& collisionBox = *get_object_by_record(collision.plane.sourceObjectRecord);
          auto& scale = collisionBox.scale;
          auto matInverseRotation = collisionBox.rotation.toMatrix4f().inverse();
          auto collisionBoxToTargetCamera = targetCameraPosition - collisionBox.position;
          auto target = matInverseRotation.transformVec3f(collisionBoxToTargetCamera);

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
  }

  // Update the actual view camera position/orientation
  {
    auto& camera = get_camera();

    if (context->scene.frame > 0) {
      #if GAMMA_DEVELOPER_MODE
        // Allow camera panning to behave at full speed, irrespective of game speed
        float alpha = 10.f * (dt / state.gameSpeed);
      #else
        float alpha = 10.f * dt;
      #endif

      camera.position = Vec3f::lerp(camera.position, targetCameraPosition, alpha);
    } else {
      camera.position = targetCameraPosition;
    }

    #if GAMMA_DEVELOPER_MODE
      point_camera_at(lookAtPosition);
    #else
      if (time_since(state.gameStartTime) > titleTransitionDuration) {
        point_camera_at(lookAtPosition);
      } else {
        // @temporary
        float alpha = easeInOutQuart(time_since(state.gameStartTime) / titleTransitionDuration);

        camera.position = Vec3f::lerp(CAMERA_TITLE_SCREEN_POSITION, targetCameraPosition, alpha);
        camera.orientation.yaw = Gm_Lerpf(Gm_PI * 0.6f, Gm_PI, alpha);
        camera.orientation.pitch = Gm_Lerpf(0.f, 0.1f, alpha);
        camera.rotation = camera.orientation.toQuaternion();
      }
    #endif
  }

  // Adjust the field of view when moving at higher speeds
  {
    auto& camera = get_camera();
    float speedOverFovLimit = state.velocity.magnitude() - CAMERA_FOV_MINIMUM_SPEED;
    if (speedOverFovLimit < 0.f) speedOverFovLimit = 0.f;

    float fovSpeedFactor = speedOverFovLimit / (speedOverFovLimit + CAMERA_FOV_SPEED_INTERVAL);
    float targetFov = CAMERA_BASE_FOV + CAMERA_FOV_VELOCITY_MULTIPLIER * fovSpeedFactor;
    float alpha = Gm_Clampf(10.f * dt, 0.f, 1.f);

    if (state.dashLevel == 1) targetFov *= 1.1f;

    if (state.dashLevel == 2) {
      targetFov *= 1.2f;

      if (state.lastBoostTime != 0.f) {
        targetFov *= 1.f + 0.2f * (time_since(state.lastBoostTime) / (time_since(state.lastBoostTime) + 3.f));
      }
    }

    if (state.isDoingTargetedAirDash) targetFov *= 1.2f;

    if (state.lastBoostTime != 0.f && time_since(state.lastBoostTime) < 0.2f) {
      // Instantaneous peed boost FoV adjustments
      float alpha = 1.f - time_since(state.lastBoostTime) / 0.2f;

      targetFov += 15.f * alpha;
    }

    if (state.superjumpChargeTime != 0.f) {
      // Charged super jump FoV adjustments
      float alpha = state.superjumpChargeTime / (state.superjumpChargeTime + 2.f);

      targetFov -= 30.f * alpha;
    }

    if (
      state.lastAirDashTime != 0.f &&
      time_since(state.lastAirDashTime) < AIR_DASH_SPIN_DURATION
    ) {
      targetFov += 10.f * (1.f - time_since(state.lastAirDashTime) / AIR_DASH_SPIN_DURATION);
    }

    camera.fov = Gm_Lerpf(camera.fov, targetFov, alpha);
  }

  LOG_TIME();
}

// @todo remove state argument if we're not using it
void CameraSystem::handleVisibilityCulling(GmContext* context, GameState& state) {
  START_TIMING("handleVisibilityCulling");

  use_frustum_culling({
    "petal", "p_small-leaves",
    "lamp", "ladder",
    "ac-unit", "ac-fan",
    "vent-piece", "vent-corner",
    "round-vent-piece", "round-vent-corner",
    "p_ramen-sign",
    "sphere-sign",
    "stair-step",
    "bird-at-rest", "bird-flying"
  });

  use_distance_and_frustum_culling(10000.f, {
    "p_ramen-bowl",
    "p_dishes-1", "p_dumplings-1", "p_fish-1", "p_meat-1"
  });

  use_distance_culling(3000.f, { "flower" });
  use_distance_culling(4000.f, { "weeds", "p_weeds", "p_small-flower", "p_small-cosmo" });

  use_distance_culling(10000.f, {
    "electrical-pole",
    "paper-lantern", "orange-lantern",
    "onigiri", "nitamago", "chashu", "narutomaki"
  });

  // @todo store somewhere as a constant
  static std::initializer_list<std::string> meshesToFrustumCullAt1K = {
    "wood-planter", "plant-pot",
    "generator",
    "p_town-sign-spinner", "spinner-1", "pinwheel"
  };

  // @todo store somewhere as a constant
  static std::initializer_list<std::string> meshesToFrustumCullAt10K = {
    // Static meshes
    "b1-base", "b1-levels", "b1-windows",
    "b2-base", "b2-levels", "b2-columns", "b2-windows",
    "b3-base", "b3-levels", "b3-columns",
    "b4", "b4-columns", "b4-windows",
    "mini-building",
    "windmill-base", "windmill-wheel",
    "exhaust-fan", "exhaust-fan-blades",
    "cylinder-1", "cylinder-2", "dome-1",
    "concrete-b1",
    "round-tower-base", "round-tower-supports", "round-tower-roof",
    "wood-house-base", "wood-house-roof",
    "wood-tower", "wood-tower-top",
    "bridge-1-supports",
    "clock-tower",
    "wood-supports",
    "tree-trunk", "tall-trunk",
    "branch-1", "bush", "leaves", "banana-plant",
    "japanese-tree", "japanese-tree-leaves", "bamboo", "rock-1",
    "circle-sign", "circle-sign-frame",
    "dynamic-wave-sign",
    "small-boat",

    // Procedural meshes
    "mini-flag",
    "p_mini-house", "p_mini-house-roof", "p_mini-house-wood-beam", "p_mini-house-window",
    "p_shrub", "p_shrub-2", "p_banana-plant"
  };

  use_frustum_culling_at_distance(1000.f, meshesToFrustumCullAt1K);
  use_frustum_culling_at_distance(10000.f, meshesToFrustumCullAt10K);

  LOG_TIME();
}

void CameraSystem::handleLevelsOfDetail(GmContext* context) {
  START_TIMING("handleLevelsOfDetail");

  use_lod_by_distance(3000.f, { "vent-piece" });

  use_lod_by_distance(5000.f, {
    "b1-levels", "circle-sign-frame", "hot-air-balloon", "mini-flag", "vertical-banner", "branch-1",
    "p_shrub", "p_shrub-2",
    "japanese-tree", "japanese-tree-leaves"
  });

  use_lod_by_distance(8000.f, {
    "b2-levels", "banana-plant",
    "p_mini-house-roof", "p_banana-plant",
    "garden-terrain", "bamboo",
    "wood-tower"
  });

  use_lod_by_distance(10000.f, {
    "bush"
  });

  use_lod_by_distance(30000.f, {
    "bathhouse-roof-segment", "bathhouse-roof-corner"
  });

  LOG_TIME();
}

void CameraSystem::setTargetCameraState(GmContext* context, GameState& state, const CameraState& cameraState) {
  state.originalCameraState.camera3p = state.camera3p;
  state.originalCameraState.lookAtTarget = getLookAtTargetPosition(context, state);

  state.sourceCameraState = state.originalCameraState;
  state.targetCameraState = cameraState;

  state.useCameraOverride = true;
  state.cameraOverrideStartTime = get_scene_time();
}

void CameraSystem::restoreOriginalCameraState(GmContext* context, GameState& state) {
  state.sourceCameraState.camera3p = state.camera3p;
  state.sourceCameraState.lookAtTarget = getLookAtTargetPosition(context, state);
  
  state.targetCameraState = state.originalCameraState;

  state.cameraOverrideStartTime = get_scene_time();
  state.useCameraOverride = false;
}