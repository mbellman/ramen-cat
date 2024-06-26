#pragma once

#include <vector>

#include "SDL.h"
#include "SDL_ttf.h"

#include "math/vector.h"
#include "opengl/framebuffer.h"
#include "opengl/OpenGLLightDisc.h"
#include "opengl/OpenGLMesh.h"
#include "opengl/OpenGLTexture.h"
#include "opengl/shader.h"
#include "opengl/shadowmaps.h"
#include "system/AbstractRenderer.h"
#include "system/lights_objects_meshes.h"
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
    OpenGLShader presetAnimation;
    OpenGLShader probeReflector;
    OpenGLShader gpuParticle;
    OpenGLShader particle;
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
    OpenGLShader ocean;
    OpenGLShader silhouette;
    OpenGLShader post;

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
    bool hasOceanObjects;
    bool hasSilhouetteObjects;
    GLenum primitiveMode;
    std::vector<Light*> pointLights;
    std::vector<Light*> pointShadowcasters;
    std::vector<Light*> directionalLights;
    std::vector<Light*> directionalShadowcasters;
    std::vector<Light*> spotLights;
    std::vector<Light*> spotShadowcasters;
    OpenGLTexture* cloudsTexture = nullptr;
    Camera* activeCamera = nullptr;
    Matrix4f matProjection;
    Matrix4f matInverseProjection;
    Matrix4f matView;
    Matrix4f matInverseView;
    Matrix4f matPreviousView;
    Matrix4f matViewProjection;
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
    virtual void createMesh(Mesh* mesh) override;
    virtual void createShadowMap(Light* light) override;
    virtual void destroyMesh(Mesh* mesh) override;
    virtual void destroyShadowMap(Light* light) override;
    virtual void destroyProbe(const std::string& name) override;
    virtual const RenderStats& getRenderStats() override;
    virtual void present() override;
    virtual void renderSurface(SDL_Surface* surface, u32 x, u32 y, u32 w, u32 h, const Vec3f& color, const Vec4f& background) override;
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
    float lastShaderHotReloadCheckTime = 0.f;
    std::vector<OpenGLMesh*> glMeshes;
    std::vector<OpenGLDirectionalShadowMap*> glDirectionalShadowMaps;
    std::vector<OpenGLPointShadowMap*> glPointShadowMaps;
    std::vector<OpenGLSpotShadowMap*> glSpotShadowMaps;
    std::map<std::string, OpenGLCubeMap*> glProbes;
    bool areProbesRendered = false;
 
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
    void renderParticles();
    void renderReflections();
    void renderRefractiveGeometry();
    void renderOcean();
    void renderSilhouettes();
    void renderPostEffects();
    void renderDevBuffers();

    void createAndRenderProbe(const std::string& name, const Vec3f& position);
    void handleSettingsChanges();
    void renderToAccumulationBuffer();
    void swapAccumulationBuffers();
    void updateRendererContext();
    void updateLightArrays();
  };
}