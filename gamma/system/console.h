#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <tuple>

#include "system/type_aliases.h"

namespace Gamma {
  struct ConsoleMessage {
    u32 time;
    bool warning = false;
    std::string text;
    ConsoleMessage* next = nullptr;
  };

  class Console {
  public:
    template<typename ...Args>
    static void log(Args&& ...args) {
      // @todo output time with each console message
      out(false, args...);
      std::cout << "\n";
    }

    template<typename ...Args>
    static void warn(Args&& ...args) {
      out(true, args...);
      std::cout << "\n";
    }

    static void clearMessages();

    static const ConsoleMessage* getFirstMessage() {
      return Console::firstMessage;
    }

  private:
    static std::stringstream output;
    static ConsoleMessage* firstMessage;
    static ConsoleMessage* lastMessage;
    static u32 messageCounter;

    template<typename Arg, typename ...Args>
    static void out(bool warning, Arg& arg, Args&& ...args) {
      output << arg;

      if constexpr (sizeof...(args) > 0) {
        output << " ";
        out(warning, args...);
      } else {
        std::string outputString = output.str();
        std::cout << outputString;

        Console::storeMessage(outputString, warning);

        // Reset output stream
        output.str(std::string());
        output.clear();
      }
    }

    static void storeMessage(const std::string message, bool warning = false);
  };
}