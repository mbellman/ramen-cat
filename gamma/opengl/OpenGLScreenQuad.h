#pragma once

#include "system/type_aliases.h"

namespace Gamma {
  class OpenGLScreenQuad {
  public:
    static u32 totalDrawCalls;

    static void render();

  private:
    GLuint vao;
    GLuint vbo;
    static OpenGLScreenQuad* singleton;

    OpenGLScreenQuad();
    ~OpenGLScreenQuad();

    void draw();
  };
}