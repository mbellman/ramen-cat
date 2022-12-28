#include "SDL_image.h"

#include "ui_system.h"

using namespace Gamma;

// @temporary
static SDL_Surface* cat = nullptr;

void UISystem::initializeUI(GmContext* context, GameState& state) {
  // @temporary
  cat = IMG_Load("./game/ui/cat.png");

  // @todo load textures
}

void UISystem::handleUI(GmContext* context, GameState& state, float dt) {
  auto& renderer = *context->renderer;

  render_image(cat, 500, 100, 150, 150);
}