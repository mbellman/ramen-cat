#pragma once

#include "Gamma.h"

struct Plane {
  Gamma::Vec3f p1, p2, p3, p4;
  Gamma::Vec3f normal;
  float nDotU = 0.f;
};

struct GameState {
  Gamma::ThirdPersonCamera camera3p;
  Gamma::Vec3f velocity;
  Gamma::Vec3f previousPlayerPosition;
  Gamma::Vec3f lastSolidGroundPosition;
  u64 frameStartTime = 0;
  u64 lastWallBumpTime = 0;
  u64 lastTimeOnSolidGround = 0;
  bool isPlayerMovingThisFrame = false;

  std::vector<Plane> collisionPlanes;
};

void initializeGame(GmContext* context, GameState& state);
void updateGame(GmContext* context, GameState& state, float dt);