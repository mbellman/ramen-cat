#include "Gamma.h"

#include "game.h"

// @todo move to game constants
constexpr static float MAX_DT = 1.f / 30.f;

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
    if (dt > MAX_DT) {
      dt = MAX_DT;
    }

    updateGame(context, state, dt);

    Gm_RenderScene(context);
    Gm_HandleFrameEnd(context);
  }

  Gm_DestroyContext(context);

  return 0;
}