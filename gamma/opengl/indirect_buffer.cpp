#include "opengl/indirect_buffer.h"

#include "glew.h"

namespace Gamma {
  GLuint glDrawIndirectBuffer = 0;

  void Gm_InitDrawIndirectBuffer() {
    glGenBuffers(1, &glDrawIndirectBuffer);
  }

  void Gm_BufferDrawElementsIndirectCommands(const GlDrawElementsIndirectCommand* commands, u32 total) {
    // @todo eventually treat this as a command queue,
    // rather than recreating the buffer store each time
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, glDrawIndirectBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, total * sizeof(GlDrawElementsIndirectCommand), commands, GL_DYNAMIC_DRAW);
  }

  void Gm_DestroyDrawIndirectBuffer() {
    glDeleteBuffers(1, &glDrawIndirectBuffer);
  }
}