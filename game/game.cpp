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
#include "macros.h"

using namespace Gamma;

// @todo move this elsewhere
internal void initializeInputHandlers(GmContext* context, GameState& state) {
  auto& input = get_input();

  input.on<MouseButtonEvent>("mousedown", [&](const MouseButtonEvent& event) {
    if (!SDL_GetRelativeMouseMode()) {
      SDL_SetRelativeMouseMode(SDL_TRUE);
    }
  });

  input.on<Key>("keyup", [&](Key key) {
    if (key == Key::ESCAPE) {
      SDL_SetRelativeMouseMode(SDL_FALSE);
    }

    // @todo use in dev mode only
    if (key == Key::V) {
      if (Gm_IsFlagEnabled(GammaFlags::VSYNC)) {
        Gm_DisableFlags(GammaFlags::VSYNC);
      } else {
        Gm_EnableFlags(GammaFlags::VSYNC);
      }
    }
  });

  input.on<Key>("keystart", [&state, context](Key key) {
    if (key == Key::E) {
      if (state.isEditorEnabled) {
        Editor::disableGameEditor(context, state);
      } else {
        Editor::enableGameEditor(context, state);
      }
    }
  });
}

void initializeGame(GmContext* context, GameState& state) {
  Gm_EnableFlags(GammaFlags::VSYNC);

  initializeInputHandlers(context, state);

  World::initializeGameWorld(context, state);
  CameraSystem::initializeGameCamera(context, state);
  EntitySystem::initializeGameEntities(context, state);
  EffectsSystem::initializeGameEffects(context, state);
  UISystem::initializeUI(context, state);
  Editor::initializeGameEditor(context, state);

  state.previousPlayerPosition = get_player().position;
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
    add_debug_message("Position: " + Gm_ToString(player.position));
    add_debug_message("Velocity: " + Gm_ToString(state.velocity));
  }

  START_TIMING("updateGame");

  // Track start-of-frame variables
  {
    state.frameStartTime = get_running_time();
    state.isPlayerMovingThisFrame = false;
  }

  MovementSystem::handlePlayerMovementInput(context, state, dt);
  MovementSystem::handlePlayerMovementPhysics(context, state, dt);
  CameraSystem::handleGameCamera(context, state, dt);
  EntitySystem::handleGameEntities(context, state, dt);
  EffectsSystem::handleGameEffects(context, state, dt);
  UISystem::handleUI(context, state, dt);

  // Reset the player position after falling longer than 2 seconds
  {
    if (
      state.frameStartTime - state.lastTimeOnSolidGround > 2.f &&
      state.lastSolidGroundPosition.y - player.position.y > 1000.f
    ) {
      state.velocity = Vec3f(0.f);
      state.lastTimeOnSolidGround = state.frameStartTime;

      player.position = state.lastSolidGroundPosition;

      UISystem::showDialogue(context, state, "You fell down.\nPlease be careful next time.", 3.f);
    }
  }

  // Track end-of-frame variables
  {
    if (state.isOnSolidGround && state.velocity.xz().magnitude() > 1.f) {
      // If we were on solid ground, but any movement
      // occurs along the xz plane, all bets are off!
      state.isOnSolidGround = false;
    }

    state.previousPlayerPosition = player.position;
  }

  LOG_TIME();
}