#include "entity_system.h"
#include "camera_system.h"
#include "ui_system.h"
#include "macros.h"

using namespace Gamma;

const Vec3f DEFAULT_SLINGSHOT_COLOR = Vec3f(0.2f);

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
    object.scale = Vec3f(20.f, 40.f, 20.f);

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

internal void interactWithSlingshot(GmContext* context, GameState& state, Slingshot& slingshot) {
  // @todo
}

internal void handleSlingshots(GmContext* context, GameState& state, float dt) {
  auto& player = get_player();

  // Handle proximity to slingshots
  {
    for (auto& slingshot : objects("slingshot")) {
      Vec3f targetColor = DEFAULT_SLINGSHOT_COLOR;

      if ((slingshot.position - player.position).xz().magnitude() < 100.f) {
        targetColor = Vec3f(1.f);
      }

      // @optimize we don't need to constantly lerp the color for
      // every slingshot; only ones affected by player proximity
      slingshot.color = Vec3f::lerp(slingshot.color.toVec3f(), targetColor, 10.f * dt);

      commit(slingshot);
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
  add_mesh("slingshot", 100, Mesh::Cube());

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