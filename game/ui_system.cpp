#include <string>

// @todo add first-class engine features for images/shapes/text
#include "SDL_image.h"
#include "SDL_ttf.h"

#include "ui_system.h"
#include "easing.h"
#include "macros.h"

using namespace Gamma;

constexpr static float DIALOGUE_CHARACTER_DURATION = 0.05f;

// @temporary
static SDL_Surface* collectableHud = nullptr;

static SDL_Surface* onigiri = nullptr;
static SDL_Surface* demonOnigiri = nullptr;

static SDL_Surface* nitamago = nullptr;
static SDL_Surface* demonNitamago = nullptr;

// @temporary
static SDL_Surface* dialoguePane = nullptr;
static TTF_Font* dialogueFont = nullptr;

struct Dialogue {
  std::vector<std::string> queue;
  float startTime = 0.f;
  float lastCharacterTime = 0.f;
  float duration = 0.f;
  bool active = false;
  bool done = false;
  bool blocking = true;
} dialogue;

/**
 * Takes a substring of a given string, up to a given number of
 * explicit character symbols (excluding spaces of tabs).
 */
internal std::string substringWithCharacterCount(const std::string& value, u32 characterCount) {
  u32 usedCharacters = 0;
  std::string substring;
  
  for (u32 i = 0; i < value.size(); i++) {
    auto character = value.at(i);

    substring += character;

    if (character == ' ' || character == '\t') {
      continue;
    }

    usedCharacters++;

    if (usedCharacters > characterCount) {
      break;
    }
  }

  return substring;
}

internal void showNextQueuedDialogue(GmContext* context, GameState& state) {
  dialogue.active = true;
  dialogue.done = false;
  dialogue.blocking = true;

  dialogue.startTime = get_scene_time();
  dialogue.lastCharacterTime = get_scene_time();
  dialogue.duration = Gm_FLOAT_MAX;
}

// @todo clean up
internal void handleHud(GmContext* context, GameState& state, float dt) {
  auto& frame = context->window.size;

  render_image(collectableHud, 50, frame.height - collectableHud->h - 40, collectableHud->w, collectableHud->h);

  // Onigiri
  {
    float alpha = state.inventory.demonOnigiri.count > 0 ? time_since(state.inventory.demonOnigiri.firstCollectionTime) : 0.f;
    alpha *= 2.f;
    if (alpha > 1.f) alpha = 1.f;
    alpha = easeOutElastic(alpha);

    int xOffset = int(14.f * alpha);

    render_image(demonOnigiri, 65 + xOffset, frame.height - demonOnigiri->h - 50, demonOnigiri->w, demonOnigiri->h);
    render_image(onigiri, 65 - xOffset, frame.height - onigiri->h - 50, onigiri->w, onigiri->h);
  }

  // Nitamago
  {
    float alpha = state.inventory.demonNitamago.count > 0 ? time_since(state.inventory.demonNitamago.firstCollectionTime) : 0.f;
    alpha *= 2.f;
    if (alpha > 1.f) alpha = 1.f;
    alpha = easeOutElastic(alpha);

    int xOffset = int(14.f * alpha);

    render_image(demonNitamago, 180 + xOffset, frame.height - demonNitamago->h - 50, demonNitamago->w, demonNitamago->h);
    render_image(nitamago, 180 - xOffset, frame.height - nitamago->h - 50, nitamago->w, nitamago->h);
  }
}

internal void handleDialogue(GmContext* context, GameState& state) {
  if (dialogue.queue.size() == 0 || !dialogue.active) {
    return;
  }

  auto dialogueText = dialogue.queue[0];
  bool hasStartedPrintingDialogue = false;
  bool hasPrintedFullDialogue = false;

  // Render active dialogue box
  {
    auto timeSinceDialogueStart = time_since(dialogue.startTime);
    auto timeSinceLastCharacter = time_since(dialogue.lastCharacterTime);

    Region<u32> pane = {
      .x = u32(context->window.size.width / 4.f),
      .y = u32(context->window.size.height - 200),
      .width = u32(context->window.size.width / 2.f),
      .height = 150
    };

    render_image(dialoguePane, pane.x, pane.y, pane.width, pane.height);

    if (timeSinceLastCharacter >= dialogue.duration) {
      dialogue.done = true;
    }

    if (dialogue.done) {
      dialogue.queue.erase(dialogue.queue.begin());

      if (dialogue.queue.size() > 0) {
        showNextQueuedDialogue(context, state);
      } else {
        dialogue.active = false;
        dialogue.blocking = false;
      }
    } else {
      auto runningTextLength = u32(timeSinceDialogueStart / DIALOGUE_CHARACTER_DURATION);

      if (runningTextLength > 0) {
        auto runningText = substringWithCharacterCount(dialogueText, runningTextLength);

        hasStartedPrintingDialogue = true;
        hasPrintedFullDialogue = runningText == dialogueText;

        if (!hasPrintedFullDialogue) {
          dialogue.lastCharacterTime = get_scene_time();
        }

        render_text(dialogueFont, runningText, pane.x + 20, pane.y + 20);
      }
    }
  }

  // Handle inputs
  {
    auto& input = get_input();

    if (
      input.didPressKey(Key::SPACE) &&
      dialogue.blocking &&
      hasStartedPrintingDialogue
    ) {
      if (hasPrintedFullDialogue) {
        // Dismiss the dialogue
        dialogue.done = true;
      } else {
        // Fudge the dialogue start time to cause the
        // full text to print on the next frame
        dialogue.startTime = get_scene_time() - dialogueText.size() * DIALOGUE_CHARACTER_DURATION;
      }
    }
  }
}

void UISystem::initializeUI(GmContext* context, GameState& state) {
  dialoguePane = Gm_CreateRectangle(500, 150, 0x222244AA);
  dialogueFont = TTF_OpenFont("./fonts/OpenSans-Regular.ttf", 32);

  // @temporary
  collectableHud = IMG_Load("./game/assets/hud/collectable-hud.png");

  onigiri = IMG_Load("./game/assets/hud/onigiri.png");
  demonOnigiri = IMG_Load("./game/assets/hud/demon-onigiri.png");

  nitamago = IMG_Load("./game/assets/hud/nitamago.png");
  demonNitamago = IMG_Load("./game/assets/hud/demon-nitamago.png");
}

void UISystem::handleUI(GmContext* context, GameState& state, float dt) {  
  START_TIMING("handleUI");

  // handleHud(context, state, dt);
  handleDialogue(context, state);

  LOG_TIME();
}

void UISystem::showDialogue(GmContext* context, GameState& state, const std::string& text, const DialogueOptions& options) {
  dialogue.active = true;
  dialogue.done = false;
  dialogue.blocking = options.blocking;
  dialogue.queue = { text };
  dialogue.startTime = get_scene_time();
  dialogue.lastCharacterTime = get_scene_time();
  dialogue.duration = options.duration;
}

void UISystem::queueDialogue(GmContext* context, GameState& state, const std::vector<std::string>& queue) {
  dialogue.queue = queue;

  showNextQueuedDialogue(context, state);
}

bool UISystem::isDialogueQueueEmpty() {
  return dialogue.queue.size() == 0;
}

bool UISystem::hasBlockingDialogue() {
  return dialogue.active && dialogue.blocking;
}