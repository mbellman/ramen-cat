#include <algorithm>
#include <cstdio>
#include <map>

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_image.h"
#include "glew.h"
#include "SDL_opengl.h"

#include "opengl/errors.h"
#include "opengl/indirect_buffer.h"
#include "opengl/OpenGLRenderer.h"
#include "opengl/OpenGLScreenQuad.h"
#include "opengl/renderer_setup.h"
#include "math/utilities.h"
#include "system/camera.h"
#include "system/console.h"
#include "system/context.h"
#include "system/flags.h"
#include "system/lights_objects_meshes.h"
#include "system/scene.h"
#include "system/vector_helpers.h"

namespace Gamma {
  const static u32 MAX_LIGHTS = 1000;
  const static Vec4f FULL_SCREEN_TRANSFORM = { 0.0f, 0.0f, 1.0f, 1.0f };

  const static Vec3f CUBE_MAP_DIRECTIONS[6] = {
    Vec3f(-1.0f, 0.0f, 0.0f),
    Vec3f(1.0f, 0.0f, 0.0f),
    Vec3f(0.0f, -1.0f, 0.0f),
    Vec3f(0.0f, 1.0f, 0.0f),
    Vec3f(0.0f, 0.0f, -1.0f),
    Vec3f(0.0f, 0.0f, 1.0f)
  };

  const static Vec3f CUBE_MAP_UP_DIRECTIONS[6] = {
    Vec3f(0.0f, -1.0f, 0.0f),
    Vec3f(0.0f, -1.0f, 0.0f),
    Vec3f(0.0f, 0.0f, 1.0f),
    Vec3f(0.0f, 0.0f, -1.0f),
    Vec3f(0.0f, -1.0f, 0.0f),
    Vec3f(0.0f, -1.0f, 0.0f)
  };

  /**
   * OpenGLRenderer
   * --------------
   */
  void OpenGLRenderer::init() {
    internalResolution = { 1920, 1080 };

    // Initialize OpenGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    glContext = SDL_GL_CreateContext(gmContext->window.sdl_window);
    glewExperimental = true;

    glewInit();

    SDL_GL_SetSwapInterval(0);

    // Initialize global buffers
    Gm_InitDrawIndirectBuffer();

    // Initialize screen texture
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Initialize renderer
    Gm_InitRendererResources(buffers, shaders, internalResolution);

    lightDisc.init();

    // Initialize remaining shaders
    screen.init();
    screen.vertex("./gamma/opengl/shaders/quad.vert.glsl");
    screen.fragment("./gamma/opengl/shaders/screen.frag.glsl");
    screen.link();

    // Enable default OpenGL settings
    glEnable(GL_PROGRAM_POINT_SIZE);
    glFrontFace(GL_CW);

    #if !GAMMA_DEVELOPER_MODE
      // Set uniform shader constants upfront, since
      // shaders won't change during runtime
      //
      // @todo these will need to be updated if the
      // internal resolution setting is changed
      Vec2f screenSize((float)internalResolution.width, (float)internalResolution.height);

      shaders.indirectLight.use();
      shaders.indirectLight.setVec2f("screenSize", screenSize);

      shaders.indirectLightComposite.use();
      shaders.indirectLightComposite.setVec2f("screenSize", screenSize);

      shaders.reflectionsDenoise.use();
      shaders.reflectionsDenoise.setVec2f("screenSize", screenSize);

      shaders.refractivePrepass.use();
      shaders.refractivePrepass.setVec2f("screenSize", screenSize);

      shaders.refractiveGeometry.use();
      shaders.refractiveGeometry.setVec2f("screenSize", screenSize);

      shaders.ocean.use();
      shaders.ocean.setVec2f("screenSize", screenSize);

      // @todo set sampler2D texture units
    #endif
  }

  void OpenGLRenderer::destroy() {
    Gm_DestroyRendererResources(buffers, shaders);
    Gm_DestroyDrawIndirectBuffer();

    lightDisc.destroy();

    glDeleteTextures(1, &screenTexture);

    SDL_GL_DeleteContext(glContext);
  }

  void OpenGLRenderer::render() {
    OpenGLMesh::totalDrawCalls = 0;
    OpenGLScreenQuad::totalDrawCalls = 0;
    OpenGLLightDisc::totalDrawCalls = 0;

    auto& scene = gmContext->scene;

    // @todo allow the clouds texture to be changed
    if (gmContext->scene.clouds.size() > 0 && ctx.cloudsTexture == nullptr) {
      ctx.cloudsTexture = new OpenGLTexture(gmContext->scene.clouds, GL_TEXTURE3, false);
    }

    #if GAMMA_DEVELOPER_MODE
      if (gmContext->contextTime - lastShaderHotReloadCheckTime > 1.f) {
        Gm_CheckAndHotReloadShaders();

        lastShaderHotReloadCheckTime = gmContext->contextTime;
      }
    #endif

    // @todo consider moving this out of render() and
    // initializing probes before the rendering loop
    if (
      !areProbesRendered &&
      // @hack make sure all mesh textures are loaded in from the
      // initial rendered frame.
      //
      // @todo properly initialize meshes, then render probes,
      // then proceed to render the scene normally.
      frame > 0 &&
      scene.probeMap.size() > 0
    ) {
      Gm_SavePreviousFlags();
      Gm_DisableFlags(GammaFlags::RENDER_AMBIENT_OCCLUSION);
      Gm_DisableFlags(GammaFlags::RENDER_GLOBAL_ILLUMINATION);

      handleSettingsChanges();
      updateRendererContext();
      updateLightArrays();

      for (auto& [ name, position ] : scene.probeMap) {
        createAndRenderProbe(name, position);
      }

      Gm_SavePreviousFlags();
      Gm_EnableFlags(GammaFlags::RENDER_AMBIENT_OCCLUSION);
      Gm_EnableFlags(GammaFlags::RENDER_GLOBAL_ILLUMINATION);

      handleSettingsChanges();

      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      areProbesRendered = true;

      return;
    }

    handleSettingsChanges();
    updateRendererContext();
    updateLightArrays();
    renderToAccumulationBuffer();
    renderPostEffects();

    #if GAMMA_DEVELOPER_MODE
      if (Gm_IsFlagEnabled(GammaFlags::ENABLE_DEV_BUFFERS)) {
        renderDevBuffers();
      }
    #endif

    frame++;

    // Reset frame flags at the end of the render pass
    frameFlags.useStableTemporalSampling = false;
  }

