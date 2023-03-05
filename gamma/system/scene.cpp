#include <filesystem>

#include "math/utilities.h"
#include "system/scene.h"
#include "system/assert.h"
#include "system/console.h"
#include "system/context.h"
#include "system/flags.h"
#include "system/vector_helpers.h"
#include "system/yaml_parser.h"

using namespace Gamma;

const GmSceneStats Gm_GetSceneStats(GmContext* context) {
  GmSceneStats stats;

  for (auto* mesh : context->scene.meshes) {
    if (mesh->disabled || mesh->objects.totalVisible() == 0) {
      continue;
    }

    if (mesh->lods.size() > 0) {
      for (auto& lod : mesh->lods) {
        stats.verts += lod.vertexCount * lod.instanceCount;
        stats.tris += (lod.elementCount / 3) * lod.instanceCount;
      }
    } else {
      stats.verts += mesh->vertices.size() * mesh->objects.totalVisible();
      stats.tris += (mesh->faceElements.size() / 3) * mesh->objects.totalVisible();
    }

    stats.totalMeshes++;
  }

  for (auto* light : context->scene.lights) {
    if (light->power > 0.f) {
      stats.totalLights++;
    }
  }

  return stats;
}

void Gm_AddMesh(GmContext* context, const std::string& meshName, u16 maxInstances, Gamma::Mesh* mesh) {
  auto& scene = context->scene;
  auto& meshes = scene.meshes;
  auto& meshMap = scene.meshMap;

  assert(meshMap.find(meshName) == meshMap.end(), "Mesh '" + meshName + "' already exists!");

  mesh->index = (u16)meshes.size();
  mesh->name = meshName;
  mesh->objects.reserve(maxInstances);

  meshMap.emplace(meshName, mesh);
  meshes.push_back(mesh);

  if (mesh->type == MeshType::PARTICLES && mesh->particles.useGpuParticles) {
    for (u16 i = 0; i < maxInstances; i++) {
      Gm_CreateObjectFrom(context, meshName);
    }
  }

  context->renderer->createMesh(mesh);
}

void Gm_AddProbe(GmContext* context, const std::string& probeName, const Gamma::Vec3f& position) {
  context->scene.probeMap.emplace(probeName, position);
}

Gamma::Light& Gm_CreateLight(GmContext* context, Gamma::LightType type) {
  auto& lights = context->scene.lights;

  lights.push_back(new Light());

  auto& light = *lights.back();

  light.type = type;

  if (
    type == LightType::POINT_SHADOWCASTER ||
    type == LightType::DIRECTIONAL_SHADOWCASTER ||
    type == LightType::SPOT_SHADOWCASTER
  ) {
    context->renderer->createShadowMap(&light);
  }

  return light;
}

void Gm_UseSceneFile(GmContext* context, const std::string& filename) {
  auto& scene = Gm_ParseYamlFile(filename.c_str());

  // Load meshes
  for (auto& [ key, property ] : *scene["meshes"].object) {
    auto& meshConfig = *property.object;
    u32 maxInstances = Gm_ReadYamlProperty<u32>(meshConfig, "max");
    Mesh* mesh = nullptr;

    if (Gm_HasYamlProperty(meshConfig, "plane")) {
      u32 size = Gm_ReadYamlProperty<u32>(meshConfig, "plane.size");
      bool useLoopingTexture = Gm_ReadYamlProperty<u32>(meshConfig, "plane.useLoopingTexture");

      mesh = Mesh::Plane(size, useLoopingTexture);
    } else if (Gm_HasYamlProperty(meshConfig, "cube")) {
      mesh = Mesh::Cube();
    } else if (Gm_HasYamlProperty(meshConfig, "model")) {
      std::vector<std::string> filepaths;
      auto paths = Gm_ReadYamlProperty<YamlArray<std::string*>>(meshConfig, "model");

      for (auto* path : paths) {
        filepaths.push_back(*path);
      }

      mesh = Mesh::Model(filepaths);
    } else if (Gm_HasYamlProperty(meshConfig, "particles")) {
      // @todo
    }

    // if mesh == nullptr, report mesh name missing type

    if (mesh != nullptr) {
      if (Gm_HasYamlProperty(meshConfig, "texture")) {
        mesh->texture = Gm_ReadYamlProperty<std::string>(meshConfig, "texture");
      }

      if (Gm_HasYamlProperty(meshConfig, "normals")) {
        mesh->normals = Gm_ReadYamlProperty<std::string>(meshConfig, "normals");
      }

      if (Gm_HasYamlProperty(meshConfig, "type")) {
        std::string type = Gm_ReadYamlProperty<std::string>(meshConfig, "type");

        // @todo use a map
        if (type == "REFRACTIVE") {
          mesh->type = MeshType::REFRACTIVE;
        } else if (type == "REFLECTIVE") {
          mesh->type = MeshType::REFLECTIVE;
        } else if (type == "PROBE_REFLECTOR") {
          mesh->type = MeshType::PROBE_REFLECTOR;
          mesh->probe = Gm_ReadYamlProperty<std::string>(meshConfig, "probe");
        }
      }

      Gm_AddMesh(context, key, maxInstances, mesh);
    }
  }

  // @todo skybox settings, what else?

  Gm_FreeYamlObject(&scene);
}

