#include "system/flags.h"
#include "system/type_aliases.h"

namespace Gamma {
  static u32 internalFlags =
    GammaFlags::ENABLE_DENOISING |
    GammaFlags::ENABLE_DEV_TOOLS |
    GammaFlags::RENDER_REFLECTIONS |
    GammaFlags::RENDER_REFRACTIVE_GEOMETRY |
    GammaFlags::RENDER_SHADOWS |
    GammaFlags::RENDER_INDIRECT_SKY_LIGHT |
    GammaFlags::RENDER_AMBIENT_OCCLUSION |
    GammaFlags::RENDER_GLOBAL_ILLUMINATION |
    GammaFlags::RENDER_HORIZON_ATMOSPHERE;

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

  void Gm_ToggleFlag(GammaFlags flag) {
    if (internalFlags & flag) {
      Gm_DisableFlags(flag);
    } else {
      Gm_EnableFlags(flag);
    }
  }
}