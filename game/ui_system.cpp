#include <string>

// @todo add first-class engine features for images/shapes/text
#include "SDL_image.h"
#include "SDL_ttf.h"

#include "ui_system.h"
#include "macros.h"

using namespace Gamma;

constexpr static float DIALOGUE_CHARACTER_DURATION = 0.05f;

// @temporary
static SDL_Surface* cat = nullptr;
static SDL_Surface* dialoguePane = nullptr;
static TTF_Font* dialogueFont = nullptr;

struct Dialogue {
  std::vector<std::string> queue;
  float startTime = 0.f;
  float lastCharacterTime = 0.f;
  float duration = 0.f;
  bool active = false;
  bool finished = false;
  bool blocking = true;
} dialogue;


internal SDL_Surface* createRectangle(u32 width, u32 height, u32 color) {
  #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    auto* surface = SDL_CreateRGBSurface(0, width, height, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
  #else
    auto* surface = SDL_CreateRGBSurface(0, width, height, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
  #endif

  auto rgba = SDL_MapRGBA(surface->format, (color & 0xFF000000) >> 24, (color & 0x00FF0000) >> 16, (color & 0x0000FF00) >> 8, color & 0x000000FF);

  SDL_FillRect(surface, 0, rgba);

  return surface;
}

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

internal void showNextQueuedDialogue(GmContext* context, GameState& state, float duration = Gm_FLOAT_MAX) {
  dialogue.active = true;
  dialogue.finished = false;
  dialogue.blocking = true; // @todo make configurable

  dialogue.startTime = state.frameStartTime;
  dialogue.lastCharacterTime = state.frameStartTime;
  dialogue.duration = duration;
}

internal void handleDialogue(GmContext* context, GameState& state) {
  if (dialogue.queue.size() == 0 || !dialogue.active) {
    return;
  }

  bool hasStartedPrintingDialogue = false;
  bool isStillPrintingDialogue = false;

  // Render active dialogue
  {
    auto dialogueText = dialogue.queue[0];
    auto timeSinceDialogueStart = get_running_time() - dialogue.startTime;
    auto timeSinceLastCharacter = get_running_time() - dialogue.lastCharacterTime;

    if (timeSinceLastCharacter >= dialogue.duration) {
      dialogue.finished = true;
    }

    if (dialogue.finished) {
      dialogue.queue.erase(dialogue.queue.begin());

      if (dialogue.queue.size() > 0) {
        showNextQueuedDialogue(context, state);

        isStillPrintingDialogue = true;
      } else {
        dialogue.active = false;
        dialogue.blocking = false;
      }
    } else {
      auto runningTextLength = u32(timeSinceDialogueStart / DIALOGUE_CHARACTER_DURATION);

      if (runningTextLength > 0) {
        hasStartedPrintingDialogue = true;

        auto runningText = substringWithCharacterCount(dialogueText, runningTextLength);

        if (runningText != dialogueText) {
          dialogue.lastCharacterTime = get_running_time();
          isStillPrintingDialogue = true;
        }

        u32 x = u32(context->window.size.width / 4.f);
        u32 y = u32(context->window.size.height - 200);
        u32 w = u32(context->window.size.width / 2.f);
        u32 h = 150;

        render_image(dialoguePane, x, y, w, h);
        render_text(dialogueFont, runningText, x + 20, y + 20);
      } else {
        isStillPrintingDialogue = true;
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
      if (isStillPrintingDialogue) {
        // @todo description
        dialogue.startTime = get_running_time() - dialogue.queue[0].size() * DIALOGUE_CHARACTER_DURATION;
      } else {
        // @todo description
        dialogue.finished = true;
      }
    }
  }
}

void UISystem::initializeUI(GmContext* context, GameState& state) {
  dialoguePane = createRectangle(500, 150, 0x222244AA);
  dialogueFont = TTF_OpenFont("./fonts/OpenSans-Regular.ttf", 32);

  // @temporary
  cat = IMG_Load("./game/ui/cat.png");
}

void UISystem::handleUI(GmContext* context, GameState& state, float dt) {  
  START_TIMING("handleUI");

  handleDialogue(context, state);

  LOG_TIME();
}

void UISystem::showDialogue(GmContext* context, GameState& state, const std::string& text, float duration) {
  dialogue.active = true;
  dialogue.finished = false;
  dialogue.blocking = true;
  dialogue.queue = { text };
  dialogue.startTime = state.frameStartTime;
  dialogue.lastCharacterTime = state.frameStartTime;
  dialogue.duration = duration;
}

void UISystem::queueDialogue(GmContext* context, GameState& state, const std::vector<std::string>& queue) {
  dialogue.queue = queue;

  showNextQueuedDialogue(context, state);
}

void UISystem::dismissDialogue() {
  dialogue.finished = true;
}

bool UISystem::isDialogueQueueEmpty() {
  return dialogue.queue.size() == 0;
}

bool UISystem::hasBlockingDialogue() {
  return dialogue.active && dialogue.blocking;
}