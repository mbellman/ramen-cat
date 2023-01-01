#include "entity_system.h"
#include "camera_system.h"
#include "ui_system.h"
#include "macros.h"

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
    object.scale = Vec3f(20.f, 70.f, 20.f);

    commit(object);
  }
}

internal void interactWithNpc(GmContext* context, GameState& state, NonPlayerCharacter& npc) {
  auto& player = get_player();
  Vec3f npcFacePosition = npc.position + Vec3f(0, 30.f, 0);
  Vec3f npcToPlayer = (player.position - npc.position);

  Vec3f targetLookAtPosition = Vec3f(
    (npcFacePosition.x + player.position.x) / 2.f,
    npcFacePosition.y,
    (npcFacePosition.z + player.position.z) / 2.f
  );

  ThirdPersonCamera targetCamera3p = {
    .azimuth = atan2f(npcToPlayer.z, npcToPlayer.x) - Gm_TAU / 8.f,
    .altitude = 0.f,
    .radius = 150.f
  };

  state.activeNpc = &npc;

  CameraSystem::setCameraStateOverride(context, state, {
    .camera3p = targetCamera3p,
    .lookAtTarget = targetLookAtPosition
  });

  UISystem::queueDialogue(context, state, npc.dialogue);
}

internal void handleNpcs(GmContext* context, GameState& state) {
  auto& input = get_input();
  auto& player = get_player();

  // Handle interactions
  {
    if (input.didPressKey(Key::SPACE)) {
      if (state.activeNpc == nullptr) {
        for (auto& npc : state.npcs) {
          // @todo consider y distance as well
          auto xzDistance = (npc.position - player.position).xz().magnitude();

          if (xzDistance < 100.f) {
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

void EntitySystem::initializeGameEntities(GmContext* context, GameState& state) {
  loadNpcData(context, state);
}

void EntitySystem::handleGameEntities(GmContext* context, GameState& state, float dt) {
  START_TIMING("handleGameEntities");

  handleNpcs(context, state);

  LOG_TIME();
}