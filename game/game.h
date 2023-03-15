#pragma once

#include <string>
#include <vector>

#include "Gamma.h"

#include "animation_system.h"

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

// @todo move to entity_system.h
struct Jetstream {
  std::vector<Gamma::Vec3f> points;
  float radius = 500.f;
};

struct InventoryItem {
  u8 count = 0;
  float firstCollectionTime = 0.f;
  float lastCollectionTime = 0.f;
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

  std::string currentLevelName;

  Gamma::ThirdPersonCamera camera3p;
  Gamma::Vec3f velocity;
  Gamma::Vec3f direction = Gamma::Vec3f(0, 0, 1.f);
  Gamma::Vec3f previousPlayerPosition;
  Gamma::Vec3f lastSolidGroundPosition;
  Gamma::Vec3f lastHardLandingPosition;
  Gamma::Vec3f lastWallBumpNormal;
  Gamma::Vec3f lastWallBumpVelocity;

  CameraMode cameraMode = CameraMode::NORMAL;

  float lastTimeOnSolidGround = 0.f;
  float lastTimeInAir = 0.f;
  float lastJumpTime = 0.f;
  float lastWallBumpTime = 0.f;
  float lastWallKickTime = 0.f;
  float lastAirDashTime = 0.f;
  float lastHardLandingTime = 0.f;
  float lastGroundParticleSpawnTime = 0.f;
  float lastAirParticleSpawnTime = 0.f;
  float lastGliderChangeTime = 0.f;
  float lastMouseMoveTime = 0.f;

  float dayNightCycleTime = 0.f;

  bool isOnSolidGround = false;
  bool wasOnSolidGroundLastFrame = false;
  bool isMovingPlayerThisFrame = false;
  bool isRepositioningCamera = false;
  bool isGliding = false;

  bool canPerformWallKick = false;
  bool canPerformAirDash = false;

  u8 dashLevel = 0;

  float totalDistanceTraveled = 0.f;
  float airDashSpinStartYaw = 0.f;
  float airDashSpinEndYaw = 0.f;
  float currentYaw = 0.f;
  float currentPitch = 0.f;
  float turnFactor = 0.f;

  struct Animation {
    AnimationRig playerRig;
  } animation;

  struct Inventory {
    InventoryItem onigiri;
    InventoryItem nitamago;
    InventoryItem chashu;

    InventoryItem demonOnigiri;
    InventoryItem demonNitamago;
    InventoryItem demonChashu;
  } inventory;

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

  std::vector<Jetstream> jetstreams;

  // Torii Gates
  bool isInToriiGateZone = false;
  float toriiGateTransitionTime = 0.f;

  // Rings
  Gamma::Object lastUsedRing;
  float lastRingLaunchTime = 0.f;

  std::vector<Gamma::Object> initialMovingObjects;

  #if GAMMA_DEVELOPER_MODE == 1
    std::vector<Gamma::Vec3f> lastSolidGroundPositions;
    float gameSpeed = 1.f;
    bool isFreeCameraMode = false;
  #endif
};

void initializeGame(GmContext* context, GameState& state);
void updateGame(GmContext* context, GameState& state, float dt);