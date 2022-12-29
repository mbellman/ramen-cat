#include "entity_system.h"
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

void EntitySystem::initializeGameEntities(GmContext* context, GameState& state) {
  loadNonPlayerCharacterData(context, state);
}

void EntitySystem::handleGameEntities(GmContext* context, GameState& state, float dt) {
  auto& input = get_input();
  auto& player = get_player();

  // Handle interactions
  {
    if (input.didPressKey(Key::SPACE)) {
      for (auto& npc : state.npcs) {
        auto distance = (npc.position - player.position).xz().magnitude();

        if (distance < 100.f) {
          // Suppress jumps when interacting with NPCs
          state.canJumpThisFrame = false;

          if (!UISystem::isDialogueStillPrinting(context)) {
            UISystem::showDialogue(context, state, npc.dialogue[0], 5.f);
          }

          break;
        }
      }
    }
  }
}