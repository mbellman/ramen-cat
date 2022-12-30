#include "entity_system.h"
#include "camera_system.h"
#include "ui_system.h"
#include "macros.h"

using namespace Gamma;

internal void loadNonPlayerCharacterData(GmContext* context, GameState& state) {
  // @todo eventually store as binary data
  auto npcsData = Gm_LoadFileContents("./game/data_npcs.txt");
  auto lines = Gm_SplitString(npcsData, "\n");

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

      while (lines[i][0] != '@' && i < lines.size()) {
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

internal void interactWithNPC(GmContext* context, GameState& state, NonPlayerCharacter& npc) {
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

  state.activeNPC = &npc;

  // @todo move to CameraSystem
  state.originalCameraState.camera3p = state.camera3p;
  state.originalCameraState.lookAtTarget = CameraSystem::getLookAtTargetPosition(context, state);

  state.sourceCameraState = state.originalCameraState;

  state.targetCameraState.lookAtTarget = targetLookAtPosition;
  state.targetCameraState.camera3p = targetCamera3p;

  state.useCameraOverride = true;
  state.cameraOverrideStartTime = state.frameStartTime;

  UISystem::queueDialogue(context, state, npc.dialogue);
}

internal void handleNPCs(GmContext* context, GameState& state) {
  auto& input = get_input();
  auto& player = get_player();

  // Handle interactions
  {
    if (input.didPressKey(Key::SPACE)) {
      if (state.activeNPC == nullptr) {
        for (auto& npc : state.npcs) {
          // @todo consider y distance as well
          auto distance = (npc.position - player.position).xz().magnitude();

          if (distance < 100.f) {
            interactWithNPC(context, state, npc);

            break;
          }
        }
      }
    }
  }

  // Handle ending conversions with NPCs
  {
    if (state.activeNPC != nullptr && UISystem::isDialogueQueueEmpty()) {
      state.activeNPC = nullptr;

      // @todo move to CameraSystem
      state.sourceCameraState.camera3p = state.camera3p;
      state.sourceCameraState.lookAtTarget = CameraSystem::getLookAtTargetPosition(context, state);
      
      state.targetCameraState = state.originalCameraState;

      state.cameraOverrideStartTime = state.frameStartTime;
      state.useCameraOverride = false;
    }
  }
}

void EntitySystem::initializeGameEntities(GmContext* context, GameState& state) {
  loadNonPlayerCharacterData(context, state);
}

void EntitySystem::handleGameEntities(GmContext* context, GameState& state, float dt) {
  START_TIMING("handleGameEntities");

  handleNPCs(context, state);

  LOG_TIME();
}