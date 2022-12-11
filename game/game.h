#pragma once

#include "Gamma.h"

struct Plane {
  Gamma::Vec3f p1, p2, p3, p4;
  Gamma::Vec3f normal;
};

struct GameState {
  Gamma::ThirdPersonCamera camera3p;
  Gamma::Vec3f velocity;
  Gamma::Vec3f previousPlayerPosition;
  Gamma::Vec3f lastSolidGroundPosition;
  u64 lastTimeOnSolidGround = 0;
  bool isPlayerMovingThisFrame = false;

  std::vector<Plane> collisionPlanes;
};

void initializeGame(GmContext* context, GameState& state);
void updateGame(GmContext* context, GameState& state, float dt);