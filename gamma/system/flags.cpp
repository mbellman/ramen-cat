#include "system/flags.h"
#include "system/type_aliases.h"

namespace Gamma {
  static u32 internalFlags =
    GammaFlags::RENDER_REFLECTIONS |
    GammaFlags::RENDER_REFRACTIVE_GEOMETRY |
    GammaFlags::RENDER_SHADOWS |
    GammaFlags::RENDER_AMBIENT_OCCLUSION |
    GammaFlags::RENDER_INDIRECT_SKY_LIGHT |
    GammaFlags::RENDER_GLOBAL_ILLUMINATION |
    GammaFlags::RENDER_DEV_BUFFERS |
    GammaFlags::ENABLE_DENOISING;

  static u32 previousFlags = internalFlags;

  void Gm_DisableFlags(GammaFlags flags) {
    internalFlags &= ~flags;
  }

  void Gm_EnableFlags(GammaFlags flags) {
    internalFlags |= flags;
  }

  bool Gm_FlagWasDisabled(GammaFlags flag) {
    return (previousFlags & flag) && !(internalFlags & flag);
  }

  bool Gm_FlagWasEnabled(GammaFlags flag) {
    return !(previousFlags & flag) && (internalFlags & flag);
  }

  u32 Gm_GetFlags() {
    return internalFlags;
  }

  bool Gm_IsFlagEnabled(GammaFlags flag) {
    return internalFlags & flag;
  }

  void Gm_SavePreviousFlags() {
    previousFlags = internalFlags;
  }
}