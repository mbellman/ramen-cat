#pragma once

namespace Gamma {
  template<typename T>
  struct Point {
    T x;
    T y;
  };

  template<typename T>
  struct Area {
    T width;
    T height;
  };

  template<typename T>
  struct Region {
    T x;
    T y;
    T width;
    T height;
  };
}