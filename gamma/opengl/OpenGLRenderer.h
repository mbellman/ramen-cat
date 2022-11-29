#pragma once

#include <vector>

#include "SDL.h"
#include "SDL_ttf.h"

#include "math/vector.h"
#include "opengl/framebuffer.h"
#include "opengl/OpenGLLightDisc.h"
#include "opengl/OpenGLMesh.h"
#include "opengl/shader.h"
#include "opengl/shadowmaps.h"
#include "system/AbstractRenderer.h"
#include "system/entities.h"
#include "system/type_aliases.h"

namespace Gamma {
  struct RendererBuffers {
    OpenGLFrameBuffer gBuffer;
    OpenGLFrameBuffer indirectLight[2];
    OpenGLFrameBuffer reflections;
    OpenGLFrameBuffer accumulation1;
    OpenGLFrameBuffer accumulation2;
  };

  struct RendererShaders {
    // Rendering pipeline shaders
    OpenGLShader geometry;
    OpenGLShader foliage;
    OpenGLShader probeReflector;
    OpenGLShader particles;
    OpenGLShader lightingPrepass;
    OpenGLShader directionalLight;
    OpenGLShader spotLight;
    OpenGLShader pointLight;
    OpenGLShader indirectLight;
    OpenGLShader indirectLightComposite;
    OpenGLShader skybox;
    OpenGLShader copyFrame;
    OpenGLShader reflections;
    OpenGLShader reflectionsDenoise;
    OpenGLShader refractivePrepass;
    OpenGLShader refractiveGeometry;
    OpenGLShader water;

    // Shadowcaster shaders
    OpenGLShader directionalShadowcaster;
    OpenGLShader spotShadowcaster;
    OpenGLShader pointShadowcasterView;
    OpenGLShader shadowLightView;
    OpenGLShader pointShadowcaster;

    // Dev shaders
    OpenGLShader gBufferDev;
    OpenGLShader directionalShadowMapDev;
  };

  struct RendererContext {
    u32 internalWidth;
    u32 internalHeight;
    bool hasEmissiveObjects;
    bool hasReflectiveObjects;
    bool hasRefractiveObjects;
    bool hasWaterObjects;
    GLenum primitiveMode;
    std::vector<Light*> pointLights;
    std::vector<Light*> pointShadowcasters;
    std::vector<Light*> directionalLights;
    std::vector<Light*> directionalShadowcasters;
    std::vector<Light*> spotLights;
    std::vector<Light*> spotShadowcasters;
    Camera* activeCamera = nullptr;
    Matrix4f matProjection;
    Matrix4f matInverseProjection;
    Matrix4f matView;
    Matrix4f matInverseView;
    Matrix4f matPreviousView;
    OpenGLFrameBuffer* accumulationSource = nullptr;
    OpenGLFrameBuffer* accumulationTarget = nullptr;
    // @todo target (fbo)
  };

  class OpenGLRenderer final : public AbstractRenderer {
  public:
    OpenGLRenderer(GmContext* gmContext): AbstractRenderer(gmContext) {};
    ~OpenGLRenderer() {};

    virtual void init() override;
    virtual void destroy() override;
    virtual void render() override;
    virtual void createMesh(const Mesh* mesh) override;
    virtual void createShadowMap(const Light* light) override;
    virtual void destroyMesh(const Mesh* mesh) override;
    virtual void destroyShadowMap(const Light* light) override;
    virtual const RenderStats& getRenderStats() override;
    virtual void present() override;
    virtual void renderText(TTF_Font* font, const char* message, u32 x, u32 y, const Vec3f& color, const Vec4f& background) override;
    virtual void resetShadowMaps() override;

  private:
    SDL_GLContext glContext;
    RendererBuffers buffers;
    RendererShaders shaders;
    RendererContext ctx;
    OpenGLLightDisc lightDisc;
    OpenGLShader screen;
    GLuint screenTexture = 0;
    u32 frame = 0;
    std::vector<OpenGLMesh*> glMeshes;
    std::vector<OpenGLDirectionalShadowMap*> glDirectionalShadowMaps;
    std::vector<OpenGLPointShadowMap*> glPointShadowMaps;
    std::vector<OpenGLSpotShadowMap*> glSpotShadowMaps;
    std::map<std::string, OpenGLCubeMap*> glProbes;
    bool areProbesRendered = false;

    struct PostShaders {
      OpenGLShader debanding;
    } post;
 
    void renderSceneToGBuffer();
    void renderDirectionalShadowMaps();
    void renderPointShadowMaps();
    void renderSpotShadowMaps();
    void prepareLightingPass();
    void renderLightingPrepass();
    void renderDirectionalLights();
    void renderDirectionalShadowcasters();
    void renderSpotLights();
    void renderSpotShadowcasters();
    void renderPointLights();
    void renderPointShadowcasters();
    void copyEmissiveObjects();
    void renderIndirectLight();
    void renderSkybox();
    void renderParticleSystems();
    void renderReflections();
    void renderRefractiveGeometry();
    void renderWater();
    void renderPostEffects();
    void renderDevBuffers();

    void createAndRenderProbe(const std::string& name, const Vec3f& position);
    void handleSettingsChanges();
    void initializeRendererContext();
    void initializeLightArrays();
    void renderSurfaceToScreen(SDL_Surface* surface, u32 x, u32 y, const Vec3f& color, const Vec4f& background);
    void renderToAccumulationBuffer();
    void swapAccumulationBuffers();
  };
}