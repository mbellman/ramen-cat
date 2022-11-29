#pragma once

#include "system/type_aliases.h"

namespace Gamma {
  struct GlDrawElementsIndirectCommand {
    GLuint count;
    GLuint instanceCount;
    GLuint firstIndex;
    GLuint baseVertex;
    GLuint baseInstance;
  };

  void Gm_InitDrawIndirectBuffer();
  void Gm_BufferDrawElementsIndirectCommands(const GlDrawElementsIndirectCommand* commands, u32 total);
  void Gm_DestroyDrawIndirectBuffer();
}