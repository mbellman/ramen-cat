#pragma once

#include "Gamma.h"

struct GameState {
  Gamma::ThirdPersonCamera camera3p;
  Gamma::Vec3f velocity;
  Gamma::Vec3f previousPlayerPosition;
};

void initializeGame(GmContext* context, GameState& state);
void updateGame(GmContext* context, GameState& state, float dt);