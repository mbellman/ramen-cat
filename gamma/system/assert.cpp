#include "SDL.h"
#include "system/assert.h"

namespace Gamma {
  void assert(bool condition, std::string message) {
    if (!condition) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", message.c_str(), 0);
      exit(0);
    }
  }
}