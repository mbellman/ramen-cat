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
  std::string text;
  float startTime = 0.f;
  float lastCharacterTime = 0.f;
  float duration = 0.f;
  bool active = false;
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

internal std::string substringWithCharacterCount(const std::string& string, u32 characterCount) {
  u32 usedCharacters = 0;
  std::string substring;
  
  for (u32 i = 0; i < dialogue.text.size(); i++) {
    auto character = dialogue.text.at(i);

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

void UISystem::initializeUI(GmContext* context, GameState& state) {
  dialoguePane = createRectangle(500, 150, 0x222244AA);
  dialogueFont = TTF_OpenFont("./fonts/OpenSans-Regular.ttf", 32);

  // @temporary
  cat = IMG_Load("./game/ui/cat.png");
}

void UISystem::handleUI(GmContext* context, GameState& state, float dt) {
  // Render active dialogue
  {
    auto timeSinceDialogueStart = get_running_time() - dialogue.startTime;
    auto timeSinceLastCharacter = get_running_time() - dialogue.lastCharacterTime;

    if (dialogue.active && timeSinceLastCharacter < dialogue.duration) {
      auto runningDialogueLength = u32(timeSinceDialogueStart / DIALOGUE_CHARACTER_DURATION);
      auto runningDialogue = substringWithCharacterCount(dialogue.text, runningDialogueLength);

      if (runningDialogue.size() > 0) {
        if (runningDialogue != dialogue.text) {
          dialogue.lastCharacterTime = get_running_time();
        }

        u32 x = u32(context->window.size.width / 4.f);
        u32 y = u32(context->window.size.height - 200);
        u32 w = u32(context->window.size.width / 2.f);
        u32 h = 150;

        render_image(dialoguePane, x, y, w, h);
        render_text(dialogueFont, runningDialogue, x + 20, y + 20);
      }
    }
  }
}

void UISystem::showDialogue(GmContext* context, GameState& state, const std::string& text, float duration) {
  dialogue.active = true;
  dialogue.text = text;
  dialogue.startTime = dialogue.lastCharacterTime = get_running_time();
  dialogue.duration = duration;
}

void UISystem::dismissDialogue() {
  dialogue.active = false;
}