#pragma once

template <typename F>
struct _Gm_defer {
  F f;

  _Gm_defer(F f): f(f) {};

  ~_Gm_defer() {
    f();
  }
};

/**
 * defer
 * -----
 */
#define defer(code) _Gm_defer _d([&](){ code; });