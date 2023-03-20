#pragma once

#include "system/type_aliases.h"

#if __has_include("gamma_flags.h")
  // Include custom #define flag overrides
  #include "gamma_flags.h"
#endif

namespace Gamma {
  enum GammaFlags {
    WIREFRAME_MODE = 1 << 1,
    VSYNC = 1 << 2,

    ENABLE_DENOISING = 1 << 3,
    ENABLE_DEV_LIGHT_DISCS = 1 << 4,
    ENABLE_DEV_BUFFERS = 1 << 5,
    ENABLE_DEV_TOOLS = 1 << 6,

    RENDER_REFLECTIONS = 1 << 12,
    RENDER_REFRACTIVE_GEOMETRY = 1 << 13,
    RENDER_REFRACTIVE_GEOMETRY_WITHIN_REFLECTIONS = 1 << 14,
    RENDER_SHADOWS = 1 << 15,
    RENDER_AMBIENT_OCCLUSION = 1 << 16,
    RENDER_GLOBAL_ILLUMINATION = 1 << 17,
    RENDER_INDIRECT_SKY_LIGHT = 1 << 18,
    RENDER_DEPTH_OF_FIELD = 1 << 19,
    RENDER_HORIZON_ATMOSPHERE = 1 << 20
  };

  void Gm_DisableFlags(GammaFlags flags);
  void Gm_EnableFlags(GammaFlags flags);
  bool Gm_FlagWasDisabled(GammaFlags flag);
  bool Gm_FlagWasEnabled(GammaFlags flag);
  u32 Gm_GetFlags();
  bool Gm_IsFlagEnabled(GammaFlags flag);
  void Gm_SavePreviousFlags();
  void Gm_ToggleFlag(GammaFlags flag);
}