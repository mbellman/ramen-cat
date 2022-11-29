#pragma once

#include "system/type_aliases.h"

namespace Gamma {
  class OpenGLScreenQuad {
  public:
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