// @todo refactor with Gm_CreateObjectFrom(GmContext*, u16)
Gamma::Object& Gm_CreateObjectFrom(GmContext* context, const std::string& meshName) {
  auto& meshMap = context->scene.meshMap;

  assert(meshMap.find(meshName) != meshMap.end(), "Mesh '" + meshName + "' not found");

  auto& mesh = *meshMap.at(meshName);
  auto& object = mesh.objects.createObject();

  object._record.meshIndex = mesh.index;
  object.position = Vec3f(0.0f);
  object.rotation = Quaternion(1.f, 0, 0, 0);
  object.scale = Vec3f(1.0f);

  if (mesh.lods.size() > 0) {
    // Increment the base LoD instance count by default,
    // which we use for vert/tri counts in scene stats.
    // Gm_UseLodByDistance can control the per-LoD
    // instance counts during the update loop.
    mesh.lods[0].instanceCount++;
  }

  return object;
}

// @todo refactor with Gm_CreateObjectFrom(GmContext*, const std::string&)
Gamma::Object& Gm_CreateObjectFrom(GmContext* context, u16 meshIndex) {
  assert(context->scene.meshes.size() > meshIndex, "Mesh '" + std::to_string(meshIndex) + "' not found");

  auto& mesh = *context->scene.meshes[meshIndex];
  auto& object = mesh.objects.createObject();

  object._record.meshIndex = meshIndex;
  object.position = Vec3f(0.0f);
  object.rotation = Quaternion(1.f, 0, 0, 0);
  object.scale = Vec3f(1.0f);

  if (mesh.lods.size() > 0) {
    // Increment the base LoD instance count by default,
    // which we use for vert/tri counts in scene stats.
    // Gm_UseLodByDistance can control the per-LoD
    // instance counts during the update loop.
    mesh.lods[0].instanceCount++;
  }

  return object;
}

void Gm_Commit(GmContext* context, const Gamma::Object& object) {
  auto& meshes = context->scene.meshes;
  auto& record = object._record;
  auto* mesh = meshes[record.meshIndex];

  // @todo (?) dispatch transform commands to separate buckets for multithreading
  mesh->objects.transformById(record.id, Matrix4f::transformation(
    object.position,
    object.scale,
    object.rotation
  ).transpose());

  mesh->objects.setColorById(record.id, object.color);
}

Gamma::ObjectPool& Gm_GetObjects(GmContext* context, const std::string& meshName) {
  // @todo #if GAMMA_DEVELOPER_MODE
  Gamma::assert(context->scene.meshMap.find(meshName) != context->scene.meshMap.end(), "Mesh '" + meshName + "' not found");

  return context->scene.meshMap[meshName]->objects;
}

void Gm_SaveObject(GmContext* context, const std::string& objectName, const Gamma::Object& object) {
  context->scene.objectStore[objectName] = object._record;
}

