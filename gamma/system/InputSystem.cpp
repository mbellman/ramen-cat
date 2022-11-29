#include <map>

#include "system/InputSystem.h"
#include "system/console.h"

namespace Gamma {
  const static std::map<SDL_Keycode, Key> keyMap = {
    { SDLK_a, Key::A },
    { SDLK_b, Key::B },
    { SDLK_c, Key::C },
    { SDLK_d, Key::D },
    { SDLK_e, Key::E },
    { SDLK_f, Key::F },
    { SDLK_g, Key::G },
    { SDLK_h, Key::H },
    { SDLK_i, Key::I },
    { SDLK_j, Key::J },
    { SDLK_k, Key::K },
    { SDLK_l, Key::L },
    { SDLK_m, Key::M },
    { SDLK_n, Key::N },
    { SDLK_o, Key::O },
    { SDLK_p, Key::P },
    { SDLK_q, Key::Q },
    { SDLK_r, Key::R },
    { SDLK_s, Key::S },
    { SDLK_t, Key::T },
    { SDLK_u, Key::U },
    { SDLK_v, Key::V },
    { SDLK_w, Key::W },
    { SDLK_x, Key::X },
    { SDLK_y, Key::Y },
    { SDLK_z, Key::Z },
    { SDLK_0, Key::NUM_0 },
    { SDLK_1, Key::NUM_1 },
    { SDLK_2, Key::NUM_2 },
    { SDLK_3, Key::NUM_3 },
    { SDLK_4, Key::NUM_4 },
    { SDLK_5, Key::NUM_5 },
    { SDLK_6, Key::NUM_6 },
    { SDLK_7, Key::NUM_7 },
    { SDLK_8, Key::NUM_8 },
    { SDLK_9, Key::NUM_9 },
    { SDLK_LEFT, Key::ARROW_LEFT },
    { SDLK_RIGHT, Key::ARROW_RIGHT },
    { SDLK_UP, Key::ARROW_UP },
    { SDLK_DOWN, Key::ARROW_DOWN },
    { SDLK_SPACE, Key::SPACE },
    { SDLK_LSHIFT, Key::SHIFT },
    { SDLK_RSHIFT, Key::SHIFT },
    { SDLK_ESCAPE, Key::ESCAPE },
    { SDLK_RETURN, Key::ENTER },
    { SDLK_LCTRL, Key::CONTROL },
    { SDLK_BACKSPACE, Key::BACKSPACE },
    { SDLK_TAB, Key::TAB }
  };

  u64 InputSystem::getLastKeyDown() const {
    return lastKeyDown;
  }

  void InputSystem::handleEvent(const SDL_Event& event) {
    switch (event.type) {
      case SDL_KEYDOWN:
        handleKeyDown(event.key.keysym.sym);
        break;
      case SDL_KEYUP:
        handleKeyUp(event.key.keysym.sym);
        break;
      case SDL_MOUSEMOTION:
        handleMouseMotion(event.motion);
        break;
      case SDL_MOUSEBUTTONDOWN:
        handleMouseDown(event.button);
        break;
      case SDL_MOUSEWHEEL:
        handleMouseWheel(event.wheel);
        break;
      case SDL_TEXTINPUT:
        handleTextInput(event.text.text[0]);
        break;
    }
  }

  void InputSystem::handleKeyDown(const SDL_Keycode& code) {
    if (keyMap.find(code) != keyMap.end()) {
      Key key = keyMap.at(code);
      keyState |= (u64)key;
      lastKeyDown = (u64)key;

      signal("keydown", key);
    }
  }

  void InputSystem::handleKeyUp(const SDL_Keycode& code) {
    if (keyMap.find(code) != keyMap.end()) {
      Key key = keyMap.at(code);
      keyState &= ~(u64)key;

      signal("keyup", key);
    }
  }

  void InputSystem::handleMouseDown(const SDL_MouseButtonEvent& event) {
    MouseButtonEvent buttonEvent;

    buttonEvent.position.x = event.x;
    buttonEvent.position.y = event.y;

    signal("mousedown", buttonEvent);
  }

  void InputSystem::handleMouseMotion(const SDL_MouseMotionEvent& event) {
    MouseMoveEvent moveEvent;

    moveEvent.deltaX = event.xrel;
    moveEvent.deltaY = event.yrel;

    signal("mousemove", moveEvent);
  }

  void InputSystem::handleMouseWheel(const SDL_MouseWheelEvent& event) {
    MouseWheelEvent wheelEvent;

    wheelEvent.direction = event.y < 0
      ? MouseWheelEvent::DOWN
      : MouseWheelEvent::UP;

    signal("mousewheel", wheelEvent);
  }

  void InputSystem::handleTextInput(char character) {
    signal("input", character);
  }

  bool InputSystem::isKeyHeld(Key key) {
    return keyState & (u64)key;
  }
}