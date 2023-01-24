#include "entity_system.h"
#include "camera_system.h"
#include "ui_system.h"
#include "world.h"
#include "macros.h"
#include "easing.h"
#include "game_constants.h"

#define set_active_mesh(meshName) u16 __activeMeshIndex = context->scene.meshMap.at(meshName)->index
#define is_active_mesh(object) object._record.meshIndex == __activeMeshIndex

#define for_moving_objects(meshName, code)\
  set_active_mesh(meshName);\
  for (auto& initial : state.initialMovingObjects) {\
    if (is_active_mesh(initial)) {\
      auto* __object = get_object_by_record(initial._record);\
      if (__object != nullptr) {\
        auto& object = *__object;\
        code\
      }\
    }\
  }

using namespace Gamma;

internal void loadNpcData(GmContext* context, GameState& state) {
  // @todo eventually store as binary data
  auto npcDataContents = Gm_LoadFileContents("./game/data_npcs.txt");
  auto lines = Gm_SplitString(npcDataContents, "\n");

  // @temporary
  u32 i = 0;

  // @temporary
  while (i < lines.size()) {
    if (lines[i][0] == '@') {
      NonPlayerCharacter npc;

      // @todo parse NPC @type

      i++;

      npc.position = Gm_ParseVec3f(lines[i]);

      i++;
  
      std::string dialogueLine;

      while (i < lines.size() && lines[i][0] != '@') {
        auto line = lines[i++];

        if (line[0] == '-') {
          npc.dialogue.push_back(dialogueLine);

          dialogueLine = "";

          continue;
        }

        if (dialogueLine.size() > 0) {
          dialogueLine += '\n';
        }

        dialogueLine += line;
      }

      state.npcs.push_back(npc);
    }
  }

  // @temporary
  for (auto& npc : state.npcs) {
    auto& object = create_object_from("npc");

    object.position = npc.position;
    object.color = Vec3f(1.f, 0, 1.f);
    object.scale = Vec3f(NPC_RADIUS, NPC_HEIGHT, NPC_RADIUS);

    commit(object);
  }
}

internal void loadEntityData(GmContext* context, GameState& state) {
  // @todo eventually store as binary data
  auto entityDataContents = Gm_LoadFileContents("./game/data_entities.txt");
  auto lines = Gm_SplitString(entityDataContents, "\n");

  // @temporary
  std::string entityName;

  // @temporary
  for (u32 i = 0; i < lines.size(); i++) {
    auto& line = lines[i];

    if (line.size() == 0) {
      continue;
    }

    if (line[0] == '@') {
      entityName = line.substr(1);
    } else if (entityName == "slingshot") {
      auto parts = Gm_SplitString(line, ",");

      Vec3f position = Vec3f(stof(parts[0]), stof(parts[1]), stof(parts[2]));
      float xzVelocity = stof(parts[3]);
      float yVelocity = stof(parts[4]);
      float initialRotation = stof(parts[5]);

      state.slingshots.push_back({
        .position = position,
        .xzVelocity = xzVelocity,
        .yVelocity = yVelocity,
        .initialRotation = initialRotation
      });
    }
  }

  // @temporary
  for (auto& slingshot : state.slingshots) {
    auto& object = create_object_from("slingshot");

    object.position = slingshot.position;
    object.scale = Vec3f(60.f);
    object.rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), slingshot.initialRotation);
    object.color = DEFAULT_SLINGSHOT_COLOR;

    commit(object);
  }
}

internal void interactWithNpc(GmContext* context, GameState& state, NonPlayerCharacter& npc) {
  auto& player = get_player();
  Vec3f npcFacePosition = npc.position + Vec3f(0, 30.f, 0);
  Vec3f npcToPlayer = (player.position - npc.position);

  state.activeNpc = &npc;
  state.velocity = Vec3f(0.f);

  CameraSystem::setTargetCameraState(context, state, {
    .camera3p = {
      .azimuth = atan2f(npcToPlayer.z, npcToPlayer.x) - Gm_TAU / 8.f,
      .altitude = 0.f,
      .radius = NPC_INTERACTION_CAMERA_RADIUS
    },
    .lookAtTarget = {
      (npcFacePosition.x + player.position.x) / 2.f,
      npcFacePosition.y,
      (npcFacePosition.z + player.position.z) / 2.f
    }
  });

  UISystem::queueDialogue(context, state, npc.dialogue);
}