void Gm_SaveLight(GmContext* context, const std::string& lightName, Gamma::Light* light) {
  context->scene.lightStore[lightName] = light;
}

bool Gm_HasObject(GmContext* context, const std::string& objectName) {
  return Gm_FindObject(context, objectName) != nullptr;
}

Gamma::Object* Gm_FindObject(GmContext* context, const std::string& objectName) {
  auto& scene = context->scene;
  auto& store = scene.objectStore;

  if (store.find(objectName) == store.end()) {
    return nullptr;
  }

  auto& record = store.at(objectName);
  auto& mesh = scene.meshes[record.meshIndex];

  return mesh->objects.getByRecord(record);
}

Gamma::Object& Gm_GetObject(GmContext* context, const std::string& objectName) {
  auto& scene = context->scene;
  // @todo assert that the object exists
  auto& record = scene.objectStore.at(objectName);
  auto& mesh = scene.meshes[record.meshIndex];

  return *mesh->objects.getByRecord(record);
}

Gamma::Object* Gm_GetObjectByRecord(GmContext* context, const Gamma::ObjectRecord& record) {
  auto& meshes = context->scene.meshes;

  if (record.meshIndex > meshes.size() - 1) {
    return nullptr;
  }

  return meshes[record.meshIndex]->objects.getByRecord(record);
}

Gamma::Light& Gm_GetLight(GmContext* context, const std::string& lightName) {
  auto& lightStore = context->scene.lightStore;
  // @todo assert that the light exists

  return *lightStore.at(lightName);
}

void Gm_RemoveObject(GmContext* context, const Gamma::Object& object) {
  auto& record = object._record;
  auto& mesh = context->scene.meshes[record.meshIndex];

  mesh->objects.removeById(record.id);
}

void Gm_RemoveLight(GmContext* context, Gamma::Light* light) {
  auto& scene = context->scene;
  auto& renderer = context->renderer;

  renderer->destroyShadowMap(light);

  Gm_VectorRemove(scene.lights, light);

  delete light;
}

// @incomplete (needs testing)
void Gm_ResetScene(GmContext* context) {
  auto& scene = context->scene;

  for (auto* mesh : scene.meshes) {
    context->renderer->destroyMesh(mesh);

    mesh->objects.free();

    delete mesh;
  }

  for (auto* light : scene.lights) {
    if (
      light->type == LightType::DIRECTIONAL_SHADOWCASTER ||
      light->type == LightType::POINT_SHADOWCASTER ||
      light->type == LightType::SPOT_SHADOWCASTER
    ) {
      context->renderer->destroyShadowMap(light);
    }

    delete light;
  }

  for (auto& [ name, position ] : scene.probeMap) {
    context->renderer->destroyProbe(name);
  }

  scene.meshes.clear();
  scene.lights.clear();
  scene.meshMap.clear();
  scene.probeMap.clear();
  scene.objectStore.clear();
  scene.lightStore.clear();

  scene.frame = 0;
  scene.sceneTime = 0.f;
}

void Gm_PointCameraAt(GmContext* context, const Gamma::Object& object, bool upsideDown) {
  Gm_PointCameraAt(context, object.position, upsideDown);
}

void Gm_PointCameraAt(GmContext* context, const Gamma::Vec3f& position, bool upsideDown) {
  auto& camera = context->scene.camera;
  Vec3f forward = (position - camera.position).unit();
  Vec3f sideways = Vec3f::cross(forward, Vec3f(0, 1.0f, 0));

  Vec3f up = upsideDown
    ? Vec3f::cross(forward, sideways)
    : Vec3f::cross(sideways, forward);

  camera.orientation.face(forward, up);

  camera.rotation = camera.orientation.toQuaternion();
}

void Gm_SmoothlyPointCameraAt(GmContext* context, const Gamma::Object& object, float alpha, bool upsideDown) {
  Gm_SmoothlyPointCameraAt(context, object.position, alpha, upsideDown);
}

