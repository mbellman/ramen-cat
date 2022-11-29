#include "opengl/renderer_setup.h"
#include "system/flags.h"

#include "glew.h"

namespace Gamma {
  void Gm_InitRendererResources(RendererBuffers& buffers, RendererShaders& shaders, const Area<u32>& internalResolution) {
    // Initialize buffers
    buffers.gBuffer.init();
    buffers.gBuffer.setSize(internalResolution);
    buffers.gBuffer.addColorAttachment(ColorFormat::RGBA);    // (RGB) Albedo, (A) Depth
    buffers.gBuffer.addColorAttachment(ColorFormat::RGBA16);  // (RGB) Normal, (A) Emissivity
    buffers.gBuffer.addDepthStencilAttachment();
    buffers.gBuffer.bindColorAttachments();

    buffers.indirectLight[0].init();
    buffers.indirectLight[0].setSize({ internalResolution.width / 2, internalResolution.height / 2 });
    buffers.indirectLight[0].addColorAttachment(ColorFormat::RGBA8, 2);  // (RGB) GI, (A) AO
    buffers.indirectLight[0].bindColorAttachments();

    buffers.indirectLight[1].init();
    buffers.indirectLight[1].setSize({ internalResolution.width / 2, internalResolution.height / 2 });
    buffers.indirectLight[1].addColorAttachment(ColorFormat::RGBA8, 2);  // (RGB) GI, (A) AO
    buffers.indirectLight[1].bindColorAttachments();

    buffers.reflections.init();
    buffers.reflections.setSize(internalResolution);
    // @todo consider whether we need a half-size reflections buffer
    // buffers.reflections.setSize({ internalResolution.width / 2, internalResolution.height / 2 });
    buffers.reflections.addColorAttachment(ColorFormat::RGBA);  // (RGB) Color, (A) Depth
    buffers.gBuffer.shareDepthStencilAttachment(buffers.reflections);
    buffers.reflections.bindColorAttachments();

    buffers.accumulation1.init();
    buffers.accumulation1.setSize(internalResolution);
    buffers.accumulation1.addColorAttachment(ColorFormat::RGBA);  // (RGB) Color, (A) Depth

    // @todo OpenGLFrameBuffer::enableMipmaps()
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glGenerateMipmap(GL_TEXTURE_2D);

    buffers.gBuffer.shareDepthStencilAttachment(buffers.accumulation1);
    buffers.accumulation1.bindColorAttachments();

    buffers.accumulation2.init();
    buffers.accumulation2.setSize(internalResolution);
    buffers.accumulation2.addColorAttachment(ColorFormat::RGBA);  // (RGB) Color, (A) Depth

    // @todo OpenGLFrameBuffer::enableMipmaps()
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glGenerateMipmap(GL_TEXTURE_2D);

    buffers.gBuffer.shareDepthStencilAttachment(buffers.accumulation2);
    buffers.accumulation2.bindColorAttachments();

    // Initialize shaders
    shaders.geometry.init();
    shaders.geometry.vertex("./gamma/opengl/shaders/geometry.vert.glsl");
    shaders.geometry.fragment("./gamma/opengl/shaders/geometry.frag.glsl");
    shaders.geometry.link();

    shaders.foliage.init();
    shaders.foliage.vertex("./gamma/opengl/shaders/foliage.vert.glsl");
    shaders.foliage.fragment("./gamma/opengl/shaders/geometry.frag.glsl");
    shaders.foliage.link();

    shaders.probeReflector.init();
    shaders.probeReflector.vertex("./gamma/opengl/shaders/geometry.vert.glsl");
    shaders.probeReflector.fragment("./gamma/opengl/shaders/probe-reflector.frag.glsl");
    shaders.probeReflector.link();

    shaders.particles.init();
    shaders.particles.vertex("./gamma/opengl/shaders/particle-system.vert.glsl");
    shaders.particles.fragment("./gamma/opengl/shaders/particle-system.frag.glsl");
    shaders.particles.link();

    shaders.lightingPrepass.init();
    shaders.lightingPrepass.vertex("./gamma/opengl/shaders/quad.vert.glsl");
    shaders.lightingPrepass.fragment("./gamma/opengl/shaders/lighting-prepass.frag.glsl");
    shaders.lightingPrepass.link();

    shaders.directionalLight.init();
    shaders.directionalLight.vertex("./gamma/opengl/shaders/quad.vert.glsl");
    shaders.directionalLight.fragment("./gamma/opengl/shaders/directional-light-without-shadow.frag.glsl");
    shaders.directionalLight.link();

    shaders.spotLight.init();
    shaders.spotLight.vertex("./gamma/opengl/shaders/light-disc.vert.glsl");
    shaders.spotLight.fragment("./gamma/opengl/shaders/spot-light-without-shadow.frag.glsl");
    shaders.spotLight.link();

    shaders.pointLight.init();
    shaders.pointLight.vertex("./gamma/opengl/shaders/light-disc.vert.glsl");
    shaders.pointLight.fragment("./gamma/opengl/shaders/point-light-without-shadow.frag.glsl");
    shaders.pointLight.link();

    shaders.directionalShadowcaster.init();
    shaders.directionalShadowcaster.vertex("./gamma/opengl/shaders/quad.vert.glsl");
    shaders.directionalShadowcaster.fragment("./gamma/opengl/shaders/directional-light-with-shadow.frag.glsl");
    shaders.directionalShadowcaster.link();

    shaders.spotShadowcaster.init();
    shaders.spotShadowcaster.vertex("./gamma/opengl/shaders/light-disc.vert.glsl");
    shaders.spotShadowcaster.fragment("./gamma/opengl/shaders/spot-light-with-shadow.frag.glsl");
    shaders.spotShadowcaster.link();

    shaders.pointShadowcaster.init();
    shaders.pointShadowcaster.vertex("./gamma/opengl/shaders/light-disc.vert.glsl");
    shaders.pointShadowcaster.fragment("./gamma/opengl/shaders/point-light-with-shadow.frag.glsl");
    shaders.pointShadowcaster.link();

    shaders.shadowLightView.init();
    shaders.shadowLightView.vertex("./gamma/opengl/shaders/shadow-light-view.vert.glsl");
    shaders.shadowLightView.fragment("./gamma/opengl/shaders/shadow-light-view.frag.glsl");
    shaders.shadowLightView.link();

    shaders.pointShadowcasterView.init();
    shaders.pointShadowcasterView.vertex("./gamma/opengl/shaders/point-light-view.vert.glsl");
    shaders.pointShadowcasterView.geometry("./gamma/opengl/shaders/point-light-view.geom.glsl");
    shaders.pointShadowcasterView.fragment("./gamma/opengl/shaders/point-light-view.frag.glsl");
    shaders.pointShadowcasterView.link();

    shaders.indirectLight.init();
    shaders.indirectLight.vertex("./gamma/opengl/shaders/quad.vert.glsl");
    shaders.indirectLight.fragment("./gamma/opengl/shaders/indirect-light.frag.glsl");
    shaders.indirectLight.link();

    shaders.indirectLightComposite.init();
    shaders.indirectLightComposite.vertex("./gamma/opengl/shaders/quad.vert.glsl");
    shaders.indirectLightComposite.fragment("./gamma/opengl/shaders/indirect-light-composite.frag.glsl");
    shaders.indirectLightComposite.link();

    shaders.skybox.init();
    shaders.skybox.vertex("./gamma/opengl/shaders/quad.vert.glsl");
    shaders.skybox.fragment("./gamma/opengl/shaders/skybox.frag.glsl");
    shaders.skybox.link();

    shaders.copyFrame.init();
    shaders.copyFrame.vertex("./gamma/opengl/shaders/quad.vert.glsl");
    shaders.copyFrame.fragment("./gamma/opengl/shaders/copy-frame.frag.glsl");
    shaders.copyFrame.link();

    // @todo define different SSR quality levels
    shaders.reflections.init();
    shaders.reflections.vertex("./gamma/opengl/shaders/quad.vert.glsl");
    shaders.reflections.fragment("./gamma/opengl/shaders/reflections.frag.glsl");
    shaders.reflections.link();

    shaders.reflectionsDenoise.init();
    shaders.reflectionsDenoise.vertex("./gamma/opengl/shaders/quad.vert.glsl");
    shaders.reflectionsDenoise.fragment("./gamma/opengl/shaders/reflections-denoise.frag.glsl");
    shaders.reflectionsDenoise.link();

    shaders.refractivePrepass.init();
    shaders.refractivePrepass.vertex("./gamma/opengl/shaders/geometry.vert.glsl");
    shaders.refractivePrepass.fragment("./gamma/opengl/shaders/refractive-prepass.frag.glsl");
    shaders.refractivePrepass.link();

    shaders.refractiveGeometry.init();
    shaders.refractiveGeometry.vertex("./gamma/opengl/shaders/geometry.vert.glsl");
    shaders.refractiveGeometry.fragment("./gamma/opengl/shaders/refractive-geometry.frag.glsl");
    shaders.refractiveGeometry.link();

    shaders.water.init();
    shaders.water.vertex("./gamma/opengl/shaders/geometry.vert.glsl");
    shaders.water.fragment("./gamma/opengl/shaders/water.frag.glsl");
    shaders.water.link();

    #if GAMMA_DEVELOPER_MODE
      shaders.gBufferDev.init();
      shaders.gBufferDev.vertex("./gamma/opengl/shaders/quad.vert.glsl");
      shaders.gBufferDev.fragment("./gamma/opengl/shaders/dev/g-buffer.frag.glsl");
      shaders.gBufferDev.link();

      shaders.directionalShadowMapDev.init();
      shaders.directionalShadowMapDev.vertex("./gamma/opengl/shaders/quad.vert.glsl");
      shaders.directionalShadowMapDev.fragment("./gamma/opengl/shaders/dev/directional-shadow-map.frag.glsl");
      shaders.directionalShadowMapDev.link();
    #endif
  }

