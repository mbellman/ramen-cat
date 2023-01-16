#include "entity_system.h"
#include "camera_system.h"
#include "ui_system.h"
#include "macros.h"

using namespace Gamma;

const Vec3f DEFAULT_SLINGSHOT_COLOR = Vec3f(0.2f);
const Vec3f HIGHLIGHT_SLINGSHOT_COLOR = Vec3f(1.f);

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
    object.scale = Vec3f(20.f, 70.f, 20.f);

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
    } else {
      Vec3f position = Gm_ParseVec3f(line);

      state.slingshots.push_back({
        .position = position
      });
    }
  }

  // @temporary
  for (auto& slingshot : state.slingshots) {
    auto& object = create_object_from("slingshot");

    object.position = slingshot.position;
    object.color = DEFAULT_SLINGSHOT_COLOR;
    object.scale = Vec3f(60.f);

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
      .radius = 150.f
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
    // @todo define these constants elsewhere and use them in player/NPC generation
    const float NPC_RADIUS = 20.f;
    const float NPC_HEIGHT = 70.f;
    const float NPC_INTERACTION_DISTANCE = 120.f;

    if (input.didPressKey(Key::SPACE)) {
      if (state.activeNpc == nullptr) {
        for (auto& npc : state.npcs) {
          float xzDistance = (npc.position - player.position).xz().magnitude();

          if (
            xzDistance < NPC_INTERACTION_DISTANCE &&
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

// @todo cleanup/clarity
internal void interactWithSlingshot(GmContext* context, GameState& state, Object& slingshot) {
  auto& player = get_player();

  Vec3f slingshotToPlayer = player.position - slingshot.position;
  float slingshotToPlayerAngle = atan2f(slingshotToPlayer.z, slingshotToPlayer.x);
  Vec3f playerDirection = slingshotToPlayer.xz().unit();

  // @todo make configurable
  float xVelocity = 350.f * playerDirection.x * -1.f;
  float zVelocity = 350.f * playerDirection.z * -1.f;
  Vec3f slingshotVelocity = Vec3f(xVelocity, 1500.f, zVelocity);
  Vec3f slingshotDirection = slingshot.rotation.getDirection();

  state.lastSlingshotInteractionTime = state.frameStartTime;
  state.startingSlingshotAngle = Gm_Modf(-atan2f(slingshotDirection.z, slingshotDirection.x) + Gm_HALF_PI, Gm_TAU);
  state.targetSlingshotAngle = Gm_Modf(-slingshotToPlayerAngle + Gm_HALF_PI, Gm_TAU);
  state.activeSlingshotRecord = slingshot._record;
  state.slingshotVelocity = slingshotVelocity;
  state.velocity = Vec3f(0.f);

  // Control camera override behavior
  {
    CameraSystem::setTargetCameraState(context, state, {
      .camera3p = {
        .azimuth = slingshotToPlayerAngle - Gm_PI / 16.f,
        .altitude = 0.f,
        .radius = 150.f
      },
      .lookAtTarget = slingshot.position
    });

    // Have the restored camera state center behind/above
    // the player, once launched from the slingshot
    state.originalCameraState.camera3p.azimuth = Gm_Modf(atan2f(slingshotVelocity.z, slingshotVelocity.x) - Gm_PI, Gm_TAU);
    state.originalCameraState.camera3p.altitude = Gm_HALF_PI * 0.8f;
    state.originalCameraState.camera3p.radius = (state.cameraMode == CameraMode::NORMAL ? 300.f : 600.f) + 200.f * (state.originalCameraState.camera3p.altitude / Gm_HALF_PI);
  }
}

// @todo create easing helpers
internal float easeOut(float t) {
  return 1.f - powf(1.f - t, 5);
}

internal void handleSlingshots(GmContext* context, GameState& state, float dt) {
  auto& player = get_player();
  auto& input = get_input();

  // Handle proximity to slingshots
  {
    for (auto& slingshot : objects("slingshot")) {
      Vec3f targetColor = DEFAULT_SLINGSHOT_COLOR;

      if ((slingshot.position - player.position).xz().magnitude() < 100.f) {
        targetColor = HIGHLIGHT_SLINGSHOT_COLOR;

        if (input.didPressKey(Key::SPACE) && state.velocity.y == 0.f) {
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

      if (timeSinceLastSlingshotInteraction < 0.5f) {
        // Wind-up
        auto* slingshot = Gm_GetObjectByRecord(context, state.activeSlingshotRecord);

        if (slingshot != nullptr) {
          float alpha = 2.f * timeSinceLastSlingshotInteraction;
          float angle = Gm_LerpCircularf(state.startingSlingshotAngle, state.targetSlingshotAngle, easeOut(alpha), Gm_HALF_PI);

          slingshot->rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), angle);

          commit(*slingshot);
        }
      } else {
        // Launch
        CameraSystem::restoreOriginalCameraState(context, state);

        state.lastSlingshotInteractionTime = 0.f;
        state.velocity = state.slingshotVelocity;
      }
    }
  }
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
}

void EntitySystem::handleGameEntities(GmContext* context, GameState& state, float dt) {
  START_TIMING("handleGameEntities");

  handleNpcs(context, state);
  handleSlingshots(context, state, dt);
  handleOcean(context);

  LOG_TIME();
}

bool EntitySystem::isInteractingWithEntity(GmContext* context, GameState& state) {
  return state.lastSlingshotInteractionTime != 0.f;
}