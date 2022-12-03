#pragma once

#include "Gamma.h"

struct GameState {
  Gamma::ThirdPersonCamera camera;
};

void initializeGame(GmContext* context, GameState& state);
void updateGame(GmContext* context, GameState& state, float dt);