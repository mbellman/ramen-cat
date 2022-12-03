#include "Gamma.h"

#include "game.h"

int main(int argc, char* argv[]) {
  using namespace Gamma;

  auto* context = Gm_CreateContext();
  GameState state;

  Gm_OpenWindow(context, "Gamma Project", { 1200, 675 });
  Gm_SetRenderMode(context, GmRenderMode::OPENGL);

  initializeGame(context, state);

  while (!context->window.closed) {
    float dt = Gm_GetDeltaTime(context);

    Gm_LogFrameStart(context);
    Gm_HandleEvents(context);

    updateGame(context, state, dt);

    Gm_RenderScene(context);
    Gm_LogFrameEnd(context);
  }

  Gm_DestroyContext(context);

  return 0;
}