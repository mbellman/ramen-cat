#pragma once

#include "game.h"

namespace Editor {
  void enableGameEditor(GmContext* context, GameState& state);
  void disableGameEditor(GmContext* context, GameState& state);
  void handleGameEditor(GmContext* context, GameState& state, float dt);
}