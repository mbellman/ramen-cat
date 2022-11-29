#include <string>

#include "opengl/OpenGLTexture.h"
#include "system/assert.h"

#include "glew.h"
#include "SDL_image.h"

namespace Gamma {
  OpenGLTexture::OpenGLTexture(const std::string& path, GLenum unit) {
    this->unit = unit;
    this->path = path;

    SDL_Surface* surface = IMG_Load(path.c_str());

    assert(surface != 0, "Failed to load texture: " + path);

    GLuint format = surface->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;

    glGenTextures(1, &id);

    bind();

    glTexImage2D(GL_TEXTURE_2D, 0, format, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    SDL_FreeSurface(surface);
  }

  OpenGLTexture::~OpenGLTexture() {
    glDeleteTextures(1, &id);
  }

  void OpenGLTexture::bind() {
    glActiveTexture(unit);
    glBindTexture(GL_TEXTURE_2D, id);
  }

  const std::string& OpenGLTexture::getPath() const {
    return path;
  }
}