  /**
   * @todo description
   */
  void OpenGLRenderer::handleSettingsChanges() {
    if (Gm_FlagWasEnabled(GammaFlags::VSYNC)) {
      SDL_GL_SetSwapInterval(1);

      #if GAMMA_DEVELOPER_MODE
        Console::log("[Gamma] V-Sync enabled");
      #endif
    } else if (Gm_FlagWasDisabled(GammaFlags::VSYNC)) {
      SDL_GL_SetSwapInterval(0);

      #if GAMMA_DEVELOPER_MODE
        Console::log("[Gamma] V-Sync disabled");
      #endif
    }

    if (Gm_FlagWasEnabled(GammaFlags::ENABLE_DENOISING)) {
      shaders.indirectLight.define("USE_DENOISING", "1");
    } else if (Gm_FlagWasDisabled(GammaFlags::ENABLE_DENOISING)) {
      shaders.indirectLight.define("USE_DENOISING", "0");
    }

    if (Gm_FlagWasEnabled(GammaFlags::ENABLE_DEV_LIGHT_DISCS)) {
      shaders.pointLight.define("USE_DEV_LIGHT_DISCS", "1");
      shaders.pointShadowcaster.define("USE_DEV_LIGHT_DISCS", "1");
      shaders.spotLight.define("USE_DEV_LIGHT_DISCS", "1");
      shaders.spotShadowcaster.define("USE_DEV_LIGHT_DISCS", "1");
    } else if (Gm_FlagWasDisabled(GammaFlags::ENABLE_DEV_LIGHT_DISCS)) {
      shaders.pointLight.define("USE_DEV_LIGHT_DISCS", "0");
      shaders.pointShadowcaster.define("USE_DEV_LIGHT_DISCS", "0");
      shaders.spotLight.define("USE_DEV_LIGHT_DISCS", "0");
      shaders.spotShadowcaster.define("USE_DEV_LIGHT_DISCS", "0");
    }

    if (Gm_FlagWasEnabled(GammaFlags::RENDER_INDIRECT_SKY_LIGHT)) {
      shaders.lightingPrepass.define("USE_INDIRECT_SKY_LIGHT", "1");
    } else if (Gm_FlagWasDisabled(GammaFlags::RENDER_INDIRECT_SKY_LIGHT)) {
      shaders.lightingPrepass.define("USE_INDIRECT_SKY_LIGHT", "0");
    }

    if (Gm_FlagWasEnabled(GammaFlags::RENDER_AMBIENT_OCCLUSION)) {
      shaders.indirectLight.define("USE_SCREEN_SPACE_AMBIENT_OCCLUSION", "1");
      shaders.indirectLightComposite.define("USE_COMPOSITED_INDIRECT_LIGHT", "1");
    } else if (Gm_FlagWasDisabled(GammaFlags::RENDER_AMBIENT_OCCLUSION)) {
      shaders.indirectLight.define("USE_SCREEN_SPACE_AMBIENT_OCCLUSION", "0");

      if (!Gm_IsFlagEnabled(GammaFlags::RENDER_GLOBAL_ILLUMINATION)) {
        shaders.indirectLightComposite.define("USE_COMPOSITED_INDIRECT_LIGHT", "0");
      }
    }

    if (Gm_FlagWasEnabled(GammaFlags::RENDER_GLOBAL_ILLUMINATION)) {
      shaders.indirectLight.define("USE_SCREEN_SPACE_GLOBAL_ILLUMINATION", "1");
      shaders.indirectLightComposite.define("USE_COMPOSITED_INDIRECT_LIGHT", "1");
    } else if (Gm_FlagWasDisabled(GammaFlags::RENDER_GLOBAL_ILLUMINATION)) {
      shaders.indirectLight.define("USE_SCREEN_SPACE_GLOBAL_ILLUMINATION", "0");

      if (!Gm_IsFlagEnabled(GammaFlags::RENDER_AMBIENT_OCCLUSION)) {
        shaders.indirectLightComposite.define("USE_COMPOSITED_INDIRECT_LIGHT", "0");
      }
    }

    if (Gm_FlagWasEnabled(GammaFlags::RENDER_DEPTH_OF_FIELD)) {
      shaders.post.define("USE_DEPTH_OF_FIELD", "1");
    } else if (Gm_FlagWasDisabled(GammaFlags::RENDER_DEPTH_OF_FIELD)) {
      shaders.post.define("USE_DEPTH_OF_FIELD", "0");
    }

    if (Gm_FlagWasEnabled(GammaFlags::RENDER_HORIZON_ATMOSPHERE)) {
      shaders.post.define("USE_HORIZON_ATMOSPHERE", "1");
    } else if (Gm_FlagWasDisabled(GammaFlags::RENDER_HORIZON_ATMOSPHERE)) {
      shaders.post.define("USE_HORIZON_ATMOSPHERE", "0");
    }
  }

  /**
   * @todo description
   */
  void OpenGLRenderer::updateRendererContext() {
    auto& scene = gmContext->scene;

    // Accumulation buffers
    ctx.accumulationSource = &buffers.accumulation1;
    ctx.accumulationTarget = &buffers.accumulation2;

    // Render dimensions/primitive type
    ctx.internalWidth = internalResolution.width;
    ctx.internalHeight = internalResolution.height;
    ctx.primitiveMode = Gm_IsFlagEnabled(GammaFlags::WIREFRAME_MODE) ? GL_LINES : GL_TRIANGLES;

    // Camera projection/view/inverse matrices
    ctx.activeCamera = &gmContext->scene.camera;
    ctx.matProjection = Matrix4f::glPerspective(internalResolution, ctx.activeCamera->fov, scene.zNear, scene.zFar).transpose();
    ctx.matPreviousView = ctx.matView;

    ctx.matView = (
      ctx.activeCamera->rotation.toMatrix4f() *
      // Matrix4f::rotation(ctx.activeCamera->orientation) *
      Matrix4f::translation(ctx.activeCamera->position.invert().gl())
    ).transpose();

    if (frameFlags.useStableTemporalSampling) {
      ctx.matPreviousView = ctx.matView;
    }

    ctx.matInverseProjection = ctx.matProjection.inverse();
    ctx.matInverseView = ctx.matView.inverse();
    ctx.matViewProjection = ctx.matView * ctx.matProjection;

    // Track special object types
    ctx.hasEmissiveObjects = false;
    ctx.hasReflectiveObjects = false;
    ctx.hasRefractiveObjects = false;
    ctx.hasOceanObjects = false;
    ctx.hasSilhouetteObjects = false;

    for (auto* glMesh : glMeshes) {
      auto& mesh = *glMesh->getSourceMesh();

      if (!mesh.disabled && mesh.objects.totalActive() > 0) {
        if (mesh.type == MeshType::EMISSIVE) {
          ctx.hasEmissiveObjects = true;
        } else if (mesh.type == MeshType::REFLECTIVE) {
          ctx.hasReflectiveObjects = true;
        } else if (mesh.type == MeshType::REFRACTIVE) {
          ctx.hasRefractiveObjects = true;
        } else if (mesh.type == MeshType::OCEAN) {
          ctx.hasOceanObjects = true;
        } else if (mesh.type == MeshType::DEFAULT_WITH_OCCLUSION_SILHOUETTE) {
          ctx.hasSilhouetteObjects = true;
        }
      }
    }
  }

  /**
   * @todo description
   */
  void OpenGLRenderer::updateLightArrays() {
    ctx.pointLights.clear();
    ctx.pointShadowcasters.clear();
    ctx.directionalLights.clear();
    ctx.directionalShadowcasters.clear();
    ctx.spotLights.clear();
    ctx.spotShadowcasters.clear();

    for (auto* light : gmContext->scene.lights) {
      switch (light->type) {
        case LightType::POINT:
          ctx.pointLights.push_back(light);
          break;
        case LightType::POINT_SHADOWCASTER:
          if (Gm_IsFlagEnabled(GammaFlags::RENDER_SHADOWS)) {
            ctx.pointShadowcasters.push_back(light);
          } else {
            ctx.pointLights.push_back(light);
          }

          break;
        case LightType::DIRECTIONAL:
          ctx.directionalLights.push_back(light);
          break;
        case LightType::DIRECTIONAL_SHADOWCASTER:
          if (Gm_IsFlagEnabled(GammaFlags::RENDER_SHADOWS)) {
            ctx.directionalShadowcasters.push_back(light);
          } else {
            ctx.directionalLights.push_back(light);
          }

          break;
        case LightType::SPOT:
          ctx.spotLights.push_back(light);
          break;
        case LightType::SPOT_SHADOWCASTER:
          if (Gm_IsFlagEnabled(GammaFlags::RENDER_SHADOWS)) {
            ctx.spotShadowcasters.push_back(light);
          } else {
            ctx.spotLights.push_back(light);
          }

          break;
      }
    }
  }

  /**
   * @todo description
   */
  void OpenGLRenderer::renderToAccumulationBuffer() {
    renderSceneToGBuffer();

    if (Gm_IsFlagEnabled(GammaFlags::RENDER_SHADOWS)) {
      if (glDirectionalShadowMaps.size() > 0) {
        renderDirectionalShadowMaps();
      }

      if (glSpotShadowMaps.size() > 0) {
        renderSpotShadowMaps();
      }

      if (glPointShadowMaps.size() > 0) {
        renderPointShadowMaps();
      }
    }

    prepareLightingPass();
    renderLightingPrepass();

    if (ctx.directionalLights.size() > 0) {
      renderDirectionalLights();
    }

    if (ctx.directionalShadowcasters.size() > 0) {
      renderDirectionalShadowcasters();
    }

    if (ctx.spotLights.size() > 0) {
      renderSpotLights();
    }

    if (ctx.spotShadowcasters.size() > 0) {
      renderSpotShadowcasters();
    }

    if (ctx.pointLights.size() > 0) {
      renderPointLights();
    }

    if (ctx.pointShadowcasters.size() > 0) {
      renderPointShadowcasters();
    }

    if (ctx.hasEmissiveObjects) {
      copyEmissiveObjects();
    }

    // @todo always perform the final compositing step, since
    // this includes emissive albedo light. rename shaders/
    // terminology accordingly
    if (
      Gm_IsFlagEnabled(GammaFlags::RENDER_AMBIENT_OCCLUSION) ||
      Gm_IsFlagEnabled(GammaFlags::RENDER_GLOBAL_ILLUMINATION) ||
      Gm_IsFlagEnabled(GammaFlags::RENDER_INDIRECT_SKY_LIGHT)
    ) {
      renderIndirectLight();
    }

    glDisable(GL_BLEND);

    renderSkybox();

    if (ctx.hasReflectiveObjects && Gm_IsFlagEnabled(GammaFlags::RENDER_REFLECTIONS)) {
      renderReflections();
    }

    if (ctx.hasRefractiveObjects && Gm_IsFlagEnabled(GammaFlags::RENDER_REFRACTIVE_GEOMETRY)) {
      renderRefractiveGeometry();
    }

    if (ctx.hasOceanObjects) {
      renderOcean();
    }

    // @todo if (ctx.hasParticles)
    renderParticles();

    if (ctx.hasSilhouetteObjects) {
      renderSilhouettes();
    }

    swapAccumulationBuffers();
  }

