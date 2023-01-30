#pragma once

#include <string>
#include <vector>

#include "Gamma.h"

struct Plane {
  Gamma::Vec3f p1, p2, p3, p4;
  Gamma::Vec3f t1, t2, t3, t4;
  Gamma::Vec3f normal;
  float nDotU = 0.f;
  float maxY = 0.f;
  float minY = 0.f;

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
  float xzVelocity = 350.f;
  float yVelocity = 1500.f;
  float initialRotation = 0.f;
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
  // @temporary
  float gameStartTime = 0.f;

  Gamma::ThirdPersonCamera camera3p;
  Gamma::Vec3f velocity;
  Gamma::Vec3f direction = Gamma::Vec3f(0, 0, 1.f);
  Gamma::Vec3f previousPlayerPosition;
  Gamma::Vec3f lastSolidGroundPosition;
  Gamma::Vec3f lastWallBumpNormal;
  Gamma::Vec3f lastWallBumpVelocity;

  CameraMode cameraMode = CameraMode::NORMAL;

  float frameStartTime = 0.f;
  float lastTimeOnSolidGround = 0.f;
  float lastWallBumpTime = 0.f;
  float lastGroundParticleSpawnTime = 0.f;
  float lastAirParticleSpawnTime = 0.f;

  float dayNightCycleTime = 0.f;

  bool isOnSolidGround = false;
  bool isMovingPlayerThisFrame = false;
  bool isDashing = false;
  bool isRepositioningCamera = false;

  bool canPerformWallKick = false;
  bool canPerformAirDash = false;

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

  std::vector<Plane> collisionPlanes;

  std::vector<NonPlayerCharacter> npcs;

  std::vector<Slingshot> slingshots;
  // @todo define a struct for this
  float lastSlingshotInteractionTime = 0.f;
  Gamma::ObjectRecord activeSlingshotRecord;
  Gamma::Vec3f slingshotVelocity;
  float startingSlingshotAngle = 0.f;
  float targetSlingshotAngle = 0.f;

  std::vector<Gamma::Object> initialMovingObjects;

  #if GAMMA_DEVELOPER_MODE == 1
    std::vector<Gamma::Vec3f> lastSolidGroundPositions;
  #endif
};

void initializeGame(GmContext* context, GameState& state);
void updateGame(GmContext* context, GameState& state, float dt);