  void Gm_DestroyRendererResources(RendererBuffers& buffers, RendererShaders& shaders) {
    buffers.gBuffer.destroy();
    buffers.indirectLight[0].destroy();
    buffers.indirectLight[1].destroy();
    buffers.reflections.destroy();
    buffers.accumulation1.destroy();
    buffers.accumulation2.destroy();

    shaders.geometry.destroy();
    shaders.probeReflector.destroy();
    shaders.particles.destroy();
    shaders.lightingPrepass.destroy();
    shaders.directionalLight.destroy();
    shaders.spotLight.destroy();
    shaders.pointLight.destroy();
    shaders.directionalShadowcaster.destroy();
    shaders.spotShadowcaster.destroy();
    shaders.pointShadowcaster.destroy();
    shaders.shadowLightView.destroy();
    shaders.pointShadowcasterView.destroy();
    shaders.indirectLight.destroy();
    shaders.indirectLightComposite.destroy();
    shaders.skybox.destroy();
    shaders.copyFrame.destroy();
    shaders.reflections.destroy();
    shaders.reflectionsDenoise.destroy();
    shaders.refractivePrepass.destroy();
    shaders.refractiveGeometry.destroy();
    shaders.water.destroy();

    #if GAMMA_DEVELOPER_MODE
      shaders.gBufferDev.destroy();
      shaders.directionalShadowMapDev.destroy();
    #endif
  }
}