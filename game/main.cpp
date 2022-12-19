#include "Gamma.h"

#include "game.h"

int main(int argc, char* argv[]) {
  using namespace Gamma;

  auto* context = Gm_CreateContext();
  GameState state;

  Gm_OpenWindow(context, "Video Game!", { 1200, 675 });
  Gm_SetRenderMode(context, GmRenderMode::OPENGL);

  initializeGame(context, state);

  while (!context->window.closed) {
    float dt = Gm_GetDeltaTime(context);

    Gm_HandleFrameStart(context);

    // @todo handle this within the engine (?)
    if (dt > 0.1f) {
      dt = 0.1f;
    }

    updateGame(context, state, dt);

    Gm_RenderScene(context);
    Gm_HandleFrameEnd(context);
  }

  Gm_DestroyContext(context);

  return 0;
}