internal void handleNpcs(GmContext* context, GameState& state) {
  auto& input = get_input();
  auto& player = get_player();

  // Handle talking to NPCs
  {
    if (input.didPressKey(Key::SPACE)) {
      if (state.activeNpc == nullptr) {
        for (auto& npc : state.npcs) {
          float npcXzDistance = (npc.position - player.position).xz().magnitude();

          if (
            npcXzDistance < NPC_INTERACTION_TRIGGER_DISTANCE &&
            player.position.y < npc.position.y + NPC_HEIGHT &&
            player.position.y > npc.position.y - NPC_HEIGHT
          ) {
            interactWithNpc(context, state, npc);

            break;
          }
        }
      }
    }
  }

  // Handle ending conversions with NPCs
  {
    if (state.activeNpc != nullptr && UISystem::isDialogueQueueEmpty()) {
      state.activeNpc = nullptr;

      CameraSystem::restoreOriginalCameraState(context, state);
    }
  }
}

internal void interactWithSlingshot(GmContext* context, GameState& state, Object& object) {
  auto& player = get_player();

  // @temporary
  float xzVelocity = 350.f;
  float yVelocity = 1500.f;

  // @temporary
  for (auto& slingshot : state.slingshots) {
    if (slingshot.position == object.position) {
      xzVelocity = slingshot.xzVelocity;
      yVelocity = slingshot.yVelocity;

      break;
    }
  }

  Vec3f slingshotToPlayer = player.position - object.position;
  float slingshotToPlayerAngle = atan2f(slingshotToPlayer.z, slingshotToPlayer.x);
  Vec3f playerDirection = slingshotToPlayer.xz().unit();

  // @todo make configurable
  float xVelocity = xzVelocity * playerDirection.x * -1.f;
  float zVelocity = xzVelocity * playerDirection.z * -1.f;
  Vec3f slingshotVelocity = Vec3f(xVelocity, yVelocity, zVelocity);
  Vec3f slingshotDirection = object.rotation.getDirection();

  state.lastSlingshotInteractionTime = state.frameStartTime;
  state.startingSlingshotAngle = Gm_Modf(-atan2f(slingshotDirection.z, slingshotDirection.x) + Gm_HALF_PI, Gm_TAU);
  state.targetSlingshotAngle = Gm_Modf(-slingshotToPlayerAngle + Gm_HALF_PI, Gm_TAU);
  state.activeSlingshotRecord = object._record;
  state.slingshotVelocity = slingshotVelocity;
  state.velocity = Vec3f(0.f);

  // Control camera override behavior
  {
    CameraSystem::setTargetCameraState(context, state, {
      .camera3p = {
        .azimuth = slingshotToPlayerAngle - Gm_PI / 16.f,
        .altitude = 0.f,
        .radius = SLINGSHOT_WIND_UP_CAMERA_RADIUS
      },
      .lookAtTarget = object.position
    });

    // Have the restored camera state center behind/above
    // the player, once launched from the slingshot
    state.originalCameraState.camera3p.azimuth = Gm_Modf(atan2f(slingshotVelocity.z, slingshotVelocity.x) - Gm_PI, Gm_TAU);
    state.originalCameraState.camera3p.altitude = Gm_HALF_PI * 0.8f;
    state.originalCameraState.camera3p.radius =
      (state.cameraMode == CameraMode::NORMAL ? CAMERA_NORMAL_BASE_RADIUS : CAMERA_ZOOM_OUT_BASE_RADIUS)
      + CAMERA_RADIUS_ALTITUDE_MULTIPLIER * (state.originalCameraState.camera3p.altitude / Gm_HALF_PI);
  }
}

internal void handleSlingshots(GmContext* context, GameState& state, float dt) {
  auto& player = get_player();
  auto& input = get_input();

  // Handle proximity to slingshots
  {
    for (auto& slingshot : objects("slingshot")) {
      Vec3f targetColor = DEFAULT_SLINGSHOT_COLOR;

      if ((slingshot.position - player.position).xz().magnitude() < SLINGSHOT_INTERACTION_TRIGGER_DISTANCE) {
        targetColor = HIGHLIGHT_SLINGSHOT_COLOR;

        if (input.didPressKey(Key::SPACE)) {
          state.velocity = Vec3f(0.f);

          interactWithSlingshot(context, state, slingshot);
        }
      }

      // @optimize we don't need to constantly lerp the color for
      // every slingshot; only ones affected by player proximity
      slingshot.color = Vec3f::lerp(slingshot.color.toVec3f(), targetColor, 10.f * dt);

      commit(slingshot);
    }
  }

  // Handle launching from slingshots
  {
    if (state.lastSlingshotInteractionTime != 0.f) {
      const float timeSinceLastSlingshotInteraction = time_since(state.lastSlingshotInteractionTime);
      auto* slingshot = get_object_by_record(state.activeSlingshotRecord);

      if (timeSinceLastSlingshotInteraction < SLINGSHOT_WIND_UP_DURATION_SECONDS) {
        // Wind-up
        if (slingshot != nullptr) {
          // @todo change the easing function to not require artificially reducing alpha
          float alpha = 0.3f * (1.f / SLINGSHOT_WIND_UP_DURATION_SECONDS) * timeSinceLastSlingshotInteraction;
          float angle = Gm_LerpCircularf(state.startingSlingshotAngle, state.targetSlingshotAngle, easeOutElastic(alpha), Gm_HALF_PI);

          slingshot->rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), angle);

          commit(*slingshot);
        }
      } else {
        // Launch
        CameraSystem::restoreOriginalCameraState(context, state);

        state.velocity = state.slingshotVelocity;
        state.lastSlingshotInteractionTime = 0.f;
        state.isOnSolidGround = false;

        if (slingshot != nullptr) {
          slingshot->rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), state.targetSlingshotAngle);

          commit(*slingshot);
        }
      }
    }
  }
}

