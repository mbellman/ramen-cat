#pragma once

#include "math/vector.h"
#include "system/type_aliases.h"

namespace Gamma {
  /**
   * pVec4
   * -----
   *
   * A packed vector containing four 8-bit integer components.
   */
  struct pVec4 {
    u8 r = 255;
    u8 g = 255;
    u8 b = 255;
    u8 a = 255;

    pVec4() {};
    pVec4(u8 r, u8 g, u8 b) : r(r), g(g), b(b), a(255) {};
    pVec4(u8 r, u8 g, u8 b, u8 a) : r(r), g(g), b(b), a(a) {};
    pVec4(const Vec3f& value);
    pVec4(const Vec4f& value);
  };
}