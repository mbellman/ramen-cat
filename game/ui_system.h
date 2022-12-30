#pragma once

#include <string>
#include <vector>

#include "Gamma.h"

#include "game.h"

struct DialogueOptions {

};

namespace UISystem {
  void initializeUI(GmContext* context, GameState& state);
  void handleUI(GmContext* context, GameState& state, float dt);
  void showDialogue(GmContext* context, GameState& state, const std::string& text, float duration = Gm_FLOAT_MAX);
  void queueDialogue(GmContext* context, GameState& state, const std::vector<std::string>& queue);
  void dismissDialogue();
  bool isDialogueQueueEmpty();
  bool hasBlockingDialogue();
}