internal void handleLanterns(GmContext* context, GameState& state, float dt) {
  for_moving_objects("lantern", {
    auto& basePosition = initial.position;
    float a = get_running_time() + basePosition.z / 200.f;
    float angle = 0.2f * sinf(a);

    float x = LANTERN_HORIZONTAL_DRIFT * sin(a);
    float y = LANTERN_VERTICAL_DRIFT * powf(Gm_Absf(x) / LANTERN_HORIZONTAL_DRIFT, 2);

    object.rotation = Quaternion::fromAxisAngle(Vec3f(0, 0, 1.f), angle); 
    object.position = basePosition + Vec3f(x, y, 0);

    commit(object);
  });
}

internal void handleWindmillWheels(GmContext* context, GameState& state, float dt) {
  for_moving_objects("windmill-wheel", {
    auto rotationAxis = initial.rotation.getDirection();
    // Rotate larger windmill wheels more slowly
    float scaleRatio = Gm_Clampf(initial.scale.magnitude() / 500.f, 0.f, 1.f);
    float rotationSpeedFactor = Gm_Lerpf(2.f, 0.2f, scaleRatio);
    float angle = rotationSpeedFactor * get_running_time();

    object.rotation = Quaternion::fromAxisAngle(rotationAxis, angle) * initial.rotation;

    commit(object);
  });
}

internal void handleAcFans(GmContext* context, GameState& state, float dt) {
  for_moving_objects("ac-fan", {
    auto rotationAxis = initial.rotation.getDirection();
    float angle = 3.f * get_running_time();

    object.rotation = Quaternion::fromAxisAngle(rotationAxis, angle) * initial.rotation;

    commit(object);
  });
}

internal void handleHotAirBalloons(GmContext* context, GameState& state, float dt) {
  for_moving_objects("hot-air-balloon", {
    float offset = object.position.x + object.position.z;
    float heightRate = 0.5f * get_running_time() + offset;
    float heightOscillation = object.scale.x / 10.f;
    float rotationRate = 0.7f * get_running_time() + offset;

    object.position = initial.position + Vec3f(0, heightOscillation, 0) * sinf(heightRate);
    object.rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), 0.05f * sinf(rotationRate));

    commit(object);
  });
}

internal void handleOcean(GmContext* context) {
  auto& camera = get_camera();
  auto& ocean = objects("ocean")[0];
  auto& floor = objects("ocean-floor")[0];

  ocean.position.x = camera.position.x;
  floor.position.x = camera.position.x;

  ocean.position.z = camera.position.z;
  floor.position.z = camera.position.z;

  commit(ocean);
  commit(floor);
}

void EntitySystem::initializeGameEntities(GmContext* context, GameState& state) {
  add_mesh("npc", 100, Mesh::Cube());
  add_mesh("slingshot", 100, Mesh::Model("./game/assets/slingshot.obj"));
 
  mesh("slingshot")->roughness = 0.9f;

  loadNpcData(context, state);
  loadEntityData(context, state);

  // Save initial reference copies of moving objects
  {
    for (auto& asset : World::meshAssets) {
      if (asset.moving) {
        for (auto& object : objects(asset.name)) {
          state.initialMovingObjects.push_back(object);
        }
      }
    }
  }
}

void EntitySystem::handleGameEntities(GmContext* context, GameState& state, float dt) {
  START_TIMING("handleGameEntities");

  handleNpcs(context, state);
  handleSlingshots(context, state, dt);
  handleLanterns(context, state, dt);
  handleWindmillWheels(context, state, dt);
  handleAcFans(context, state, dt);
  handleHotAirBalloons(context, state, dt);
  handleOcean(context);

  LOG_TIME();
}

bool EntitySystem::isInteractingWithEntity(GmContext* context, GameState& state) {
  return state.lastSlingshotInteractionTime != 0.f;
}