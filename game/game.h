#pragma once

#include "Gamma.h"

struct CollisionVolume {
  Gamma::Vec3f corner1;
  Gamma::Vec3f corner2;
  // @todo account for rotation
};

struct GameState {
  Gamma::ThirdPersonCamera camera3p;
  Gamma::Vec3f velocity;
  Gamma::Vec3f previousPlayerPosition;

  // @todo
  std::vector<CollisionVolume> collisions;
};

void initializeGame(GmContext* context, GameState& state);
void updateGame(GmContext* context, GameState& state, float dt);