#pragma once

#include <string>
#include <vector>

#include "Gamma.h"

struct Plane {
  Gamma::Vec3f p1, p2, p3, p4;
  Gamma::Vec3f t1, t2, t3, t4;
  Gamma::Vec3f normal;
  float nDotU = 0.f;

  #if GAMMA_DEVELOPER_MODE
    Gamma::ObjectRecord sourceObjectRecord;
  #endif
};

// @todo move to entity_system.h
struct NonPlayerCharacter {
  Gamma::Vec3f position;
  std::vector<std::string> dialogue;
};

// @todo move to entity_system.h
struct Slingshot {
  Gamma::Vec3f position;
};

struct CameraState {
  Gamma::ThirdPersonCamera camera3p;
  Gamma::Vec3f lookAtTarget;
};

enum CameraMode {
  FIRST_PERSON,
  NORMAL,
  ZOOM_OUT
};

struct GameState {
  Gamma::ThirdPersonCamera camera3p;
  Gamma::Vec3f velocity;
  Gamma::Vec3f direction = Gamma::Vec3f(0, 0, 1.f);
  Gamma::Vec3f previousPlayerPosition;
  Gamma::Vec3f lastSolidGroundPosition;
  Gamma::Vec3f lastBumpedWallNormal;

  CameraMode cameraMode = CameraMode::NORMAL;

  float frameStartTime = 0.f;
  float lastTimeOnSolidGround = 0.f;
  float lastWallKickInputTime = 0.f;
  float lastWallBumpTime = 0.f;
  float lastWallKickTime = 0.f;
  float lastParticleSpawnTime = 0.f;

  // @bug this isn't always correct
  bool isOnSolidGround = false;
  bool isPlayerMovingThisFrame = false;

  // NPC handling
  NonPlayerCharacter* activeNpc = nullptr;

  // Camera override handling
  CameraState originalCameraState;
  CameraState sourceCameraState;
  CameraState targetCameraState;
  float cameraOverrideStartTime = 0.f;
  float cameraOverrideDuration = 0.5f;
  bool useCameraOverride = false;

  // @todo use in dev mode only
  bool isEditorEnabled = false;

  // @todo calculate nearby collision planes on each frame,
  // and only check collisions against those
  std::vector<Plane> collisionPlanes;

  std::vector<NonPlayerCharacter> npcs;

  std::vector<Slingshot> slingshots;
  float lastSlingshotInteractionTime = 0.f;
  Gamma::ObjectRecord activeSlingshotRecord;
  float targetSlingshotAngle = 0.f;
  Gamma::Vec3f slingshotVelocity;
};

void initializeGame(GmContext* context, GameState& state);
void updateGame(GmContext* context, GameState& state, float dt);