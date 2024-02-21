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

  const static std::map<Uint8, Key> controllerButtonMap = {
    { SDL_CONTROLLER_BUTTON_A, Key::CONTROLLER_A },
    { SDL_CONTROLLER_BUTTON_B, Key::CONTROLLER_B },
    { SDL_CONTROLLER_BUTTON_X, Key::CONTROLLER_X },
    { SDL_CONTROLLER_BUTTON_Y, Key::CONTROLLER_Y }
  };

  bool InputSystem::didClickMouse() const {
    return didClickMouseThisFrame;
  }

  bool InputSystem::didMoveMouse() const {
    return mouseDelta.x != 0 || mouseDelta.y != 0;
  }

  bool InputSystem::didMoveMouseWheel() const {
    return didMoveMouseWheelThisFrame;
  }

  bool InputSystem::didPressKey(Key key) const {
    return pressedKeyState & (u64)key;
  }

  bool InputSystem::didReleaseKey(Key key) const {
    return releasedKeyState & (u64)key;
  }

  bool InputSystem::didReleaseMouse() const {
    return didReleaseMouseThisFrame;
  }

  bool InputSystem::didRightClickMouse() const {
    return didRightClickThisFrame;
  }

  u64 InputSystem::getLastKeyDown() const {
    return lastKeyDown;
  }

  const Vec2f& InputSystem::getLeftStick() const {
    return leftStick;
  }

  const Point<int>& InputSystem::getMouseDelta() const {
    return mouseDelta;
  }

  const MouseWheelEvent::Direction InputSystem::getMouseWheelDirection() const {
    return mousewheelDirection;
  }

  const Vec2f& InputSystem::getRightStick() const {
    return rightStick;
  }

  void InputSystem::handleEvent(const SDL_Event& event) {
    switch (event.type) {
      case SDL_CONTROLLERAXISMOTION: {
        // @todo cleanup
        const float DEAD_ZONE = 0.2f;
        float v = float(event.jaxis.value) / 32767.f;
        bool isDeadZone = v < DEAD_ZONE && v > -DEAD_ZONE;

        if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
          if (isDeadZone) {
            leftStick.x = 0.f;
          } else {
            leftStick.x = v;
          }
        } else if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
          if (isDeadZone) {
            leftStick.y = 0.f;
          } else {
            leftStick.y = v;
          }
        } else if (event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX) {
          if (isDeadZone) {
            rightStick.x = 0.f;
          } else {
            rightStick.x = v;
          }
        } else if (event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY) {
          if (isDeadZone) {
            rightStick.y = 0.f;
          } else {
            rightStick.y = v;
          }
        }

        break;
      }
      case SDL_CONTROLLERBUTTONDOWN:
        handleControllerButtonDown(event.cbutton.button);
        break;
      case SDL_CONTROLLERBUTTONUP:
        handleControllerButtonUp(event.cbutton.button);
        break;
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
      case SDL_MOUSEBUTTONUP:
        handleMouseUp(event.button);
        break;
      case SDL_MOUSEWHEEL:
        handleMouseWheel(event.wheel);
        break;
      case SDL_TEXTINPUT:
        handleTextInput(event.text.text[0]);
        break;
    }
  }

  // @todo refactor with handleKeyDown()
  void InputSystem::handleControllerButtonDown(Uint8 button) {
    if (controllerButtonMap.find(button) != controllerButtonMap.end()) {
      Key key = controllerButtonMap.at(button);

      if (!isKeyHeld(key)) {
        signal("keystart", key);

        pressedKeyState |= (u64)key;
      }

      heldKeyState |= (u64)key;
      lastKeyDown = (u64)key;

      signal("keydown", key);
    }
  }

  void InputSystem::handleControllerButtonUp(Uint8 button) {
    if (controllerButtonMap.find(button) != controllerButtonMap.end()) {
      Key key = controllerButtonMap.at(button);

      heldKeyState &= ~(u64)key;
      pressedKeyState &= ~(u64)key;
      releasedKeyState |= (u64)key;

      signal("keyup", key);
    }
  }

  // @todo refactor with handleControllerButtonDown()
  void InputSystem::handleKeyDown(const SDL_Keycode& code) {
    if (keyMap.find(code) != keyMap.end()) {
      Key key = keyMap.at(code);

      if (!isKeyHeld(key)) {
        signal("keystart", key);

        pressedKeyState |= (u64)key;
      }

      heldKeyState |= (u64)key;
      lastKeyDown = (u64)key;

      signal("keydown", key);
    }
  }

  void InputSystem::handleKeyUp(const SDL_Keycode& code) {
    if (keyMap.find(code) != keyMap.end()) {
      Key key = keyMap.at(code);

      heldKeyState &= ~(u64)key;
      pressedKeyState &= ~(u64)key;
      releasedKeyState |= (u64)key;

      signal("keyup", key);
    }
  }

  void InputSystem::handleMouseDown(const SDL_MouseButtonEvent& event) {
    MouseButtonEvent buttonEvent;

    buttonEvent.position.x = event.x;
    buttonEvent.position.y = event.y;

    signal("mousedown", buttonEvent);

    didClickMouseThisFrame = true;
    didRightClickThisFrame = event.button == SDL_BUTTON_RIGHT;
    isMouseButtonHeldDown = true;
  }

  void InputSystem::handleMouseUp(const SDL_MouseButtonEvent& event) {
    MouseButtonEvent buttonEvent;

    buttonEvent.position.x = event.x;
    buttonEvent.position.y = event.y;

    signal("mouseup", buttonEvent);

    didReleaseMouseThisFrame = true;
    isMouseButtonHeldDown = false;
  }

  void InputSystem::handleMouseMotion(const SDL_MouseMotionEvent& event) {
    MouseMoveEvent moveEvent;

    moveEvent.deltaX = event.xrel;
    moveEvent.deltaY = event.yrel;

    signal("mousemove", moveEvent);

    mouseDelta.x = event.xrel;
    mouseDelta.y = event.yrel;
  }

  void InputSystem::handleMouseWheel(const SDL_MouseWheelEvent& event) {
    MouseWheelEvent wheelEvent;

    wheelEvent.direction = event.y < 0
      ? MouseWheelEvent::DOWN
      : MouseWheelEvent::UP;

    signal("mousewheel", wheelEvent);

    didMoveMouseWheelThisFrame = true;
    mousewheelDirection = wheelEvent.direction;
  }

  void InputSystem::handleTextInput(char character) {
    signal("input", character);
  }

  bool InputSystem::isKeyHeld(Key key) const {
    return heldKeyState & (u64)key;
  }

  bool InputSystem::isMouseHeld() const {
    return isMouseButtonHeldDown;
  }

  void InputSystem::resetPerFrameState() {
    pressedKeyState = 0;
    releasedKeyState = 0;
    didClickMouseThisFrame = false;
    didRightClickThisFrame = false;
    didReleaseMouseThisFrame = false;
    didMoveMouseWheelThisFrame = false;
    mouseDelta = { 0, 0 };
  }
}