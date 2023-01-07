#pragma once

#include "system/type_aliases.h"

#if __has_include("gamma_flags.h")
  // Include custom #define flag overrides
  #include "gamma_flags.h"
#endif

namespace Gamma {
  enum GammaFlags {
    FREE_CAMERA_MODE = 1 << 0,
    MOVABLE_OBJECTS = 1 << 1,  // @todo
    WIREFRAME_MODE = 1 << 2,
    VSYNC = 1 << 3,
    ENABLE_DENOISING = 1 << 4,
    ENABLE_DEV_BUFFERS = 1 << 5,
    RENDER_REFLECTIONS = 1 << 6,
    RENDER_REFRACTIVE_GEOMETRY = 1 << 7,
    RENDER_REFRACTIVE_GEOMETRY_WITHIN_REFLECTIONS = 1 << 8,
    RENDER_SHADOWS = 1 << 9,
    RENDER_AMBIENT_OCCLUSION = 1 << 10,
    RENDER_GLOBAL_ILLUMINATION = 1 << 11,
    RENDER_INDIRECT_SKY_LIGHT = 1 << 12,
    RENDER_DEPTH_OF_FIELD = 1 << 13
  };

  void Gm_DisableFlags(GammaFlags flags);
  void Gm_EnableFlags(GammaFlags flags);
  bool Gm_FlagWasDisabled(GammaFlags flag);
  bool Gm_FlagWasEnabled(GammaFlags flag);
  u32 Gm_GetFlags();
  bool Gm_IsFlagEnabled(GammaFlags flag);
  void Gm_SavePreviousFlags();
}