#include "system/Commander.h"
#include "system/console.h"
#include "system/flags.h"

namespace Gamma {
  struct Command {
    const char* keyword;
    const char* displayName;
    GammaFlags flag;
  };

  static Command commands[] = {
    { "reflect", "Reflections", GammaFlags::RENDER_REFLECTIONS },
    { "refract", "Refractive geometry", GammaFlags::RENDER_REFRACTIVE_GEOMETRY },
    { "shadow", "Shadows", GammaFlags::RENDER_SHADOWS },
    { "rro", "Reflections of refractive geometry", GammaFlags::RENDER_REFRACTIVE_GEOMETRY_WITHIN_REFLECTIONS },
    { "ao", "Ambient occlusion", GammaFlags::RENDER_AMBIENT_OCCLUSION },
    { "gi", "Global illumination", GammaFlags::RENDER_GLOBAL_ILLUMINATION },
    { "skylight", "Indirect sky light", GammaFlags::RENDER_INDIRECT_SKY_LIGHT },
    { "dev buffers", "Dev buffers", GammaFlags::RENDER_DEV_BUFFERS },
    { "wireframe", "Wireframe mode", GammaFlags::WIREFRAME_MODE },
    { "denoising", "Denoising", GammaFlags::ENABLE_DENOISING }
  };

  Commander::Commander() {
    input.on<Key>("keydown", [&](Key key) {
      if (key == Key::C && input.isKeyHeld(Key::CONTROL) && isEnteringCommand) {
        resetCurrentCommand();
      } else if (key == Key::TAB) {
        if (isEnteringCommand) {
          resetCurrentCommand();
        } else {
          isEnteringCommand = true;
        }
      } else if (key == Key::BACKSPACE && isEnteringCommand && currentCommand.length() > 0) {
        currentCommand.pop_back();
      } else if (key == Key::ESCAPE && isEnteringCommand) {
        resetCurrentCommand();
      }
    });

    input.on<Key>("keyup", [&](Key key) {
      if (key == Key::ENTER && isEnteringCommand) {
        processCurrentCommand();
      }
    });

    input.on<char>("input", [&](char character) {
      if (isEnteringCommand) {
        currentCommand += character;
      }
    });
  }

  bool Commander::currentCommandIncludes(std::string match) {
    return currentCommand.find(match) != std::string::npos;
  }

  const std::string& Commander::getCommand() const {
    return currentCommand;
  }

  bool Commander::isOpen() const {
    return isEnteringCommand;
  }

  void Commander::processCurrentCommand() {
    constexpr static u32 totalCommands = sizeof(commands) / sizeof(Command);
    auto command = std::string(currentCommand);

    if (currentCommandIncludes("enable")) {
      for (u32 i = 0; i < totalCommands; i++) {
        auto& command = commands[i];

        if (currentCommandIncludes(command.keyword)) {
          Gm_EnableFlags(command.flag);

          Console::log("[Gamma]", command.displayName, "enabled");
        }
      }
    } else if (currentCommandIncludes("disable")) {
      for (u32 i = 0; i < totalCommands; i++) {
        auto& command = commands[i];

        if (currentCommandIncludes(command.keyword)) {
          Gm_DisableFlags(command.flag);

          Console::log("[Gamma]", command.displayName, "disabled");
        }
      }
    }

    signal("command", command);

    resetCurrentCommand();
  }

  void Commander::resetCurrentCommand() {
    currentCommand = "";
    isEnteringCommand = false;
  }
}