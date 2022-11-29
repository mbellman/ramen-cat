#pragma once

#include <string>

#include "system/InputSystem.h"
#include "system/Signaler.h"
#include "system/traits.h"

#include "SDL_events.h"

namespace Gamma {
  class Commander : public Signaler {
  public:
    InputSystem input;

    Commander();

    const std::string& getCommand() const;
    bool isOpen() const;

  private:
    bool isEnteringCommand = false;
    std::string currentCommand = "";

    bool currentCommandIncludes(std::string match);
    void processCurrentCommand();
    void resetCurrentCommand();
  };
}