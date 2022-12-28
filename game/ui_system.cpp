// @todo add first-class engine features for images/shapes/text
#include "SDL_image.h"
#include "SDL_ttf.h"

#include "ui_system.h"

using namespace Gamma;

// @temporary
static SDL_Surface* cat = nullptr;
static SDL_Surface* textBox = nullptr;
static TTF_Font* font = nullptr;

void UISystem::initializeUI(GmContext* context, GameState& state) {
  // @temporary
  cat = IMG_Load("./game/ui/cat.png");
  textBox = SDL_CreateRGBSurface(0, 500, 150, 24, 0, 0, 0, 0);
  font = TTF_OpenFont("./fonts/OpenSans-Regular.ttf", 32);
  SDL_FillRect(textBox, 0, 0xFF00FF);

  // @todo load UI textures
}

void UISystem::handleUI(GmContext* context, GameState& state, float dt) {
  // @temporary
  render_image(cat, 500, 100, 150, 150);
  render_image(textBox, 300, 400, textBox->w, textBox->h);
  render_text(font, "Hello there!", 200, 300);
}