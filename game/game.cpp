#include <string>

#include "Gamma.h"

#include "game.h"
#include "world.h"
#include "movement_system.h"
#include "animation_system.h"
#include "camera_system.h"
#include "entity_system.h"
#include "effects_system.h"
#include "ui_system.h"
#include "editor.h"
#include "game_constants.h"
#include "macros.h"

using namespace Gamma;

internal void handleTimeCommand(GmContext* context, GameState& state, const std::string& command) {
  float time;

  try {
    auto parts = Gm_SplitString(command, " ");

    time = stof(parts[1]);
  } catch (const std::exception& e) {
    Console::warn("Invalid time command");

    return;
  }

  state.dayNightCycleTime = time;

  EffectsSystem::updateDayNightCycleLighting(context, state);
}

internal void handleLevelCommand(GmContext* context, GameState& state, const std::string& command) {
  std::string levelName;
  float time;

  try {
    auto parts = Gm_SplitString(command, " ");

    levelName = parts[1];
  } catch (const std::exception& e) {
    Console::warn("Invalid level command");

    return;
  }

  World::loadLevel(context, state, levelName);
}

// @todo move this elsewhere
internal void initializeInputHandlers(GmContext* context, GameState& state) {
  auto& input = get_input();

  input.on<MouseButtonEvent>("mousedown", [&](const MouseButtonEvent& event) {
    if (!Gm_IsWindowFocused()) {
      Gm_FocusWindow();
    }
  });

  input.on<Key>("keyup", [&](Key key) {
    if (key == Key::ESCAPE) {
      Gm_UnfocusWindow();
    }

    #if GAMMA_DEVELOPER_MODE
      if (key == Key::L) {
        Gm_ToggleFlag(GammaFlags::ENABLE_DEV_LIGHT_DISCS);
      }

      if (key == Key::T) {
        Gm_ToggleFlag(GammaFlags::ENABLE_DEV_TOOLS);
      }

      if (key == Key::B) {
        Gm_ToggleFlag(GammaFlags::ENABLE_DEV_BUFFERS);
      }
    #endif
  });

  #if GAMMA_DEVELOPER_MODE
    input.on<Key>("keystart", [&state, &input, context](Key key) {
      if (key == Key::E) {
        if (state.isEditorEnabled) {
          Editor::disableGameEditor(context, state);
          Console::log("Exiting editor mode");
        } else {
          Editor::enableGameEditor(context, state);
          Console::log("Entering editor mode");
        }
      }

      if (key == Key::V && !input.isKeyHeld(Key::CONTROL)) {
        if (Gm_IsFlagEnabled(GammaFlags::VSYNC)) {
          Gm_DisableFlags(GammaFlags::VSYNC);
        } else {
          Gm_EnableFlags(GammaFlags::VSYNC);
        }
      }
    });

    context->commander.on<std::string>("command", [&state, context](std::string command) {
      if (Gm_StringStartsWith(command, "time")) {
        handleTimeCommand(context, state, command);
      } else if (Gm_StringStartsWith(command, "level")) {
        handleLevelCommand(context, state, command);
      }
    });
  #endif
}

void initializeGame(GmContext* context, GameState& state) {
  Gm_EnableFlags(GammaFlags::VSYNC);
  Gm_DisableFlags(GammaFlags::ENABLE_DEV_TOOLS);

  initializeInputHandlers(context, state);

  World::initializeGameWorld(context, state);
  World::loadLevel(context, state, "overworld");
  AnimationSystem::initializeAnimations(context, state);
  CameraSystem::initializeGameCamera(context, state);
  EntitySystem::initializeGameEntities(context, state);
  EffectsSystem::initializeGameEffects(context, state);
  UISystem::initializeUI(context, state);

  #if GAMMA_DEVELOPER_MODE
    Editor::initializeGameEditor(context, state);
  #endif

  state.previousPlayerPosition = get_player().position;

  // Set title screen position
  // @todo do this per-level
  {
    #if GAMMA_DEVELOPER_MODE
      // Skip the title screen transition
      context->scene.sceneTime = 3.f;
      state.gameStartTime = 3.f;
    #else
      auto& camera = get_camera();

      camera.position = CAMERA_TITLE_SCREEN_POSITION;
      camera.orientation.yaw = Gm_PI * 0.6f;
      camera.rotation = camera.orientation.toQuaternion();
    #endif
  }
}