  /**
   * @todo description
   */
  void OpenGLRenderer::renderSceneToGBuffer() {
    buffers.gBuffer.write();

    glViewport(0, 0, ctx.internalWidth, ctx.internalHeight);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glStencilMask(0xFF);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);
    glCullFace(GL_BACK);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 0xFF, 0xFF);
    glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);

    shaders.geometry.use();
    shaders.geometry.setMatrix4f("matViewProjection", ctx.matViewProjection);
    shaders.geometry.setInt("meshTexture", 0);
    shaders.geometry.setInt("meshNormalMap", 1);

    // Render emissive objects
    glStencilMask(MeshType::EMISSIVE);

    for (auto* glMesh : glMeshes) {
      if (glMesh->isMeshType(MeshType::EMISSIVE)) {
        shaders.geometry.setBool("hasTexture", glMesh->hasTexture());
        shaders.geometry.setBool("hasNormalMap", glMesh->hasNormalMap());

        glMesh->render(ctx.primitiveMode);
      }
    }

    // Render reflective objects
    glStencilMask(MeshType::REFLECTIVE);

    for (auto* glMesh : glMeshes) {
      if (glMesh->isMeshType(MeshType::REFLECTIVE)) {
        shaders.geometry.setBool("hasTexture", glMesh->hasTexture());
        shaders.geometry.setBool("hasNormalMap", glMesh->hasNormalMap());

        glMesh->render(ctx.primitiveMode);
      }
    }

    // Render objects of the default mesh type
    #define render_default_mesh()\
      shaders.geometry.setBool("hasTexture", glMesh->hasTexture());\
      shaders.geometry.setBool("hasNormalMap", glMesh->hasNormalMap());\
      shaders.geometry.setBool("useCloseTranslucency", mesh.useCloseTranslucency);\
      shaders.geometry.setBool("useXzPlaneTexturing", mesh.useXzPlaneTexturing);\
      shaders.geometry.setBool("useYPlaneTexturing", mesh.useYPlaneTexturing);\
      shaders.geometry.setFloat("emissivity", mesh.emissivity);\
      shaders.geometry.setFloat("roughness", mesh.roughness);\
      glMesh->render(ctx.primitiveMode)\

    // Render silhouettes first, since rendering them second would not affect
    // stencil buffer pixels already written with 0xFF (MeshType::DEFAULT)
    glStencilMask(MeshType::DEFAULT_WITH_OCCLUSION_SILHOUETTE);

    for (auto* glMesh : glMeshes) {
      auto& mesh = *glMesh->getSourceMesh();

      if (mesh.type == MeshType::DEFAULT_WITH_OCCLUSION_SILHOUETTE) {
        render_default_mesh();
      }
    }

    // Render non-silhouettes last
    glStencilMask(MeshType::DEFAULT);

    for (auto* glMesh : glMeshes) {      
      auto& mesh = *glMesh->getSourceMesh();

      if (mesh.type == MeshType::DEFAULT) {
        render_default_mesh();
      }
    }

    // Render preset animated meshes
    shaders.presetAnimation.use();
    shaders.presetAnimation.setMatrix4f("matViewProjection", ctx.matViewProjection);
    shaders.presetAnimation.setInt("meshTexture", 0);
    shaders.presetAnimation.setInt("meshNormalMap", 1);
    shaders.presetAnimation.setFloat("time", gmContext->contextTime);

    for (auto* glMesh : glMeshes) {
      if (glMesh->isMeshType(MeshType::PRESET_ANIMATED)) {
        auto& animation = glMesh->getSourceMesh()->animation;

        shaders.presetAnimation.setInt("animation.type", animation.type);
        shaders.presetAnimation.setFloat("animation.speed", animation.speed);
        shaders.presetAnimation.setFloat("animation.factor", animation.factor);
        shaders.presetAnimation.setBool("hasTexture", glMesh->hasTexture());
        shaders.presetAnimation.setBool("hasNormalMap", glMesh->hasNormalMap());
        shaders.presetAnimation.setFloat("emissivity", glMesh->getSourceMesh()->emissivity);
        shaders.presetAnimation.setFloat("roughness", glMesh->getSourceMesh()->roughness);

        glMesh->render(ctx.primitiveMode);
      }
    }

    // Render probe reflectors
    // @todo use ctx.hasProbeReflectors
    // @todo render probe reflectors, sans reflections, within probe cubemaps
    if (areProbesRendered) {
      glStencilFunc(GL_ALWAYS, MeshType::PROBE_REFLECTOR, 0xFF);
      glStencilMask(0xFF);

      shaders.probeReflector.use();
      shaders.probeReflector.setMatrix4f("matViewProjection", ctx.matViewProjection);
      shaders.probeReflector.setInt("meshTexture", 0);
      shaders.probeReflector.setInt("meshNormalMap", 1);
      shaders.probeReflector.setInt("probeMap", 3);
      shaders.probeReflector.setVec3f("cameraPosition", ctx.activeCamera->position);

      for (auto* glMesh : glMeshes) {
        if (glMesh->isMeshType(MeshType::PROBE_REFLECTOR)) {
          auto& probeName = glMesh->getSourceMesh()->probe;
          auto& position = gmContext->scene.probeMap[probeName];

          shaders.probeReflector.setBool("hasTexture", glMesh->hasTexture());
          shaders.probeReflector.setBool("hasNormalMap", glMesh->hasNormalMap());
          shaders.probeReflector.setVec3f("probePosition", position);

          if (glProbes.find(probeName) != glProbes.end()) {
            glProbes[probeName]->read();

            glMesh->render(ctx.primitiveMode);
          }
        }
      }
    }

    glDisable(GL_STENCIL_TEST);
  }

  /**
   * @todo description
   */
  void OpenGLRenderer::renderDirectionalShadowMaps() {
    auto& camera = *ctx.activeCamera;
    auto& shader = shaders.shadowLightView;

    shader.use();
    shader.setFloat("time", gmContext->contextTime);
    shader.setInt("meshTexture", 0);

    for (u32 mapIndex = 0; mapIndex < glDirectionalShadowMaps.size(); mapIndex++) {
      auto& glShadowMap = *glDirectionalShadowMaps[mapIndex];
      auto& light = *ctx.directionalShadowcasters[mapIndex];

      glShadowMap.buffer.write();

      for (u32 cascade = 0; cascade < 4; cascade++) {
        glShadowMap.buffer.writeToAttachment(cascade);
        Matrix4f matLightViewProjection = Gm_CreateCascadedLightViewProjectionMatrixGL(cascade, light.direction, camera);

        shader.setMatrix4f("matLightViewProjection", matLightViewProjection);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // @todo glMultiDrawElementsIndirect for static world geometry
        // (will require a handful of other changes to mesh organization/data buffering)
        for (auto* glMesh : glMeshes) {
          auto& mesh = *glMesh->getSourceMesh();

          if (
            mesh.type == MeshType::PARTICLES ||
            !mesh.canCastShadows ||
            mesh.maxCascade < (cascade + 1)
          ) {
            continue;
          }

          auto& animation = mesh.animation;

          shader.setInt("animation.type", animation.type);
          shader.setFloat("animation.speed", animation.speed);
          shader.setFloat("animation.factor", animation.factor);
          shader.setBool("hasTexture", glMesh->hasTexture());

          glMesh->render(ctx.primitiveMode, mesh.useLowestLevelOfDetailForShadows);
        }
      }
    }
  }

  /**
   * @todo description
   */
  void OpenGLRenderer::renderSpotShadowMaps() {
    auto& shader = shaders.shadowLightView;

    shader.use();
    shader.setInt("meshTexture", 0);

    for (u32 mapIndex = 0; mapIndex < glSpotShadowMaps.size(); mapIndex++) {
      auto& glShadowMap = *glSpotShadowMaps[mapIndex];
      auto& light = *glShadowMap.light;

      if (light.isStatic && glShadowMap.isRendered) {
        continue;
      }

      Matrix4f matLightProjection = Matrix4f::glPerspective({ 1024, 1024 }, 120.0f, 1.0f, light.radius);
      Matrix4f matLightView = Matrix4f::lookAt(light.position.gl(), light.direction.invert().gl(), Vec3f(0.0f, 1.0f, 0.0f));
      Matrix4f matLightViewProjection = (matLightProjection * matLightView).transpose();

      glShadowMap.buffer.write();

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      shader.setMatrix4f("matLightViewProjection", matLightViewProjection);

      // @todo glMultiDrawElementsIndirect for static world geometry
      // (will require a handful of other changes to mesh organization/data buffering)
      // @todo allow specific meshes to be associated with spot lights + rendered to shadow maps
      for (auto* glMesh : glMeshes) {
        auto& mesh = *glMesh->getSourceMesh();
        auto& animation = mesh.animation;

        shader.setInt("animation.type", animation.type);
        shader.setFloat("animation.speed", animation.speed);
        shader.setFloat("animation.factor", animation.factor);
        shader.setBool("hasTexture", glMesh->hasTexture());

        if (mesh.canCastShadows) {
          glMesh->render(ctx.primitiveMode, mesh.useLowestLevelOfDetailForShadows);
        }
      }

      glShadowMap.isRendered = true;
    }
  }

  /**
   * @todo description
   */
  void OpenGLRenderer::renderPointShadowMaps() {
    auto& shader = shaders.pointShadowcasterView;

    shader.use();

    for (u32 mapIndex = 0; mapIndex < glPointShadowMaps.size(); mapIndex++) {
      auto& glShadowMap = *glPointShadowMaps[mapIndex];
      auto& light = *ctx.pointShadowcasters[mapIndex];

      if (light.isStatic && glShadowMap.isRendered) {
        continue;
      }

      glShadowMap.buffer.write();

      glClear(GL_DEPTH_BUFFER_BIT);

      Matrix4f matLightProjection = Matrix4f::glPerspective({ 1024, 1024 }, 90.f, 1.f, light.radius);

      for (u32 i = 0; i < 6; i++) {
        auto& direction = CUBE_MAP_DIRECTIONS[i];
        auto& upDirection = CUBE_MAP_UP_DIRECTIONS[i];

        Matrix4f matLightView = Matrix4f::lookAt(light.position.gl(), direction, upDirection);
        Matrix4f lightMatrix = (matLightProjection * matLightView).transpose();

        shader.setMatrix4f("lightMatrices[" + std::to_string(i) + "]", lightMatrix);
      }

      shader.setVec3f("lightPosition", light.position.gl());
      shader.setFloat("farPlane", light.radius);

      // @todo glMultiDrawElementsIndirect for static world geometry
      // (will require a handful of other changes to mesh organization/data buffering)
      // @todo allow specific meshes to be associated with point lights + rendered to shadow maps
      for (auto* glMesh : glMeshes) {
        auto& mesh = *glMesh->getSourceMesh();

        // @todo handle foliage (requires point shadowcaster view shader updates)

        if (mesh.canCastShadows) {
          glMesh->render(ctx.primitiveMode, mesh.useLowestLevelOfDetailForShadows);
        }
      }

      glShadowMap.isRendered = true;
    }
  }

  /**
   * @todo description
   */
  void OpenGLRenderer::prepareLightingPass() {
    buffers.gBuffer.read();
    ctx.accumulationTarget->write();

    glViewport(0, 0, ctx.internalWidth, ctx.internalHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_LESS, MeshType::PARTICLES, 0xFF);
    glStencilMask(0x00);
  }

  /**
   * Applies indirect sky lighting if enabled, and copies depth
   * information from the G-Buffer to the accumulation buffer.
   */
  void OpenGLRenderer::renderLightingPrepass() {
    auto& shader = shaders.lightingPrepass;
    auto& scene = gmContext->scene;

    shader.use();
    shader.setVec4f("transform", FULL_SCREEN_TRANSFORM);
    shader.setInt("texColorAndDepth", 0);
    shader.setInt("texNormalAndMaterial", 1);
    shader.setVec3f("cameraPosition", ctx.activeCamera->position);
    shader.setMatrix4f("matInverseProjection", ctx.matInverseProjection);
    shader.setMatrix4f("matInverseView", ctx.matInverseView);

    shader.setVec3f("sunDirection", scene.sky.sunDirection);
    shader.setVec3f("sunColor", scene.sky.sunColor);
    shader.setVec3f("atmosphereColor", scene.sky.atmosphereColor);
    shader.setFloat("altitude", scene.sky.altitude);

    OpenGLScreenQuad::render();
  }

  /**
   * @todo description
   */
  void OpenGLRenderer::renderDirectionalLights() {
    auto& camera = *ctx.activeCamera;
    auto& shader = shaders.directionalLight;

    shader.use();
    shader.setVec4f("transform", FULL_SCREEN_TRANSFORM);
    shader.setInt("texColorAndDepth", 0);
    shader.setInt("texNormalAndMaterial", 1);
    shader.setVec3f("cameraPosition", camera.position);
    shader.setMatrix4f("matInverseProjection", ctx.matInverseProjection);
    shader.setMatrix4f("matInverseView", ctx.matInverseView);

    // @todo limit to MAX_DIRECTIONAL_LIGHTS
    for (u32 i = 0; i < ctx.directionalLights.size(); i++) {
      auto& light = *ctx.directionalLights[i];
      std::string indexedLight = "lights[" + std::to_string(i) + "]";

      shader.setVec3f(indexedLight + ".color", light.color);
      shader.setFloat(indexedLight + ".power", light.power);
      shader.setVec3f(indexedLight + ".direction", light.direction);
    }

    OpenGLScreenQuad::render();
  }

  /**
   * @todo description
   */
  void OpenGLRenderer::renderDirectionalShadowcasters() {
    auto& camera = *ctx.activeCamera;
    auto& shader = shaders.directionalShadowcaster;

    shader.use();
    shader.setFloat("zNear", gmContext->scene.zNear);
    shader.setFloat("zFar", gmContext->scene.zFar);

    for (u32 i = 0; i < ctx.directionalShadowcasters.size(); i++) {
      auto& glShadowMap = *glDirectionalShadowMaps[i];
      auto& light = *glShadowMap.light;

      glShadowMap.buffer.read();

      shader.setVec4f("transform", FULL_SCREEN_TRANSFORM);
      // @todo define an enum for reserved color attachment indexes
      shader.setInt("texColorAndDepth", 0);
      shader.setInt("texNormalAndMaterial", 1);
      shader.setInt("texShadowMaps[0]", 3);
      shader.setInt("texShadowMaps[1]", 4);
      shader.setInt("texShadowMaps[2]", 5);
      shader.setInt("texShadowMaps[3]", 6);
      shader.setMatrix4f("lightMatrices[0]", Gm_CreateCascadedLightViewProjectionMatrixGL(0, light.direction, camera));
      shader.setMatrix4f("lightMatrices[1]", Gm_CreateCascadedLightViewProjectionMatrixGL(1, light.direction, camera));
      shader.setMatrix4f("lightMatrices[2]", Gm_CreateCascadedLightViewProjectionMatrixGL(2, light.direction, camera));
      shader.setMatrix4f("lightMatrices[3]", Gm_CreateCascadedLightViewProjectionMatrixGL(3, light.direction, camera));
      shader.setVec3f("cameraPosition", camera.position);
      shader.setMatrix4f("matInverseProjection", ctx.matInverseProjection);
      shader.setMatrix4f("matInverseView", ctx.matInverseView);
      shader.setVec3f("light.color", light.color);
      shader.setFloat("light.power", light.power);
      shader.setVec3f("light.direction", light.direction);

      OpenGLScreenQuad::render();
    }
  }

  /**
   * @todo description
   */
  void OpenGLRenderer::renderSpotLights() {
    auto& camera = *ctx.activeCamera;
    auto& shader = shaders.spotLight;

    shader.use();
    shader.setInt("texColorAndDepth", 0);
    shader.setInt("texNormalAndMaterial", 1);
    shader.setVec3f("cameraPosition", camera.position);
    shader.setMatrix4f("matInverseProjection", ctx.matInverseProjection);
    shader.setMatrix4f("matInverseView", ctx.matInverseView);

    lightDisc.draw(ctx.spotLights, internalResolution, *ctx.activeCamera);
  }

  /**
   * @todo description
   */
  void OpenGLRenderer::renderSpotShadowcasters() {
    auto& camera = *ctx.activeCamera;
    auto& shader = shaders.spotShadowcaster;

    shader.use();
    shader.setInt("texColorAndDepth", 0);
    shader.setInt("texNormalAndMaterial", 1);
    shader.setInt("texShadowMap", 3);
    shader.setVec3f("cameraPosition", camera.position);
    shader.setMatrix4f("matInverseProjection", ctx.matInverseProjection);
    shader.setMatrix4f("matInverseView", ctx.matInverseView);
    shader.setFloat("time", gmContext->contextTime);

    for (u32 i = 0; i < ctx.spotShadowcasters.size(); i++) {
      auto& glShadowMap = *glSpotShadowMaps[i];
      auto& light = *glShadowMap.light;

      Matrix4f lightProjection = Matrix4f::glPerspective({ 1024, 1024 }, 120.0f, 1.0f, light.radius);
      Matrix4f lightView = Matrix4f::lookAt(light.position.gl(), light.direction.invert().gl(), Vec3f(0.0f, 1.0f, 0.0f));
      Matrix4f lightMatrix = (lightProjection * lightView).transpose();

      shader.setMatrix4f("lightMatrix", lightMatrix);

      glShadowMap.buffer.read();
      lightDisc.draw(light, internalResolution, *ctx.activeCamera);
    }
  }

  /**
   * @todo description
   */
  void OpenGLRenderer::renderPointLights() {
    auto& camera = *ctx.activeCamera;
    auto& shader = shaders.pointLight;

    shader.use();
    shader.setInt("texColorAndDepth", 0);
    shader.setInt("texNormalAndMaterial", 1);
    shader.setVec3f("cameraPosition", camera.position);
    shader.setMatrix4f("matInverseProjection", ctx.matInverseProjection);
    shader.setMatrix4f("matInverseView", ctx.matInverseView);

    lightDisc.draw(ctx.pointLights, internalResolution, *ctx.activeCamera);
  }

  /**
   * @todo description
   */
  void OpenGLRenderer::renderPointShadowcasters() {
    auto& camera = *ctx.activeCamera;
    auto& shader = shaders.pointShadowcaster;

    shader.use();
    shader.setInt("texColorAndDepth", 0);
    shader.setInt("texNormalAndMaterial", 1);
    shader.setInt("texShadowMap", 3);
    shader.setVec3f("cameraPosition", camera.position);
    shader.setMatrix4f("matInverseProjection", ctx.matInverseProjection);
    shader.setMatrix4f("matInverseView", ctx.matInverseView);

    for (u32 i = 0; i < ctx.pointShadowcasters.size(); i++) {
      auto& glShadowMap = *glPointShadowMaps[i];
      auto& light = *glShadowMap.light;

      glShadowMap.buffer.read();
      lightDisc.draw(light, internalResolution, *ctx.activeCamera);
    }
  }

  /**
   * @todo description
   */
  void OpenGLRenderer::copyEmissiveObjects() {
    // Only copy the color/depth frame where emissive
    // objects have been drawn into the G-Buffer
    glStencilFunc(GL_EQUAL, MeshType::EMISSIVE, 0xFF);

    auto& shader = shaders.copyFrame;

    shader.use();
    shader.setVec4f("transform", FULL_SCREEN_TRANSFORM);
    shader.setInt("texColorAndDepth", 0);

    OpenGLScreenQuad::render();

    // Restore the lighting stencil function, since we
    // may render indirect lighting after this
    glStencilFunc(GL_LESS, MeshType::PARTICLES, 0xFF);
  }

  /**
   * @todo description
   */
  void OpenGLRenderer::renderIndirectLight() {
    auto& currentIndirectLightBuffer = buffers.indirectLight[frame % 2];
    auto& previousIndirectLightBuffer = buffers.indirectLight[(frame + 1) % 2];

    if (
      Gm_IsFlagEnabled(GammaFlags::RENDER_AMBIENT_OCCLUSION) ||
      Gm_IsFlagEnabled(GammaFlags::RENDER_GLOBAL_ILLUMINATION)
    ) {
      buffers.gBuffer.read();
      ctx.accumulationTarget->read();

      // @todo OpenGLFrameBuffer::createMipmaps(u32 levels)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
      glGenerateMipmap(GL_TEXTURE_2D);

      previousIndirectLightBuffer.read();
      currentIndirectLightBuffer.write();

      glClear(GL_COLOR_BUFFER_BIT);

      shaders.indirectLight.use();

      #if GAMMA_DEVELOPER_MODE
        Vec2f screenSize((float)internalResolution.width, (float)internalResolution.height);

        shaders.indirectLight.setVec2f("screenSize", screenSize);
      #endif

      shaders.indirectLight.setVec4f("transform", FULL_SCREEN_TRANSFORM);
      shaders.indirectLight.setInt("texColorAndDepth", 0);
      shaders.indirectLight.setInt("texNormalAndMaterial", 1);
      shaders.indirectLight.setInt("texIndirectLightT1", 2);
      shaders.indirectLight.setVec3f("cameraPosition", ctx.activeCamera->position);
      shaders.indirectLight.setMatrix4f("matProjection", ctx.matProjection);
      shaders.indirectLight.setMatrix4f("matView", ctx.matView);
      shaders.indirectLight.setMatrix4f("matInverseProjection", ctx.matInverseProjection);
      shaders.indirectLight.setMatrix4f("matInverseView", ctx.matInverseView);
      shaders.indirectLight.setMatrix4f("matViewT1", ctx.matPreviousView);
      shaders.indirectLight.setInt("frame", gmContext->scene.frame);
      shaders.indirectLight.setFloat("zNear", gmContext->scene.zNear);
      shaders.indirectLight.setFloat("zFar", gmContext->scene.zFar);

      OpenGLScreenQuad::render();

      ctx.accumulationTarget->read();

      // @todo OpenGLFrameBuffer::setMaxMipmapLevel(u32 level)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    }

    // Composite the indirect lighting result into the accumulation target buffer
    buffers.gBuffer.read();
    currentIndirectLightBuffer.read();
    ctx.accumulationTarget->write();

    shaders.indirectLightComposite.use();

    #if GAMMA_DEVELOPER_MODE
      Vec2f screenSize((float)internalResolution.width, (float)internalResolution.height);

      shaders.indirectLightComposite.setVec2f("screenSize", screenSize);
    #endif

    shaders.indirectLightComposite.setVec4f("transform", FULL_SCREEN_TRANSFORM);
    shaders.indirectLightComposite.setInt("texColorAndDepth", 0);
    shaders.indirectLightComposite.setInt("texNormalAndMaterial", 1);
    shaders.indirectLightComposite.setInt("texIndirectLight", 2);
    shaders.indirectLightComposite.setFloat("zNear", gmContext->scene.zNear);
    shaders.indirectLightComposite.setFloat("zFar", gmContext->scene.zFar);

    glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE);

    OpenGLScreenQuad::render();

    glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);
  }

  /**
   * @todo description
   */
  void OpenGLRenderer::renderSkybox() {
    glStencilFunc(GL_EQUAL, MeshType::SKYBOX, 0xFF);

    auto& scene = gmContext->scene;

    if (ctx.cloudsTexture != nullptr) {
      ctx.cloudsTexture->bind();
    }

    shaders.skybox.use();
    shaders.skybox.setInt("texClouds", 3);
    shaders.skybox.setVec4f("transform", FULL_SCREEN_TRANSFORM);
    shaders.skybox.setVec3f("cameraPosition", ctx.activeCamera->position);
    shaders.skybox.setMatrix4f("matInverseProjection", ctx.matInverseProjection);
    shaders.skybox.setMatrix4f("matInverseView", ctx.matInverseView);
    shaders.skybox.setFloat("time", scene.sceneTime);
    shaders.skybox.setVec3f("sunDirection", scene.sky.sunDirection);
    shaders.skybox.setVec3f("sunColor", scene.sky.sunColor);
    shaders.skybox.setVec3f("atmosphereColor", scene.sky.atmosphereColor);
    shaders.skybox.setFloat("altitude", scene.sky.altitude);

    OpenGLScreenQuad::render();
  }

  /**
   * @todo description
   */
  void OpenGLRenderer::renderParticles() {
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glStencilFunc(GL_ALWAYS, MeshType::PARTICLES, 0xFF);
    glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE);
    glStencilMask(0xFF);

    // Render GPU particles
    {
      shaders.gpuParticle.use();
      shaders.gpuParticle.setMatrix4f("matProjection", ctx.matProjection);
      shaders.gpuParticle.setMatrix4f("matView", ctx.matView);
      shaders.gpuParticle.setFloat("time", gmContext->scene.sceneTime);

      for (auto& glMesh : glMeshes) {        
        auto& mesh = *glMesh->getSourceMesh();

        if (mesh.type == MeshType::PARTICLES && mesh.particles.useGpuParticles) {
          auto& particles = mesh.particles;

          // @optimize it would be preferable to use a UBO for particle systems,
          // and simply set the particle system ID uniform here. we're doing
          // too many uniform updates as things currently stand.

          // Set particle system parameters
          shaders.gpuParticle.setInt("particles.total", glMesh->getObjectCount());
          shaders.gpuParticle.setVec3f("particles.spawn", particles.spawn);
          shaders.gpuParticle.setFloat("particles.spread", particles.spread);
          shaders.gpuParticle.setFloat("particles.minimum_radius", particles.minimumRadius);
          shaders.gpuParticle.setFloat("particles.median_speed", particles.medianSpeed);
          shaders.gpuParticle.setFloat("particles.speed_variation", particles.speedVariation);
          shaders.gpuParticle.setFloat("particles.median_size", particles.medianSize);
          shaders.gpuParticle.setFloat("particles.size_variation", particles.sizeVariation);
          shaders.gpuParticle.setFloat("particles.deviation", particles.deviation);

          // Set particle path parameters
          constexpr static u32 MAX_PATH_POINTS = 10;
          u32 totalPathPoints = std::min((u32)particles.path.size(), (u32)MAX_PATH_POINTS);

          for (u8 i = 0; i < totalPathPoints; i++) {
            shaders.gpuParticle.setVec3f("path.points[" + std::to_string(i) + "]", particles.path[i]);
          }

          shaders.gpuParticle.setInt("path.total", totalPathPoints);
          shaders.gpuParticle.setBool("path.is_circuit", particles.isCircuit);

          glMesh->render(ctx.primitiveMode);
        }
      }
    }

    // Render instanced particle meshes
    {
      shaders.particle.use();
      shaders.particle.setMatrix4f("matProjection", ctx.matProjection);
      shaders.particle.setMatrix4f("matView", ctx.matView);

      for (auto& glMesh : glMeshes) {
        auto& mesh = *glMesh->getSourceMesh();

        if (mesh.type == MeshType::PARTICLES && !mesh.particles.useGpuParticles) {
          glMesh->render(ctx.primitiveMode);
        }
      }
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glStencilMask(MeshType::SKYBOX);
  }

  /**
   * @bug @todo this shader is broken since we don't pass sky uniforms in!!!!
   *
   * @todo description
   */
  void OpenGLRenderer::renderReflections() {
    if (
      ctx.hasRefractiveObjects &&
      Gm_IsFlagEnabled(GammaFlags::RENDER_REFRACTIVE_GEOMETRY) &&
      Gm_IsFlagEnabled(GammaFlags::RENDER_REFRACTIVE_GEOMETRY_WITHIN_REFLECTIONS)
    ) {
      // @todo fix + explain this
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_CULL_FACE);

      glStencilFunc(GL_NOTEQUAL, MeshType::REFLECTIVE, 0xFF);
      glStencilMask(MeshType::REFRACTIVE);

      shaders.refractivePrepass.use();

      #if GAMMA_DEVELOPER_MODE
        Vec2f screenSize((float)internalResolution.width, (float)internalResolution.height);

        shaders.refractivePrepass.setVec2f("screenSize", screenSize);
      #endif

      shaders.refractivePrepass.setInt("texColorAndDepth", 0);
      shaders.refractivePrepass.setMatrix4f("matViewProjection", ctx.matViewProjection);
      shaders.refractivePrepass.setFloat("zNear", gmContext->scene.zNear);
      shaders.refractivePrepass.setFloat("zFar", gmContext->scene.zFar);

      for (auto* glMesh : glMeshes) {
        if (glMesh->isMeshType(MeshType::REFRACTIVE)) {
          glMesh->render(ctx.primitiveMode);
        }
      }

      glDisable(GL_DEPTH_TEST);
      glDisable(GL_CULL_FACE);
    }

    auto& camera = *ctx.activeCamera;

    buffers.gBuffer.read();
    ctx.accumulationTarget->read();
    buffers.reflections.write();

    // Render reflections (screen-space + skybox)
    //
    // @todo allow controllable reflection parameters
    glStencilFunc(GL_EQUAL, MeshType::REFLECTIVE, 0xFF);

    shaders.reflections.use();
    shaders.reflections.setVec4f("transform", FULL_SCREEN_TRANSFORM);
    shaders.reflections.setInt("texColorAndDepth", 0);
    shaders.reflections.setInt("texNormalAndMaterial", 1);
    shaders.reflections.setVec3f("cameraPosition", camera.position);
    shaders.reflections.setMatrix4f("matView", ctx.matView);
    shaders.reflections.setMatrix4f("matInverseView", ctx.matInverseView);
    shaders.reflections.setMatrix4f("matProjection", ctx.matProjection);
    shaders.reflections.setMatrix4f("matInverseProjection", ctx.matInverseProjection);
    shaders.reflections.setFloat("zNear", gmContext->scene.zNear);
    shaders.reflections.setFloat("zFar", gmContext->scene.zFar);

    OpenGLScreenQuad::render();

    buffers.reflections.read();
    ctx.accumulationTarget->write();

    shaders.reflectionsDenoise.use();

    #if GAMMA_DEVELOPER_MODE
      Vec2f screenSize((float)internalResolution.width, (float)internalResolution.height);

      shaders.reflectionsDenoise.setVec2f("screenSize", screenSize);
    #endif

    shaders.reflectionsDenoise.setVec4f("transform", FULL_SCREEN_TRANSFORM);
    shaders.reflectionsDenoise.setInt("texColorAndDepth", 0);

    OpenGLScreenQuad::render();
  }

  /**
   * @todo description
   */
  void OpenGLRenderer::renderRefractiveGeometry() {
    auto& camera = *ctx.activeCamera;
    auto& scene = gmContext->scene;

    // Swap buffers so we can temporarily render the
    // refracted geometry to the second accumulation
    // buffer while reading from the first
    swapAccumulationBuffers();

    // At this point, the accumulation source buffer
    // will contain all effects/shading rendered up
    // to this point, which we can read from when
    // rendering refractions. Write to the second
    // accumulation buffer so we can copy that back
    // into the first afterward.
    ctx.accumulationSource->read();
    ctx.accumulationTarget->write();

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glStencilFunc(GL_ALWAYS, MeshType::REFRACTIVE, 0xFF);
    glStencilMask(0xFF);

    shaders.refractiveGeometry.use();

    #if GAMMA_DEVELOPER_MODE
      Vec2f screenSize((float)internalResolution.width, (float)internalResolution.height);

      shaders.refractiveGeometry.setVec2f("screenSize", screenSize);
    #endif

    shaders.refractiveGeometry.setInt("texColorAndDepth", 0);
    shaders.refractiveGeometry.setInt("meshNormalMap", 1);
    shaders.refractiveGeometry.setMatrix4f("matViewProjection", ctx.matViewProjection);
    shaders.refractiveGeometry.setMatrix4f("matProjection", ctx.matProjection);
    shaders.refractiveGeometry.setMatrix4f("matInverseProjection", ctx.matInverseProjection);
    shaders.refractiveGeometry.setMatrix4f("matView", ctx.matView);
    shaders.refractiveGeometry.setMatrix4f("matInverseView", ctx.matInverseView);
    shaders.refractiveGeometry.setVec3f("cameraPosition", camera.position);

    shaders.refractiveGeometry.setVec3f("sunDirection", scene.sky.sunDirection);
    shaders.refractiveGeometry.setVec3f("sunColor", scene.sky.sunColor);
    shaders.refractiveGeometry.setVec3f("atmosphereColor", scene.sky.atmosphereColor);
    shaders.refractiveGeometry.setFloat("altitude", scene.sky.altitude);

    for (auto* glMesh : glMeshes) {
      if (glMesh->isMeshType(MeshType::REFRACTIVE)) {
        shaders.refractiveGeometry.setBool("hasNormalMap", glMesh->hasNormalMap());

        glMesh->render(ctx.primitiveMode);
      }
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Now that the current target accumulation buffer contains
    // the rendered refractive geometry, swap the buffers so we
    // can write refractions back into the original target
    // accumulation buffer
    swapAccumulationBuffers();

    ctx.accumulationSource->read();
    ctx.accumulationTarget->write();

    glStencilFunc(GL_EQUAL, MeshType::REFRACTIVE, 0xFF);

    shaders.copyFrame.use();
    shaders.copyFrame.setVec4f("transform", FULL_SCREEN_TRANSFORM);
    shaders.copyFrame.setInt("texColorAndDepth", 0);

    OpenGLScreenQuad::render();
  }

  /**
   * @todo description
   */
  void OpenGLRenderer::renderOcean() {
    auto& camera = *ctx.activeCamera;
    auto& scene = gmContext->scene;

    // Swap buffers so we can temporarily render the
    // refracted geometry to the second accumulation
    // buffer while reading from the first
    swapAccumulationBuffers();

    // At this point, the accumulation source buffer
    // will contain all effects/shading rendered up
    // to this point, which we can read from when
    // rendering refractions. Write to the second
    // accumulation buffer so we can copy that back
    // into the first afterward.
    ctx.accumulationSource->read();
    ctx.accumulationTarget->write();

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glStencilFunc(GL_ALWAYS, MeshType::OCEAN, 0xFF);
    glStencilMask(0xFF);

    shaders.ocean.use();

    #if GAMMA_DEVELOPER_MODE
      Vec2f screenSize((float)internalResolution.width, (float)internalResolution.height);

      shaders.ocean.setVec2f("screenSize", screenSize);
    #endif

    // If there are any directional shadowcasters, use the shadow map
    // and light view/projection matrix for the first to check for shadowed
    // areas on the ocean surface. In the ocean shader, we artificially lower
    // the sky intensity in shadowed areas to avoid sunlight and clouds
    // being erroneously reflected at full intensity.
    if (ctx.directionalShadowcasters.size() > 0) {
      auto* light = ctx.directionalShadowcasters[0];
      auto* glShadowMap = glDirectionalShadowMaps[0];

      glShadowMap->buffer.read();

      // Use the fourth shadow cascade by default, since we don't need the
      // edges of the shadowed area to be smooth, and the fourth cascade
      // covers the widest area/depth range
      auto matLightViewProjection = Gm_CreateCascadedLightViewProjectionMatrixGL(3, light->direction, *ctx.activeCamera);

      shaders.ocean.setInt("texShadowMap", 6);
      shaders.ocean.setMatrix4f("matLightViewProjection", matLightViewProjection);
    }

    if (ctx.cloudsTexture != nullptr) {
      ctx.cloudsTexture->bind();
    }

    shaders.ocean.setInt("texColorAndDepth", 0);
    shaders.ocean.setInt("texClouds", 3);
    shaders.ocean.setMatrix4f("matViewProjection", ctx.matViewProjection);
    shaders.ocean.setMatrix4f("matProjection", ctx.matProjection);
    shaders.ocean.setMatrix4f("matInverseProjection", ctx.matInverseProjection);
    shaders.ocean.setMatrix4f("matView", ctx.matView);
    shaders.ocean.setMatrix4f("matInverseView", ctx.matInverseView);
    shaders.ocean.setVec3f("cameraPosition", camera.position);
    shaders.ocean.setFloat("time", scene.sceneTime);
    shaders.ocean.setFloat("zNear", scene.zNear);
    shaders.ocean.setFloat("zFar", scene.zFar);
    shaders.ocean.setVec3f("sunDirection", scene.sky.sunDirection);
    shaders.ocean.setVec3f("sunColor", scene.sky.sunColor);
    shaders.ocean.setVec3f("atmosphereColor", scene.sky.atmosphereColor);
    shaders.ocean.setFloat("altitude", scene.sky.altitude);

    for (auto* glMesh : glMeshes) {
      if (glMesh->isMeshType(MeshType::OCEAN)) {
        shaders.ocean.setFloat("turbulence", glMesh->getSourceMesh()->ocean.turbulence);

        glMesh->render(ctx.primitiveMode);
      }
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Now that the current target accumulation buffer contains
    // the rendered ocean objects, swap the buffers so we can
    // write them back into the original target accumulation buffer
    swapAccumulationBuffers();

    ctx.accumulationSource->read();
    ctx.accumulationTarget->write();

    glStencilFunc(GL_EQUAL, MeshType::OCEAN, 0xFF);

    shaders.copyFrame.use();
    shaders.copyFrame.setVec4f("transform", FULL_SCREEN_TRANSFORM);
    shaders.copyFrame.setInt("texColorAndDepth", 0);

    OpenGLScreenQuad::render();
  }

  void OpenGLRenderer::renderSilhouettes() {
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);
    glStencilFunc(GL_LESS, MeshType::DEFAULT_WITH_OCCLUSION_SILHOUETTE, 0xFF);

    shaders.silhouette.use();
    shaders.silhouette.setMatrix4f("matViewProjection", ctx.matViewProjection);
    shaders.silhouette.setInt("meshTexture", 0);

    for (auto* glMesh : glMeshes) {
      if (glMesh->getSourceMesh()->type == MeshType::DEFAULT_WITH_OCCLUSION_SILHOUETTE) {
        glMesh->render(ctx.primitiveMode);
      }
    }

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
  }

  /**
   * @todo description
   */
  void OpenGLRenderer::renderPostEffects() {
    buffers.gBuffer.read();
    ctx.accumulationSource->read();

    // @todo possibly use nearest-neighbor accumulation buffer filtering combined with FXAA
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    if (Gm_IsFlagEnabled(GammaFlags::RENDER_DEPTH_OF_FIELD)) {
      // @todo OpenGLFrameBuffer::createMipmaps(u32 levels)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
      glGenerateMipmap(GL_TEXTURE_2D);
    }

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glViewport(0, 0, gmContext->window.size.width, gmContext->window.size.height);
    glDisable(GL_STENCIL_TEST);

    auto& scene = gmContext->scene;

    shaders.post.use();
    shaders.post.setVec4f("transform", FULL_SCREEN_TRANSFORM);
    shaders.post.setInt("texColorAndDepth", 0);
    shaders.post.setInt("texNormalAndMaterial", 1);
    shaders.post.setMatrix4f("matInverseProjection", ctx.matInverseProjection);
    shaders.post.setMatrix4f("matInverseView", ctx.matInverseView);
    shaders.post.setVec3f("cameraPosition", ctx.activeCamera->position);
    shaders.post.setFloat("screenWarpTime", scene.sceneTime - scene.fx.screenWarpTime);
    shaders.post.setVec3f("atmosphereColor", scene.sky.atmosphereColor);
    shaders.post.setFloat("zNear", scene.zNear);
    shaders.post.setFloat("zFar", scene.zFar);
    shaders.post.setFloat("time", scene.sceneTime);

    // Game-specific modifications
    {
      shaders.post.setVec3f("redshiftSpawn", scene.fx.redshiftSpawn);
      shaders.post.setFloat("redshiftInProgress", scene.fx.redshiftInProgress);
      shaders.post.setFloat("redshiftOutProgress", scene.fx.redshiftOutProgress);
      shaders.post.setFloat("dashRainbowIntensity", scene.fx.dashRainbowIntensity);
    }

    OpenGLScreenQuad::render();
  }

  /**
   * @todo description
   */
  void OpenGLRenderer::renderDevBuffers() {
    buffers.gBuffer.read();

    shaders.gBufferDev.use();
    shaders.gBufferDev.setInt("texColorAndDepth", 0);
    shaders.gBufferDev.setInt("texNormalAndMaterial", 1);
    shaders.gBufferDev.setFloat("zNear", gmContext->scene.zNear);
    shaders.gBufferDev.setFloat("zFar", gmContext->scene.zFar);
    shaders.gBufferDev.setVec4f("transform", { 0.53f, 0.82f, 0.43f, 0.11f });

    OpenGLScreenQuad::render();

    for (u32 i = 0; i < glDirectionalShadowMaps.size(); i++) {
      float yOffset = 0.52f - float(i) * 0.32f;

      glDirectionalShadowMaps[i]->buffer.read();

      shaders.directionalShadowMapDev.use();
      shaders.directionalShadowMapDev.setInt("texCascade0", 3);
      shaders.directionalShadowMapDev.setInt("texCascade1", 4);
      shaders.directionalShadowMapDev.setInt("texCascade2", 5);
      shaders.directionalShadowMapDev.setInt("texCascade3", 6);
      shaders.directionalShadowMapDev.setVec4f("transform", { 0.53f, yOffset, 0.43f, 0.11f });

      OpenGLScreenQuad::render();
    }

    // @todo point light shadow maps?
    // @todo spot light shadow maps?
  }

  void OpenGLRenderer::createMesh(Mesh* mesh) {
    glMeshes.push_back(new OpenGLMesh(mesh));

    #if GAMMA_DEVELOPER_MODE
      // @todo move to OpenGLMesh
      u32 totalVertices = mesh->vertices.size();
      u32 totalTriangles = mesh->faceElements.size() / 3;

      Console::log("[Gamma] OpenGLMesh created:", totalVertices, "vertices,", totalTriangles, "triangles");
    #endif
  }

  void OpenGLRenderer::createShadowMap(Light* light) {
    switch (light->type) {
      case LightType::DIRECTIONAL_SHADOWCASTER:
        glDirectionalShadowMaps.push_back(new OpenGLDirectionalShadowMap(light));
        break;
      case LightType::POINT_SHADOWCASTER:
        glPointShadowMaps.push_back(new OpenGLPointShadowMap(light));
        break;
      case LightType::SPOT_SHADOWCASTER:
        glSpotShadowMaps.push_back(new OpenGLSpotShadowMap(light));
        break;
    }
  }

  void OpenGLRenderer::destroyMesh(Mesh* mesh) {
    OpenGLMesh* targetGlMesh = nullptr;

    for (auto* glMesh : glMeshes) {
      if (glMesh->getSourceMesh() == mesh) {
        targetGlMesh = glMesh;

        break;
      }
    }

    Gm_VectorRemove(glMeshes, targetGlMesh);

    delete targetGlMesh;

    Console::log("[Gamma] Mesh destroyed!");
  }

  void OpenGLRenderer::destroyShadowMap(Light* light) {
    #define clear_light_from(shadowMaps, light) \
      for (auto& shadowMap : shadowMaps) {\
        if (shadowMap->light == light) {\
          shadowMap->buffer.destroy();\
          Gm_VectorRemove(shadowMaps, shadowMap);\
          break;\
        }\
      }\

    // @todo test this to make sure it works!
    switch (light->type) {
      case DIRECTIONAL_SHADOWCASTER:
        clear_light_from(glDirectionalShadowMaps, light);

        break;
      case POINT_SHADOWCASTER:
        clear_light_from(glPointShadowMaps, light);

        break;
      case SPOT_SHADOWCASTER:
        clear_light_from(glSpotShadowMaps, light);

        break;
    }

    Console::log("[Gamma] Shadowcaster destroyed!");
  }

  void OpenGLRenderer::destroyProbe(const std::string& name) {
    auto* cubeMap = glProbes.at(name);

    cubeMap->destroy();

    delete cubeMap;

    glProbes.erase(name);
  }

  const RenderStats& OpenGLRenderer::getRenderStats() {
    GLint total = 0;
    GLint available = 0;
    const char* vendor = (const char*)glGetString(GL_VENDOR);

    if (strcmp(vendor, "NVIDIA Corporation") == 0) {
      glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &total);
      glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &available);
    }

    stats.gpuMemoryTotal = total / 1000;
    stats.gpuMemoryUsed = (total - available) / 1000;
    stats.totalDrawCalls = OpenGLMesh::totalDrawCalls + OpenGLScreenQuad::totalDrawCalls + OpenGLLightDisc::totalDrawCalls;
    stats.isVSynced = SDL_GL_GetSwapInterval() == 1;

    return stats;
  }

  void OpenGLRenderer::present() {
    SDL_GL_SwapWindow(gmContext->window.sdl_window);
  }

  void OpenGLRenderer::createAndRenderProbe(const std::string& name, const Vec3f& position) {
    auto probe = new OpenGLCubeMap();

    probe->init();
    // @todo configurable probe size
    probe->setSize({ 1024, 1024 });
    probe->addColorAttachment(ColorFormat::RGB16, 3);
    probe->bindColorAttachments();

    glProbes[name] = probe;

    std::vector<u8> order = { 1, 0, 3, 2, 5, 4 };

    for (u8 i = 0; i < 6; i++) {
      auto& direction = CUBE_MAP_DIRECTIONS[order[i]];
      auto& upDirection = CUBE_MAP_UP_DIRECTIONS[order[i]];
      float farDistance = 1000.0f;

      Camera probeCamera;

      probeCamera.position = position;
      probeCamera.orientation.face(direction, upDirection);

      Matrix4f matProjection = Matrix4f::glPerspective({ 1024, 1024 }, 90.f, 1.f, farDistance).transpose();

      if (i == 2 || i == 3) {
        // @hack @todo there may be a bug in Orientation::face()
        probeCamera.orientation.yaw += Gm_PI / 2.f;
      }

      probeCamera.rotation = probeCamera.orientation.toQuaternion();

      Matrix4f matView = (
        probeCamera.rotation.toMatrix4f() *
        // Matrix4f::rotation(probeCamera.orientation) *
        Matrix4f::translation(probeCamera.position.invert().gl())
      ).transpose();

      ctx.activeCamera = &probeCamera;
      ctx.matProjection = matProjection;
      ctx.matView = matView;
      ctx.matPreviousView = matView;
      ctx.matInverseProjection = ctx.matProjection.inverse();
      ctx.matInverseView = ctx.matView.inverse();

      renderToAccumulationBuffer();

      ctx.accumulationSource->read();
      probe->writeToFace(i);

      shaders.copyFrame.use();
      shaders.copyFrame.setVec4f("transform", { 0.0f, 0.0f, -1.0f, 1.0f });
      shaders.copyFrame.setInt("texColorAndDepth", 0);

      OpenGLScreenQuad::render();
    }
  }

  void OpenGLRenderer::renderSurface(SDL_Surface* surface, u32 x, u32 y, u32 w, u32 h, const Vec3f& color, const Vec4f& background) {
    auto& window = gmContext->window;
    float offsetX = -1.0f + (2 * x + w) / (float)window.size.width;
    float offsetY = 1.0f - (2 * y + h) / (float)window.size.height;
    float scaleX = w / (float)window.size.width;
    float scaleY = -1.0f * h / (float)window.size.height;
    int format = surface->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    screen.use();
    screen.setVec4f("transform", { offsetX, offsetY, scaleX, scaleY });
    screen.setVec3f("color", color);
    screen.setVec4f("background", background);

    OpenGLScreenQuad::render();
  }

  void OpenGLRenderer::renderText(TTF_Font* font, const char* message, u32 x, u32 y, const Vec3f& color, const Vec4f& background) {
    SDL_Surface* text = TTF_RenderText_Blended_Wrapped(font, message, { 255, 255, 255 }, gmContext->window.size.width);

    // @todo support scaling
    renderSurface(text, x, y, text->w, text->h, color, background);

    SDL_FreeSurface(text);
  }

  void OpenGLRenderer::resetShadowMaps() {
    for (auto* glShadowMap : glSpotShadowMaps) {
      glShadowMap->isRendered = false;
    }

    for (auto* glShadowMap : glPointShadowMaps) {
      glShadowMap->isRendered = false;
    }
  }

  void OpenGLRenderer::swapAccumulationBuffers() {
    OpenGLFrameBuffer* source = ctx.accumulationSource;

    ctx.accumulationSource = ctx.accumulationTarget;
    ctx.accumulationTarget = source;
  }
}