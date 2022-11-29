#include "system/console.h"

#include "SDL.h"

namespace Gamma {
  std::stringstream Console::output;
  ConsoleMessage* Console::firstMessage = nullptr;
  ConsoleMessage* Console::lastMessage = nullptr;
  u32 Console::messageCounter = 0;

  void Console::clearMessages() {
    // @todo
  }

  void Console::storeMessage(const std::string message) {
    auto* consoleMessage = new ConsoleMessage();

    // @todo use system time or something we can
    // determine HH:MM:SS from
    consoleMessage->time = SDL_GetTicks();
    consoleMessage->text = message;
    consoleMessage->next = nullptr;

    if (firstMessage == nullptr) {
      firstMessage = consoleMessage;
    } else {
      lastMessage->next = consoleMessage;
    }

    lastMessage = consoleMessage;

    if (++messageCounter > 5) {
      auto* newFirstMessage = firstMessage->next;

      delete firstMessage;

      firstMessage = newFirstMessage;
    }
  }
}