#include "system/packed_data.h"
#include "system/type_aliases.h"

#define CLAMP(f) (f < 0.f ? 0.f : f > 1.f ? 1.f : f)

namespace Gamma {
  /**
   * pVec4
   * -----
   */
  pVec4::pVec4(const Vec3f& value) {
    r = u8(CLAMP(value.x) * 255.f);
    g = u8(CLAMP(value.y) * 255.f);
    b = u8(CLAMP(value.z) * 255.f);
    a = 255;
  }

  pVec4::pVec4(const Vec4f& value) {
    r = u8(CLAMP(value.x) * 255.f);
    g = u8(CLAMP(value.y) * 255.f);
    b = u8(CLAMP(value.z) * 255.f);
    a = u8(CLAMP(value.w) * 255.f);
  }

  bool pVec4::operator==(const pVec4& color) const {
    return (
      color.r == r &&
      color.b == b &&
      color.g == g &&
      color.a == a
    );
  }

  Vec3f pVec4::toVec3f() const {
    return Vec3f(
      r / 255.f,
      g / 255.f,
      b / 255.f
    );
  }
}