void Gm_SmoothlyPointCameraAt(GmContext* context, const Gamma::Vec3f& position, float alpha, bool upsideDown) {
  auto& camera = context->scene.camera;
  Vec3f forward = (position - camera.position).unit();
  Vec3f sideways = Vec3f::cross(forward, Vec3f(0, 1.0f, 0));

  Vec3f up = upsideDown
    ? Vec3f::cross(forward, sideways)
    : Vec3f::cross(sideways, forward);
  
  auto currentOrientation = camera.orientation;

  camera.orientation.face(forward, up);

  camera.orientation.roll = Gm_Lerpf(currentOrientation.roll, camera.orientation.roll, alpha);
  camera.orientation.pitch = Gm_Lerpf(currentOrientation.pitch, camera.orientation.pitch, alpha);
  camera.orientation.yaw = Gm_LerpCircularf(currentOrientation.yaw, camera.orientation.yaw, alpha, Gm_PI);
  camera.orientation.yaw = Gm_Modf(camera.orientation.yaw, Gm_TAU);

  camera.rotation = camera.orientation.toQuaternion();
}

void Gm_HandleFreeCameraMode(GmContext* context, float speed, float dt) {
  auto& scene = context->scene;
  auto& camera = scene.camera;
  auto& input = scene.input;
  const auto& orientation = camera.orientation;
  Vec3f direction;

  if (input.isKeyHeld(Key::A)) {
    direction += orientation.getLeftDirection();
  } else if (input.isKeyHeld(Key::D)) {
    direction += orientation.getRightDirection();
  }

  if (input.isKeyHeld(Key::W)) {
    direction += orientation.getDirection();
  } else if (input.isKeyHeld(Key::S)) {
    direction += orientation.getDirection().invert();
  }

  if (direction.magnitude() > 0.f) {
    scene.freeCameraVelocity += direction.unit() * speed * dt;
  }

  camera.position += scene.freeCameraVelocity * dt;

  scene.freeCameraVelocity *= (0.995f - dt * 6.0f);

  if (scene.freeCameraVelocity.magnitude() < 0.1f) {
    scene.freeCameraVelocity = Vec3f(0.0f);
  }
}

void Gm_UseFrustumCulling(GmContext* context, const std::initializer_list<std::string>& meshNames) {
  auto& meshMap = context->scene.meshMap;

  for (auto& meshName : meshNames) {
    meshMap[meshName]->objects.partitionByVisibility(context->scene.camera);
  }
}

void Gm_UseLodByDistance(GmContext* context, float distance, const std::initializer_list<std::string>& meshNames) {
  auto& meshMap = context->scene.meshMap;
  auto& camera = context->scene.camera;

  for (auto& meshName : meshNames) {
    auto& mesh = *meshMap[meshName];

    u32 instanceOffset = 0;

    for (u32 lodIndex = 0; lodIndex < mesh.lods.size(); lodIndex++) {
      mesh.lods[lodIndex].instanceOffset = instanceOffset;

      if (lodIndex < mesh.lods.size() - 1) {
        // Group all objects within the distance threshold
        // in front of those outside it, and use the pivot
        // defining that boundary to determine our instance
        // count for this LoD set
        instanceOffset = (u32)mesh.objects.partitionByDistance((u16)instanceOffset, distance * float(lodIndex + 1), camera.position);

        mesh.lods[lodIndex].instanceCount = instanceOffset - mesh.lods[lodIndex].instanceOffset;
      } else {
        // The final LoD can just use the remaining set
        // of objects beyond the last LoD distance threshold
        mesh.lods[lodIndex].instanceCount = (u32)mesh.objects.totalVisible() - instanceOffset;
      }
    }
  }
}

void Gm_RenderImage(GmContext* context, SDL_Surface* image, u32 x, u32 y, u32 w, u32 h) {
  context->scene.ui.surfaces.push_back({ image, x, y, w, h });
}

void Gm_RenderText(GmContext* context, TTF_Font* font, std::string text, u32 x, u32 y) {
  context->scene.ui.texts.push_back({ font, text, x, y });
}