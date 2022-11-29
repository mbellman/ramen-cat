#pragma once

#include <filesystem>
#include <functional>
#include <initializer_list>
#include <map>
#include <string>
#include <vector>

#include "system/camera.h"
#include "system/entities.h"
#include "system/InputSystem.h"
#include "system/Signaler.h"
#include "system/traits.h"
#include "system/type_aliases.h"

#define addMesh(meshName, maxInstances, mesh) Gm_AddMesh(context, meshName, maxInstances, mesh)
#define addProbe(probeName, position) Gm_AddProbe(context, probeName, position)
#define createLight(type) Gm_CreateLight(context, type)
#define createObjectFrom(meshName) Gm_CreateObjectFrom(context, meshName)
#define commit(object) Gm_Commit(context, object)
#define saveObject(objectName, object) Gm_SaveObject(context, objectName, object)
#define saveLight(lightName, light) Gm_SaveLight(context, lightName, light)
#define hasObject(objectName) Gm_HasObject(context, objectName)
#define findObject(objectName) Gm_FindObject(context, objectName)
#define object(objectName) Gm_GetObject(context, objectName)
#define light(lightName) Gm_GetLight(context, lightName)
#define removeObject(object) Gm_RemoveObject(context, object)
#define removeLight(light) Gm_RemoveLight(context, light)
#define mesh(meshName) context->scene.meshMap.at(meshName)
#define objects(meshName) Gm_GetObjects(context, meshName)
#define pointCameraAt(...) Gm_PointCameraAt(context, __VA_ARGS__)
#define useFrustumCulling(...) Gm_UseFrustumCulling(context, __VA_ARGS__)
#define useLodByDistance(distance, ...) Gm_UseLodByDistance(context, distance, __VA_ARGS__)

#define getInput() context->scene.input
#define getCamera() context->scene.camera
#define getRunningTime() context->scene.runningTime

#define addDebugMessage(message) context->debugMessages.push_back(message)

struct GmContext;

struct GmSceneStats {
  u32 verts = 0;
  u32 tris = 0;
};

struct GmScene {
  Gamma::Camera camera;
  Gamma::InputSystem input;
  std::vector<Gamma::Mesh*> meshes;
  std::vector<Gamma::Light*> lights;
  std::map<std::string, Gamma::Mesh*> meshMap;
  std::map<std::string, Gamma::Vec3f> probeMap;
  std::map<std::string, Gamma::ObjectRecord> objectStore;
  // @todo when recycling a light, its lightStore entry should be removed
  std::map<std::string, Gamma::Light*> lightStore;
  Gamma::Vec3f freeCameraVelocity = Gamma::Vec3f(0.0f);
  u16 runningMeshId = 0;
  u32 frame = 0;
  float runningTime = 0.0f;
};

const GmSceneStats Gm_GetSceneStats(GmContext* context);
void Gm_AddMesh(GmContext* context, const std::string& meshName, u16 maxInstances, Gamma::Mesh* mesh);
void Gm_AddProbe(GmContext* context, const std::string& probeName, const Gamma::Vec3f& position);
Gamma::Light& Gm_CreateLight(GmContext* context, Gamma::LightType type);
void Gm_UseSceneFile(GmContext* context, const std::string& filename);
Gamma::Object& Gm_CreateObjectFrom(GmContext* context, const std::string& meshName);
void Gm_Commit(GmContext* context, const Gamma::Object& object);
Gamma::ObjectPool& Gm_GetObjects(GmContext* context, const std::string& meshName);
void Gm_SaveObject(GmContext* context, const std::string& objectName, const Gamma::Object& object);
void Gm_SaveLight(GmContext* context, const std::string& lightName, Gamma::Light* light);
bool Gm_HasObject(GmContext* context, const std::string& objectName);
Gamma::Object* Gm_FindObject(GmContext* context, const std::string& objectName);
Gamma::Object& Gm_GetObject(GmContext* context, const std::string& objectName);
Gamma::Light& Gm_GetLight(GmContext* context, const std::string& lightName);
void Gm_RemoveObject(GmContext* context, const Gamma::Object& object);
void Gm_RemoveLight(GmContext* context, Gamma::Light* light);
void Gm_PointCameraAt(GmContext* context, const Gamma::Object& object, bool upsideDown = false);
void Gm_PointCameraAt(GmContext* context, const Gamma::Vec3f& position, bool upsideDown = false);
void Gm_HandleFreeCameraMode(GmContext* context, float dt);
void Gm_UseFrustumCulling(GmContext* context, const std::initializer_list<std::string>& meshNames);
void Gm_UseLodByDistance(GmContext* context, float distance, const std::initializer_list<std::string>& meshNames);