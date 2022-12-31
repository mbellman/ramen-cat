#include <string>

#include "opengl/OpenGLTexture.h"
#include "system/assert.h"

#include "glew.h"
#include "SDL_image.h"

namespace Gamma {
  // @todo add hot-reloading in dev mode
  OpenGLTexture::OpenGLTexture(const std::string& path, GLenum unit, bool enableMipmaps) {
    this->unit = unit;
    this->path = path;

    SDL_Surface* surface = IMG_Load(path.c_str());

    assert(surface != 0, "Failed to load texture: " + path);

    GLuint format = surface->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;

    glGenTextures(1, &id);

    bind();

    glTexImage2D(GL_TEXTURE_2D, 0, format, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);

    if (enableMipmaps) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glGenerateMipmap(GL_TEXTURE_2D);
    } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

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