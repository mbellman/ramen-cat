#pragma once

#include <string>
#include <vector>

#include "Gamma.h"

struct Plane {
  Gamma::Vec3f p1, p2, p3, p4;
  Gamma::Vec3f t1, t2, t3, t4;
  Gamma::Vec3f normal;
  float nDotU = 0.f;
};

struct NonPlayerCharacter {
  Gamma::Vec3f position;
  std::vector<std::string> dialogue;
};

struct GameState {
  Gamma::ThirdPersonCamera camera3p;
  Gamma::Vec3f velocity;
  Gamma::Vec3f previousPlayerPosition;
  Gamma::Vec3f lastSolidGroundPosition;
  Gamma::Vec3f lastBumpedWallNormal;

  float frameStartTime = 0.f;
  float lastTimeOnSolidGround = 0.f;
  float lastWallKickInputTime = 0.f;
  float lastWallBumpTime = 0.f;
  float lastWallKickTime = 0.f;
  float lastParticleSpawnTime = 0.f;

  bool isOnSolidGround = false;
  bool isPlayerMovingThisFrame = false;

  // NPC handling
  NonPlayerCharacter* activeNPC = nullptr;
  u8 npcDialogueStep = 0;

  // Secondary look-at target/camera handling
  Gamma::Vec3f tweenLookAtStart;
  Gamma::Vec3f* tweenLookAtTarget = nullptr;
  float tweenLookAtStartTime = 0.f;

  // @todo use in dev mode only
  bool isEditorEnabled = false;

  // @todo calculate nearby collision planes on each frame,
  // and only check collisions against those
  std::vector<Plane> collisionPlanes;

  std::vector<NonPlayerCharacter> npcs;
};

void initializeGame(GmContext* context, GameState& state);
void updateGame(GmContext* context, GameState& state, float dt);