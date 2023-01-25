#include <string>

#include "Gamma.h"

#include "game.h"
#include "world.h"
#include "movement_system.h"
#include "camera_system.h"
#include "entity_system.h"
#include "effects_system.h"
#include "ui_system.h"
#include "editor.h"
#include "game_constants.h"
#include "macros.h"

using namespace Gamma;

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

    // @todo use in dev mode only
    if (key == Key::L) {
      Gm_ToggleFlag(GammaFlags::ENABLE_DEV_LIGHT_DISCS);
    }

    // @todo use in dev mode only
    if (key == Key::T) {
      Gm_ToggleFlag(GammaFlags::ENABLE_DEV_TOOLS);
    }
  });

  // @todo use in dev mode only
  input.on<Key>("keystart", [&state, &input, context](Key key) {
    if (key == Key::E) {
      if (state.isEditorEnabled) {
        Editor::disableGameEditor(context, state);
      } else {
        Editor::enableGameEditor(context, state);
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
}

void initializeGame(GmContext* context, GameState& state) {
  Gm_EnableFlags(GammaFlags::VSYNC);
  Gm_DisableFlags(GammaFlags::ENABLE_DEV_TOOLS);

  initializeInputHandlers(context, state);

  World::initializeGameWorld(context, state);
  CameraSystem::initializeGameCamera(context, state);
  EntitySystem::initializeGameEntities(context, state);
  EffectsSystem::initializeGameEffects(context, state);
  UISystem::initializeUI(context, state);

  #if GAMMA_DEVELOPER_MODE == 1
    Editor::initializeGameEditor(context, state);
  #endif

  state.previousPlayerPosition = get_player().position;

  // Set title screen position
  {
    auto& camera = get_camera();

    camera.position = CAMERA_TITLE_SCREEN_POSITION;
    camera.orientation.yaw = Gm_PI * 0.6f;
    camera.rotation = camera.orientation.toQuaternion();
  }
}

void updateGame(GmContext* context, GameState& state, float dt) {
  // @todo check in dev mode only
  if (state.isEditorEnabled) {
    Editor::handleGameEditor(context, state, dt);

    return;
  }

  auto& player = get_player();

  // Show debug info
  {
    add_debug_message("Player position: " + Gm_ToString(player.position));
    add_debug_message("Player velocity: " + Gm_ToString(state.velocity));
    add_debug_message("Day/Night cycle time: " + std::to_string(state.dayNightCycleTime));
  }

  START_TIMING("updateGame");

  // Set start-of-frame variables
  {
    state.frameStartTime = get_running_time();
    state.isMovingPlayerThisFrame = false;
  }

  EntitySystem::handleGameEntities(context, state, dt);
  MovementSystem::handlePlayerMovementInput(context, state, dt);
  MovementSystem::handlePlayerMovementPhysics(context, state, dt);
  CameraSystem::handleGameCamera(context, state, dt);
  CameraSystem::handleVisibilityCullingAndLevelsOfDetail(context, state);
  EffectsSystem::handleGameEffects(context, state, dt);
  UISystem::handleUI(context, state, dt);

  // @temporary
  {
    if (state.gameStartTime == 0.f && get_input().didPressKey(Key::SPACE)) {
      state.gameStartTime = state.frameStartTime;
    }
  }

  // Reset the player position after falling longer than 2 seconds
  {
    if (
      state.frameStartTime - state.lastTimeOnSolidGround > 2.f &&
      state.lastSolidGroundPosition.y - player.position.y > 1000.f
    ) {
      state.velocity = Vec3f(0.f);
      state.lastTimeOnSolidGround = state.frameStartTime;

      player.position = state.lastSolidGroundPosition;

      UISystem::showDialogue(context, state, "You fell down.\nPlease be careful next time.", {
        .duration = 3.f,
        .blocking = false
      });
    }
  }

  #if GAMMA_DEVELOPER_MODE == 1
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
      auto& input = get_input();

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
  #endif

  // Track end-of-frame variables
  {
    Vec3f motionLine = player.position - state.previousPlayerPosition;

    if (motionLine.magnitude() > 1.f) {
      state.direction = motionLine;
    }

    state.previousPlayerPosition = player.position;
  }

  LOG_TIME();
}