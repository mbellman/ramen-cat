#pragma once

#include <functional>

#include "SDL_events.h"
#include "math/plane.h"
#include "system/Signaler.h"
#include "system/type_aliases.h"

namespace Gamma {
  enum class Key : u64 {
    A = 1ULL << 0,
    B = 1ULL << 1,
    C = 1ULL << 2,
    D = 1ULL << 3,
    E = 1ULL << 4,
    F = 1ULL << 5,
    G = 1ULL << 6,
    H = 1ULL << 7,
    I = 1ULL << 8,
    J = 1ULL << 9,
    K = 1ULL << 10,
    L = 1ULL << 11,
    M = 1ULL << 12,
    N = 1ULL << 13,
    O = 1ULL << 14,
    P = 1ULL << 15,
    Q = 1ULL << 16,
    R = 1ULL << 17,
    S = 1ULL << 18,
    T = 1ULL << 19,
    U = 1ULL << 20,
    V = 1ULL << 21,
    W = 1ULL << 22,
    X = 1ULL << 23,
    Y = 1ULL << 24,
    Z = 1ULL << 25,
    NUM_0 = 1ULL << 26,
    NUM_1 = 1ULL << 27,
    NUM_2 = 1ULL << 28,
    NUM_3 = 1ULL << 29,
    NUM_4 = 1ULL << 30,
    NUM_5 = 1ULL << 31,
    NUM_6 = 1ULL << 32,
    NUM_7 = 1ULL << 33,
    NUM_8 = 1ULL << 34,
    NUM_9 = 1ULL << 35,
    ARROW_LEFT = 1ULL << 36,
    ARROW_RIGHT = 1ULL << 37,
    ARROW_UP = 1ULL << 38,
    ARROW_DOWN = 1ULL << 39,
    SPACE = 1ULL << 40,
    SHIFT = 1ULL << 41,
    ESCAPE = 1ULL << 42,
    ENTER = 1ULL << 43,
    CONTROL = 1ULL << 44,
    BACKSPACE = 1ULL << 45,
    TAB = 1ULL << 46
  };

  struct MouseMoveEvent {
    int deltaX;
    int deltaY;
  };

  struct MouseButtonEvent {
    Point<int> position;
  };

  struct MouseWheelEvent {
    enum Direction {
      UP,
      DOWN
    };

    Direction direction = UP;
  };

  struct KeyboardEvent {
    Key key;
  };

  class InputSystem : public Signaler {
  public:
    u64 getLastKeyDown() const;
    void handleEvent(const SDL_Event& event);
    bool isKeyHeld(Key key);

  private:
    u64 keyState = 0;
    u64 lastKeyDown = 0;

    void handleKeyDown(const SDL_Keycode& code);
    void handleKeyUp(const SDL_Keycode& code);
    void handleMouseDown(const SDL_MouseButtonEvent& event);
    void handleMouseMotion(const SDL_MouseMotionEvent& event);
    void handleMouseWheel(const SDL_MouseWheelEvent& event);
    void handleTextInput(char character);
  };
}