void updateGame(GmContext* context, GameState& state, float dt) {
  // @todo check in dev mode only
  if (state.isEditorEnabled) {
    Editor::handleGameEditor(context, state, dt);
    EntitySystem::handleOcean(context);

    // Update our 'altitude' parameter so height-based lighting changes take effect in the editor
    context->scene.sky.altitude = get_camera().position.y - OCEAN_PLANE_Y_POSITION;

    return;
  }

  auto& player = get_player();
  auto& input = get_input();

  #if GAMMA_DEVELOPER_MODE
    dt *= state.gameSpeed;

    // Show debug info
    {
      add_debug_message("Player position: " + Gm_ToDebugString(player.position));
      add_debug_message("Player velocity: " + Gm_ToDebugString(state.velocity));
      add_debug_message("Day/Night cycle time: " + std::to_string(state.dayNightCycleTime));
      add_debug_message("Game speed: " + std::to_string(state.gameSpeed));
    }
  #endif

  START_TIMING("updateGame");

  // Set start-of-frame variables
  {
    state.isMovingPlayerThisFrame = false;
  }

  MovementSystem::handlePlayerMovementInput(context, state, dt);
  MovementSystem::handlePlayerMovementPhysics(context, state, dt);
  EntitySystem::handleGameEntities(context, state, dt);
  AnimationSystem::handleAnimations(context, state, dt);
  CameraSystem::handleGameCamera(context, state, dt);
  CameraSystem::handleVisibilityCullingAndLevelsOfDetail(context, state);
  EffectsSystem::handleGameEffects(context, state, dt);
  UISystem::handleUI(context, state, dt);

  // @temporary
  {
    if (state.gameStartTime == 0.f) {
      if (input.didPressKey(Key::SPACE)) {
        state.gameStartTime = get_scene_time();
      }

      #if GAMMA_DEVELOPER_MODE
        if (input.didPressKey(Key::ENTER)) {
          state.gameStartTime = get_scene_time() - 2.9f;
        }
      #endif
    }
  }

  // Reset the player position when falling past the killplane
  {
    if (
      player.position.y < -1000.f
    ) {
      player.position = state.lastSolidGroundPosition;

      state.velocity = Vec3f(0.f);
      state.lastTimeOnSolidGround = get_scene_time();

      UISystem::showDialogue(context, state, "You fell down.\nPlease be careful next time.", {
        .duration = 3.f,
        .blocking = false
      });
    }
  }

  #if GAMMA_DEVELOPER_MODE
    // Keep track of our last solid ground positions so we can undo movements
    {
      if (
        !state.isOnSolidGround && (
          state.lastSolidGroundPositions.size() == 0 ||
          state.lastSolidGroundPositions.back() != state.lastSolidGroundPosition
        )
      ) {
        state.lastSolidGroundPositions.push_back(state.lastSolidGroundPosition);
      }
    }

    // Allow CTRL-Z to 'rewind' through our last solid ground positions
    {
      if (
        input.isKeyHeld(Key::CONTROL) &&
        input.didPressKey(Key::Z) &&
        state.lastSolidGroundPositions.size() > 0
      ) {
        player.position = state.lastSolidGroundPositions.back();

        state.velocity = Vec3f(0.f);
        state.lastSolidGroundPositions.pop_back();
      }
    }

    // Allow LEFT/RIGHT to adjust the game speed
    {
      if (input.didPressKey(Key::ARROW_LEFT)) {
        state.gameSpeed *= 0.5f;

        Console::log("Reduced game speed to", state.gameSpeed);
      }

      if (input.didPressKey(Key::ARROW_RIGHT)) {
        state.gameSpeed *= 2.f;

        Console::log("Increased game speed to", state.gameSpeed);
      }
    }

    // Allow F to enter free camera mode
    {
      if (input.didPressKey(Key::F)) {
        state.isFreeCameraMode = !state.isFreeCameraMode;
      }
    }
  #endif

  // Track end-of-frame variables
  {
    Vec3f motionLine = player.position - state.previousPlayerPosition;
    float sceneTime = get_scene_time();

    if (motionLine.magnitude() > 1.f) {
      state.direction = motionLine;
    }

    state.previousPlayerPosition = player.position;
    state.wasOnSolidGroundLastFrame = state.isOnSolidGround;
    state.totalDistanceTraveled += state.velocity.xz().magnitude() * dt;

    if (!state.isOnSolidGround) {
      state.lastTimeInAir = sceneTime;
    }

    if (input.didMoveMouse()) {
      state.lastMouseMoveTime = sceneTime;
    }

    context->scene.sceneTime += dt;
  }

  Vec3f playerPosition = player.position;
  Quaternion playerRotation = player.rotation;

  // Apply glider-specific character transforms after all other steps.
  // We want these effects to be visible, but not permanently set on
  // the player object, since the player position/orientation determine
  // certain aspects of camera behavior and controls.
  if (state.isGliding) {
    // @todo fix player model orientation
    Vec3f forward = player.rotation.getDirection().invert();
    float alpha = Vec3f::dot(forward, Vec3f(0, 1.f, 0));

    player.position -= player.rotation.getUpDirection() * PLAYER_RADIUS * 0.5f * alpha;
    player.position += forward * alpha * PLAYER_RADIUS;
    player.rotation *= Quaternion::fromAxisAngle(player.rotation.getLeftDirection(), -alpha);
  }

  // Commit any changes to the player object
  commit(player);

  // Reset glider-specific character transforms
  player.position = playerPosition;
  player.rotation = playerRotation;

  LOG_TIME();
}