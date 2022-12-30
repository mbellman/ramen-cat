#pragma once

#include <string>
#include <vector>

#include "Gamma.h"

#include "game.h"

struct DialogueOptions {
  float duration = Gm_FLOAT_MAX;
  bool blocking = true;
};

namespace UISystem {
  void initializeUI(GmContext* context, GameState& state);
  void handleUI(GmContext* context, GameState& state, float dt);
  void showDialogue(GmContext* context, GameState& state, const std::string& text, const DialogueOptions& options = {});
  void queueDialogue(GmContext* context, GameState& state, const std::vector<std::string>& queue);
  bool isDialogueQueueEmpty();
  bool hasBlockingDialogue();
}