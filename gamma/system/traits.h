#pragma once

namespace Gamma {
  struct Initable {
    virtual void init() = 0;
  };

  struct Destroyable {
    virtual void destroy() = 0;
  };

  struct Renderable {
    virtual void render() = 0;
  };
}