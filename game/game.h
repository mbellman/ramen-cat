#pragma once

#include "Gamma.h"

struct Plane {
  Gamma::Vec3f p1, p2, p3, p4;
  Gamma::Vec3f t1, t2, t3, t4;
  Gamma::Vec3f normal;
  float nDotU = 0.f;
};

struct GameState {
  Gamma::ThirdPersonCamera camera3p;
  Gamma::Vec3f velocity;
  Gamma::Vec3f previousPlayerPosition;
  Gamma::Vec3f lastSolidGroundPosition;

  float frameStartTime = 0.f;
  float lastWallBumpTime = 0.f;
  float lastTimeOnSolidGround = 0.f;
  float lastJumpInputTime = 0.f;

  bool isOnSolidGround = false;
  bool isPlayerMovingThisFrame = false;

  std::vector<Plane> collisionPlanes;
};

void initializeGame(GmContext* context, GameState& state);
void updateGame(GmContext* context, GameState& state, float dt);