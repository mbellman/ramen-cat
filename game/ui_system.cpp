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
  bool done = false;
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

internal void showNextQueuedDialogue(GmContext* context, GameState& state) {
  dialogue.active = true;
  dialogue.done = false;
  dialogue.blocking = true;

  dialogue.startTime = get_scene_time();
  dialogue.lastCharacterTime = get_scene_time();
  dialogue.duration = Gm_FLOAT_MAX;
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