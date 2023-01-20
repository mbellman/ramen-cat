#include <functional>
#include <string>
#include <vector>

#include "editor.h"
#include "world.h"
#include "collisions.h"
#include "macros.h"

using namespace Gamma;

enum EditorMode {
  COLLISION_PLANES,
  OBJECTS,
  LIGHTS
};

enum ActionType {
  CREATE,
  DELETE,
  POSITION,
  SCALE,
  ROTATE,
  COLOR
};

struct HistoryAction {
  ActionType type;
  Object initialObject;
};

static struct EditorState {
  Object observedObject;
  Object selectedObject;
  Light* selectedLight = nullptr;
  bool isObservingObject = false;
  bool isObjectSelected = false;
  float lastClickTime = 0.f;
  EditorMode mode = EditorMode::OBJECTS;
  ActionType currentActionType = ActionType::POSITION;
  u8 currentSelectedMeshIndex = 0;

  // @todo limit size?
  std::vector<HistoryAction> history;

  // @todo see if we end up having to use a more optimized lookup structure/world chunks/etc.
  std::vector<Plane> objectCollisionPlanes;

  // @todo see if we end up having to use a more optimized lookup structure/world chunks/etc.
  std::vector<Plane> lightCollisionPlanes;
} editor;

internal std::string getEditorModeName(EditorMode mode) {
  switch (mode) {
    case EditorMode::COLLISION_PLANES:
      return "COLLISION PLANES";
    case EditorMode::OBJECTS:
      return "OBJECTS";
    case EditorMode::LIGHTS:
      return "LIGHTS";
  }

  return "COLLISION PLANES";
}

internal std::string getActionTypeName(ActionType type) {
  switch (type) {
    case ActionType::CREATE:
      return "CREATE";
    case ActionType::DELETE:
      return "DELETE";
    case ActionType::POSITION:
      return "POSITION";
    case ActionType::SCALE:
      return "SCALE";
    case ActionType::ROTATE:
      return "ROTATE";
    case ActionType::COLOR:
      return "COLOR";
  }

  return "POSITION";
}

internal bool isSameObject(Object& a, Object& b) {
  return (
    a._record.meshIndex == b._record.meshIndex &&
    // @todo this does not consider object generations/recycling
    a._record.id == b._record.id
  );
}

internal bool areObjectPropertiesIdentical(Object& a, Object& b) {
  return (
    a.position == b.position &&
    a.rotation == b.rotation &&
    a.scale == b.scale &&
    a.color == b.color
  );
}

internal void restoreObject(GmContext* context, const Object& object) {
  auto* originalObject = get_object_by_record(object._record);

  if (originalObject != nullptr) {
    *originalObject = object;

    commit(*originalObject);
  }
}

internal void highlightObject(GmContext* context, const Object& object, const Vec3f& highlightColor) {
  Object* originalObject = get_object_by_record(object._record);

  if (originalObject == nullptr) {
    return;
  }

  Vec3f originalColor = object.color.toVec3f();
  float alpha = 0.8f + std::sinf(get_running_time() * 4.f) * 0.2f;

  originalObject->color = Vec3f::lerp(originalColor, highlightColor, alpha);

  commit(*originalObject);
}

internal void observeObject(GmContext* context, Object& object) {
  if (editor.isObservingObject) {
    restoreObject(context, editor.observedObject);
  }

  if (!editor.isObservingObject || !isSameObject(object, editor.observedObject)) {
    editor.observedObject = object;
    editor.isObservingObject = true;
  }
}

internal Light* findLightByPosition(GmContext* context, const Vec3f& position) {
  for (auto* light : context->scene.lights) {
    if (light->position == position) {
      return light;
    }
  }

  return nullptr;
}

internal void selectObject(GmContext* context, Object& object) {
  if (editor.isObjectSelected) {
    restoreObject(context, editor.selectedObject);
  }

  editor.selectedObject = object;
  editor.isObjectSelected = true;

  // Special handling
  {
    if (editor.mode == EditorMode::OBJECTS) {
      // In OBJECTS mode, change the current selected mesh index to that of the object
      u16 meshIndex = editor.selectedObject._record.meshIndex;
      auto& mesh = *context->scene.meshes[meshIndex];

      for (u8 i = 0; i < World::meshAssets.size(); i++) {
        if (mesh.name == World::meshAssets[i].name) {
          editor.currentSelectedMeshIndex = i;

          break;
        }
      }
    } else if (editor.mode == EditorMode::LIGHTS) {
      // In LIGHTS mode, change the selected light to that at the light sphere position
      editor.selectedLight = findLightByPosition(context, editor.selectedObject.position);
    }
  }
}

internal void syncLightWithObject(Light& light, const Object& object) {
  light.position = object.position;
  light.radius = 10.f * object.scale.x;
  light.color = object.color.toVec3f();
}

internal void updateCollisionPlanes(GmContext* context, GameState& state) {
  u64 start = Gm_GetMicroseconds();

  state.collisionPlanes.clear();
  editor.objectCollisionPlanes.clear();
  editor.lightCollisionPlanes.clear();

  for (auto& platform : objects("platform")) {
    Collisions::addObjectCollisionPlanes(platform, state.collisionPlanes);
  }

  for (auto& asset : World::meshAssets) {
    for (auto& object : mesh(asset.name)->objects) {
      Collisions::addObjectCollisionPlanes(object, editor.objectCollisionPlanes, asset.hitboxScale);
    }
  }

  for (auto& sphere : objects("light-sphere")) {
    Collisions::addObjectCollisionPlanes(sphere, editor.lightCollisionPlanes);
  }

  u32 total = state.collisionPlanes.size() + editor.objectCollisionPlanes.size() + editor.lightCollisionPlanes.size();

  Console::log("Rebuilt", total, "collision planes in", (Gm_GetMicroseconds() - start), "us");
}

internal void showDynamicMeshPlaceholders(GmContext* context) {
  // Show placeholders
  for (auto& asset : World::meshAssets) {
    if (asset.dynamic) {
      mesh(asset.name)->disabled = false;
    }
  }

  // Hide dynamic pieces
  for (auto& asset : World::dynamicMeshPieces) {
    mesh(asset.name)->disabled = true;
  }
}

internal void resetMovingObjects(GmContext* context, GameState& state) {
  for (auto& initialObject : state.initialMovingObjects) {
    auto* liveObject = get_object_by_record(initialObject._record);

    if (liveObject != nullptr) {
      *liveObject = initialObject;

      commit(*liveObject);
    }
  }
}

internal void updateInitialMovingObjects(GmContext* context, GameState& state) {
  state.initialMovingObjects.clear();

  for (auto& asset : World::meshAssets) {
    if (asset.moving) {
      for (auto& object : objects(asset.name)) {
        state.initialMovingObjects.push_back(object);
      }
    }
  }
}

internal void cycleEditorMode(GmContext* context, s8 delta) {
  const std::initializer_list<EditorMode> modeOrder = {
    EditorMode::COLLISION_PLANES,
    EditorMode::OBJECTS,
    EditorMode::LIGHTS
  };

  s8 totalModes = (s8)modeOrder.size();
  s8 cycleIndex = 0;

  for (u8 i = 0; i < totalModes; i++) {
    if (*(modeOrder.begin() + i) == editor.mode) {
      cycleIndex = i + delta;

      if (cycleIndex < 0) {
        cycleIndex = totalModes - 1;
      } else if (cycleIndex > totalModes - 1) {
        cycleIndex = 0;
      }

      break;
    }
  }

  editor.mode = *(modeOrder.begin() + cycleIndex);

  if (editor.mode != EditorMode::LIGHTS) {
    editor.selectedLight = nullptr;
  }
}

internal void cycleActionType(GmContext* context, s8 delta) {
  const std::initializer_list<ActionType> actionTypeOrder = {
    ActionType::POSITION,
    ActionType::SCALE,
    ActionType::ROTATE,
    ActionType::CREATE
  };

  s8 totalActionTypes = (s8)actionTypeOrder.size();
  s8 cycleIndex = 0;

  for (u8 i = 0; i < totalActionTypes; i++) {
    if (*(actionTypeOrder.begin() + i) == editor.currentActionType) {
      cycleIndex = i + delta;

      if (cycleIndex < 0) {
        cycleIndex = totalActionTypes - 1;
      } else if (cycleIndex > totalActionTypes - 1) {
        cycleIndex = 0;
      }

      break;
    }
  }

  editor.currentActionType = *(actionTypeOrder.begin() + cycleIndex);

  if (editor.currentActionType == ActionType::CREATE && editor.isObjectSelected) {
    restoreObject(context, editor.selectedObject);

    editor.isObjectSelected = false;
  }
}

// @todo description
internal Vec3f getMostSimilarAxis(
  const Vec3f& sourceAxis,
  const Vec3f& comparedForward, const Vec3f& comparedRight, const Vec3f& comparedUp,
  const Vec3f& resultForward, const Vec3f& resultRight, const Vec3f& resultUp
) {
  Vec3f similarAxis;

  float aDotF = Vec3f::dot(sourceAxis, comparedForward);
  float aDotR = Vec3f::dot(sourceAxis, comparedRight);
  float aDotU = Vec3f::dot(sourceAxis, comparedUp);

  float dF = Gm_Absf(aDotF);
  float dR = Gm_Absf(aDotR);
  float dU = Gm_Absf(aDotU);

  if (dF > dR && dF > dU) {
    similarAxis = resultForward;

    if (aDotF < 0.f) {
      similarAxis *= -1.f;
    }
  } else if (dR > dF && dR > dU) {
    similarAxis = resultRight;

    if (aDotR < 0.f) {
      similarAxis *= -1.f;
    }
  } else {
    similarAxis = resultUp;

    if (aDotU < 0.f) {
      similarAxis *= -1.f;
    }
  }

  return similarAxis;
}

// @todo description
internal Vec3f getMostSimilarObjectAxis(const Vec3f& sourceAxis, Object& object) {
  Vec3f objectForward = object.rotation.getDirection();
  Vec3f objectRight = object.rotation.getLeftDirection().invert();
  Vec3f objectUp = object.rotation.getUpDirection();  

  return getMostSimilarAxis(sourceAxis, objectForward, objectRight, objectUp, objectForward, objectRight, objectUp);
}

// @todo description
internal Vec3f getMostSimilarScalingAxis(const Vec3f& sourceAxis, Object& object) {
  Vec3f objectForward = object.rotation.getDirection();
  Vec3f objectRight = object.rotation.getLeftDirection().invert();
  Vec3f objectUp = object.rotation.getUpDirection();

  return getMostSimilarAxis(sourceAxis, objectForward, objectRight, objectUp, Vec3f(0, 0, 1.f), Vec3f(1.f, 0, 0), Vec3f(0, 1.f, 0));
}

internal Vec3f getCurrentActionDelta(GmContext* context, float mouseDx, float mouseDy, float dt) {
  auto& camera = get_camera();
  auto& input = get_input();
  auto& object = editor.history.back().initialObject;
  bool isVerticalMotion = Gm_Absf(mouseDy) > Gm_Absf(mouseDx);
  float multiplier = 1.f;
  Vec3f axis;

  switch (editor.currentActionType) {
    case ActionType::SCALE: {
      multiplier = 20.f;

      if (isVerticalMotion) {
        axis = getMostSimilarScalingAxis(camera.orientation.getUpDirection(), object);
      } else {
        axis = getMostSimilarScalingAxis(camera.orientation.getRightDirection(), object);
      }

      if (axis.x < 0 || axis.y < 0 || axis.z < 0) {
        // @todo description
        axis *= -1.f;
      }

      if (editor.mode == EditorMode::OBJECTS) {
        // @todo description
        axis *= World::meshAssets[editor.currentSelectedMeshIndex].scalingFactor;
      }

      break;
    }
    case ActionType::POSITION: {
      multiplier = 20.f;

      if (isVerticalMotion) {
        axis = camera.orientation.getUpDirection().alignToAxis();
      } else if (input.isKeyHeld(Key::Q)) {
        axis = camera.orientation.getRightDirection().alignToAxis();
      } else {
        axis = getMostSimilarObjectAxis(camera.orientation.getRightDirection(), object);
      }

      break;
    }
    case ActionType::ROTATE: {
      multiplier = 0.2f;

      if (isVerticalMotion) {
        axis = getMostSimilarObjectAxis(camera.orientation.getRightDirection(), object);
      } else {
        axis = getMostSimilarObjectAxis(camera.orientation.getUpDirection(), object).invert();
      }

      break;
    }
  }

  return isVerticalMotion
    ? axis * -mouseDy * multiplier * dt 
    : axis * mouseDx * multiplier * dt;
}

internal void createObjectHistoryAction(GmContext* context, ActionType type, Object& object) {
  if (editor.history.size() > 0) {
    auto& lastAction = editor.history.back();
    auto* liveLastActionObject = get_object_by_record(lastAction.initialObject._record);

    if (
      liveLastActionObject != nullptr &&
      areObjectPropertiesIdentical(*liveLastActionObject, lastAction.initialObject) &&
      lastAction.type != ActionType::CREATE &&
      lastAction.type != ActionType::DELETE
    ) {
      // No modifications were made to the last object in the history queue,
      // so replace the object in the last history action with this one
      lastAction.type = type;
      lastAction.initialObject = object;

      return;
    }

    if (
      isSameObject(object, lastAction.initialObject) &&
      areObjectPropertiesIdentical(object, lastAction.initialObject) &&
      lastAction.type != ActionType::CREATE &&
      lastAction.type != ActionType::DELETE
    ) {
      // Don't create a new history action when reselecting the same object,
      // unless the last action was a CREATE/DELETE, which we want to preserve
      // as separate records in the editor history
      return;
    }
  }

  HistoryAction action;

  action.type = type;
  action.initialObject = object;

  editor.history.push_back(action);
}

internal void undoLastHistoryAction(GmContext* context, GameState& state) {
  if (editor.history.size() == 0) {
    return;
  }

  auto& action = editor.history.back();

  switch (action.type) {
    case ActionType::CREATE: {
      auto& initialObject = action.initialObject;

      if (context->scene.meshes[initialObject._record.meshIndex]->name == "light-sphere") {
        // When undoing CREATE actions on light spheres,
        // remove the associated light
        auto* light = findLightByPosition(context, initialObject.position);

        if (light != nullptr) {
          remove_light(light);
        }
      }

      remove_object(initialObject);

      editor.isObjectSelected = false;

      updateCollisionPlanes(context, state);

      break;
    }
    case ActionType::DELETE: {
      auto& deletedObject = action.initialObject;
      auto& restoredObject = create_object_from(deletedObject._record.meshIndex);

      restoredObject.position = deletedObject.position;
      restoredObject.scale = deletedObject.scale;
      restoredObject.rotation = deletedObject.rotation;
      restoredObject.color = deletedObject.color;

      commit(restoredObject);

      editor.selectedObject = restoredObject;
      editor.isObjectSelected = true;

      updateCollisionPlanes(context, state);

      // When restoring deleted objects using undo, we need to update
      // older references to that object in the history as referring
      // to the restored object, which may use a different ID/generation
      // than the original. This preserves the integrity of objects across
      // CREATE -> DELETE -> undo -> undo sequences.
      for (auto& action : editor.history) {
        if (isSameObject(action.initialObject, deletedObject)) {
          action.initialObject._record = restoredObject._record;
        }
      }

      // When restoring light spheres, recreate the deleted light source
      if (context->scene.meshes[restoredObject._record.meshIndex]->name == "light-sphere") {
        auto& light = create_light(LightType::POINT);

        // @todo figure out a better way of determining power
        light.power = 5.f;

        syncLightWithObject(light, restoredObject);

        editor.selectedLight = &light;
      }

      break;
    }
    default: {
      auto& initialObject = action.initialObject;
      auto* liveLastActionObject = get_object_by_record(initialObject._record);

      if (liveLastActionObject != nullptr) {
        if (context->scene.meshes[initialObject._record.meshIndex]->name == "light-sphere") {
          // Update the associated light when undoing actions on light spheres
          auto* light = findLightByPosition(context, liveLastActionObject->position);

          if (light != nullptr) {
            syncLightWithObject(*light, initialObject);
          }
        }

        *liveLastActionObject = initialObject;

        commit(*liveLastActionObject);

        editor.selectedObject = *liveLastActionObject;
        editor.isObjectSelected = true;

        updateCollisionPlanes(context, state);
      }
    }
  }

  if (action.type == ActionType::DELETE) {
    // Don't allow the DELETE action type to be explicitly set;
    // default to POSITION when undoing deletion actions
    editor.currentActionType = ActionType::POSITION;
  } else {
    editor.currentActionType = action.type;
  }

  editor.history.pop_back();

  Console::log("[Editor] " + getActionTypeName(action.type) + " action reverted");
}

internal void createNewLight(GmContext* context, GameState& state) {
  auto& camera = get_camera();
  auto& light = create_light(LightType::POINT);
  auto& lightSphere = create_object_from("light-sphere");

  light.power = 5.f;

  // @temporary
  lightSphere.position = camera.position + camera.orientation.getDirection() * 150.f;
  lightSphere.scale = Vec3f(25.f);

  commit(lightSphere);

  syncLightWithObject(light, lightSphere);
  createObjectHistoryAction(context, ActionType::CREATE, lightSphere);
  updateCollisionPlanes(context, state);
}

internal void createNewObject(GmContext* context, GameState& state) {
  auto& camera = get_camera();
  Vec3f spawnPosition = camera.position + camera.orientation.getDirection() * 300.f;

  // Bring the spawn position closer than any objects in front of the camera
  for (auto& plane : editor.objectCollisionPlanes) {
    Vec3f start = camera.position;
    Vec3f end = spawnPosition;
    auto collision = Collisions::getLinePlaneCollision(start, end, plane);

    if (collision.hit) {
      spawnPosition = collision.point;
    }
  }

  // @todo use a common path for collision planes/objects
  if (editor.mode == EditorMode::COLLISION_PLANES) {
    auto& platform = create_object_from("platform");

    platform.position = spawnPosition;
    platform.scale = Vec3f(50.f, 20.f, 50.f);
    platform.rotation = Quaternion(1.f, 0, 0, 0);
    platform.color = Vec3f(0, 0, 1.f);

    commit(platform);

    createObjectHistoryAction(context, ActionType::CREATE, platform);
  } else if (editor.mode == EditorMode::OBJECTS) {
    auto& asset = World::meshAssets[editor.currentSelectedMeshIndex];
    auto& object = create_object_from(asset.name);

    object.position = spawnPosition;
    object.scale = asset.defaultScale;
    object.rotation = asset.defaultRotation;
    object.color = asset.defaultColor;

    commit(object);

    createObjectHistoryAction(context, ActionType::CREATE, object);
  }

  updateCollisionPlanes(context, state);
}

internal void cloneSelectedObject(GmContext* context, GameState& state) {
  auto& camera = get_camera();
  auto cameraDirection = camera.orientation.getDirection();

  auto& object = editor.mode == EditorMode::COLLISION_PLANES
    ? create_object_from("platform")
    : create_object_from(editor.selectedObject._record.meshIndex);

  Vec3f spawnPosition = camera.position + cameraDirection * 500.f;

  // Bring the spawn position closer than any objects in front of the camera
  for (auto& plane : editor.objectCollisionPlanes) {
    Vec3f start = camera.position;
    Vec3f end = spawnPosition;
    auto collision = Collisions::getLinePlaneCollision(start, end, plane);

    if (collision.hit) {
      spawnPosition = collision.point;
    }
  }

  object.position = spawnPosition;
  object.scale = editor.selectedObject.scale;
  object.rotation = editor.selectedObject.rotation;
  object.color = editor.selectedObject.color;

  commit(object);

  if (editor.mode == EditorMode::LIGHTS) {
    auto& light = create_light(LightType::POINT);

    light = *editor.selectedLight;

    syncLightWithObject(light, object);
  }

  selectObject(context, object);
  createObjectHistoryAction(context, ActionType::CREATE, object);
  updateCollisionPlanes(context, state);
}

internal void deleteObject(GmContext* context, GameState& state, Object& object) {
  auto* originalObject = get_object_by_record(object._record);

  if (originalObject != nullptr) {
    createObjectHistoryAction(context, ActionType::DELETE, object);

    remove_object(*originalObject);

    updateCollisionPlanes(context, state);

    editor.isObjectSelected = false;

    if (editor.selectedLight != nullptr) {
      remove_light(editor.selectedLight);

      editor.selectedLight = nullptr;
    }
  }
}

internal void respawnPlayer(GmContext* context, GameState& state) {
  auto& player = get_player();
  auto& camera = get_camera();

  player.position = camera.position + camera.orientation.getDirection() * 300.f;

  commit(player);

  state.velocity = Vec3f(0.f);
  state.previousPlayerPosition = player.position;

  // Prevent respawning in a lower area from erroneously
  // triggering fall position reset behavior
  state.lastTimeOnSolidGround = get_running_time();
}

internal void saveCollisionPlanesData(GmContext* context) {
  std::string data;

  for (auto& platform : objects("platform")) {
    data += Gm_ToString(platform.position) + ",";
    data += Gm_ToString(platform.scale) + ",";
    data += Gm_ToString(platform.rotation) + ",";
    data += Gm_ToString(platform.color) + "\n";
  }

  Gm_WriteFileContents("./game/data_collision_planes.txt", data);
}

internal void saveWorldObjectsData(GmContext* context) {
  std::string data;

  for (auto& asset : World::meshAssets) {
    data += "@" + asset.name + "\n";

    for (u16 i = 0; i < objects(asset.name).getHighestId(); i++) {
      auto* object = objects(asset.name).getById(i);

      if (object != nullptr) {
        data += Gm_ToString(object->position) + ",";
        data += Gm_ToString(object->scale) + ",";
        data += Gm_ToString(object->rotation) + ",";
        data += Gm_ToString(object->color) + "\n";        
      }
    }
  }

  Gm_WriteFileContents("./game/data_world_objects.txt", data);
}

internal void saveLightsData(GmContext* context) {
  std::string data;

  for (auto* light : context->scene.lights) {
    if (light->serializable) {
      data += Gm_ToString(light->position) + ",";
      data += std::to_string(light->radius) + ",";
      data += Gm_ToString(light->color) + ",";
      data += std::to_string(light->power) + "\n";
    }
  }

  Gm_WriteFileContents("./game/data_lights.txt", data);
}

namespace Editor {
  void enableGameEditor(GmContext* context, GameState& state) {
    state.isEditorEnabled = true;

    showDynamicMeshPlaceholders(context);
    resetMovingObjects(context, state);

    // Force certain meshes to be enabled/disabled
    {
      for (auto& asset : World::meshAssets) {
        if (!asset.dynamic) {
          mesh(asset.name)->objects.showAll();
        }
      }

      mesh("light-sphere")->disabled = false;
    }
  }

  void disableGameEditor(GmContext* context, GameState& state) {
    if (editor.isObservingObject) {
      restoreObject(context, editor.observedObject);
    }

    if (editor.isObjectSelected) {
      restoreObject(context, editor.selectedObject);
    }

    editor.isObservingObject = false;
    editor.isObjectSelected = false;
    editor.selectedLight = nullptr;

    state.isEditorEnabled = false;

    saveCollisionPlanesData(context);
    saveWorldObjectsData(context);
    saveLightsData(context);

    updateInitialMovingObjects(context, state);
    updateCollisionPlanes(context, state);

    World::rebuildDynamicMeshes(context);

    // Force certain meshes to be enabled/disabled
    {
      for (auto& asset : World::meshAssets) {
        mesh(asset.name)->disabled = asset.dynamic;
      }

      for (auto& asset : World::dynamicMeshPieces) {
        mesh(asset.name)->disabled = false;
      }

      mesh("light-sphere")->disabled = true;
    }
  }

  void initializeGameEditor(GmContext* context, GameState& state) {
    auto& input = get_input();
    auto& commander = context->commander;

    input.on<Key>("keyup", [context, & state](Key key) {
      // Toggle collision planes
      if (key == Key::C) {
        mesh("platform")->disabled = !mesh("platform")->disabled;
      }

      // Toggle objects
      if (key == Key::O) {
        for (auto& asset : World::meshAssets) {
          if (!asset.dynamic) {
            mesh(asset.name)->disabled = !mesh(asset.name)->disabled;
          }
        }

        for (auto& asset : World::dynamicMeshPieces) {
          mesh(asset.name)->disabled = !mesh(asset.name)->disabled || state.isEditorEnabled;
        }
      }
    });

    context->commander.on<std::string>("command", [&state, context](std::string command) {
      if (state.isEditorEnabled) {
        if (Gm_StringStartsWith(command, "color ")) {
          // @todo allow color edits to be undone
          using namespace std;

          auto colorString = Gm_SplitString(command, " ")[1];
          auto parts = Gm_SplitString(colorString, ",");

          // @todo put this in Gm_ParseVec3f, use that here
          while (parts.size() < 3) {
            parts.push_back(parts.back());
          }

          auto color = Vec3f(stof(parts[0]), stof(parts[1]), stof(parts[2]));

          if (editor.isObjectSelected) {
            auto* liveSelectedObject = get_object_by_record(editor.selectedObject._record);

            liveSelectedObject->color = color;
            editor.selectedObject = *liveSelectedObject;

            commit(*liveSelectedObject);

            if (editor.selectedLight != nullptr) {
              syncLightWithObject(*editor.selectedLight, editor.selectedObject);
            }
          }
        }
      }
    });

    // Visual aide meshes
    {
      add_mesh("light-sphere", 1000, Mesh::Sphere(10));

      mesh("light-sphere")->emissivity = 0.7f;
      mesh("light-sphere")->disabled = true;
    }

    // Generate light spheres for each light
    {
      for (auto* light : context->scene.lights) {
        auto& sphere = create_object_from("light-sphere");

        sphere.position = light->position;
        sphere.scale = Vec3f(light->radius / 10.f);
        sphere.color = light->color;

        commit(sphere);
      }
    }

    updateCollisionPlanes(context, state);
  }

  void handleGameEditor(GmContext* context, GameState& state, float dt) {
    START_TIMING("handleGameEditor");

    auto& camera = get_camera();
    auto& input = get_input();

    // Reset the observed/selected objects to
    // prevent corruption of original state
    {
      if (editor.isObservingObject) {
        restoreObject(context, editor.observedObject);

        editor.isObservingObject = false;
      }

      if (editor.isObjectSelected) {
        restoreObject(context, editor.selectedObject);
      }
    }

    // Find and focus the observed object
    {
      if (editor.currentActionType != ActionType::CREATE) {
        Vec3f cameraDirection = camera.orientation.getDirection().unit();
        Vec3f lineOfSightEnd = camera.position + cameraDirection * 4000.f;
        Vec3f inverseCameraDirection = cameraDirection.invert();
        float closestDistance = Gm_FLOAT_MAX;

        auto collisionPlanes =
          editor.mode == EditorMode::OBJECTS ? editor.objectCollisionPlanes :
          editor.mode == EditorMode::LIGHTS ? editor.lightCollisionPlanes :
          state.collisionPlanes;

        for (auto& plane : collisionPlanes) {
          float nDotC = Vec3f::dot(plane.normal, inverseCameraDirection);

          // Only consider planes facing the camera
          if (nDotC > 0.f) {
            auto collision = Collisions::getLinePlaneCollision(camera.position, lineOfSightEnd, plane);
            auto distance = (camera.position - collision.point).magnitude();

            if (collision.hit && distance < closestDistance) {
              // @todo none of the editor features should be enabled when not in developer mode anyway
              #if GAMMA_DEVELOPER_MODE
                auto* object = get_object_by_record(plane.sourceObjectRecord);
              #else
                auto* object = nullptr;
              #endif

              if (object != nullptr) {
                observeObject(context, *object);

                closestDistance = distance;
              }
            }
          }
        }
      }
    }

    // Handle inputs
    {
      auto& mouseDelta = input.getMouseDelta();

      if (input.didMoveMouseWheel()) {
        if (input.getMouseWheelDirection() == MouseWheelEvent::Direction::UP) {
          cycleActionType(context, -1);
        } else {
          cycleActionType(context, +1);
        }
      }

      if (input.didClickMouse()) {
        editor.lastClickTime = get_running_time();

        if (editor.isObservingObject || editor.isObjectSelected) {
          if (input.didRightClickMouse()) {
            // Object deselection
            editor.isObjectSelected = false;
          } else {
            // Check to ensure that we're observing an object before
            // we select the current 'observed object'. If we don't,
            // we're liable to select a stale version of the object,
            // which can produce a corrupted editor history.
            if (editor.isObservingObject) {
              selectObject(context, editor.observedObject);
            }

            createObjectHistoryAction(context, editor.currentActionType, editor.selectedObject);
          }
        } else if (editor.currentActionType == ActionType::CREATE) {
          // @todo show an object placement preview
          if (editor.mode == EditorMode::LIGHTS) {
            createNewLight(context, state);
          } else {
            createNewObject(context, state);
          }
        }
      }

      #define CTRL_Z input.isKeyHeld(Key::CONTROL) && input.didPressKey(Key::Z)
      #define CTRL_V input.isKeyHeld(Key::CONTROL) && input.didPressKey(Key::V)

      if (CTRL_Z) {
        undoLastHistoryAction(context, state);
      } else if (CTRL_V && editor.isObjectSelected) {
        cloneSelectedObject(context, state);
      } else if (input.didPressKey(Key::R)) {
        respawnPlayer(context, state);
      } else if (input.didPressKey(Key::BACKSPACE) && editor.isObjectSelected) {
        deleteObject(context, state, editor.selectedObject);
      } else if (input.didPressKey(Key::ARROW_RIGHT)) {
        cycleEditorMode(context, +1);
      } else if (input.didPressKey(Key::ARROW_LEFT)) {
        cycleEditorMode(context, -1);
      } else if (input.didPressKey(Key::ARROW_UP)) {
        // @todo cycleCurrentObject
        editor.currentSelectedMeshIndex++;

        if (editor.currentSelectedMeshIndex > World::meshAssets.size() - 1) {
          editor.currentSelectedMeshIndex = 0;
        }
      } else if (input.didPressKey(Key::ARROW_DOWN)) {
        // @todo cycleCurrentObject
        if (editor.currentSelectedMeshIndex == 0) {
          editor.currentSelectedMeshIndex = (u8)World::meshAssets.size() - 1;
        } else {
          editor.currentSelectedMeshIndex--;
        }
      }

      // Handle click-drag actions on objects
      if (
        input.isMouseHeld() &&
        !input.isKeyHeld(Key::SHIFT) &&
        editor.isObjectSelected &&
        time_since(editor.lastClickTime) > 0.1f
      ) {
        float dx = (float)mouseDelta.x;
        float dy = (float)mouseDelta.y;
        auto* originalObject = get_object_by_record(editor.selectedObject._record);
        Vec3f actionDelta = getCurrentActionDelta(context, dx, dy, dt);

        if (input.isKeyHeld(Key::CONTROL)) {
          actionDelta *= 0.1f;
        }

        if (editor.currentActionType == ActionType::POSITION) {
          originalObject->position += actionDelta;
        } else if (editor.currentActionType == ActionType::SCALE) {
          if (editor.mode == EditorMode::LIGHTS) {
            // Scale light spheres uniformly
            originalObject->scale += actionDelta.magnitude() * actionDelta.sign();
          } else {
            // Scale objects/collision planes along the action axis
            originalObject->scale += actionDelta;
          }
        } else if (editor.currentActionType == ActionType::ROTATE) {
          float angle = actionDelta.magnitude();

          if (angle > 0.f) {
            Vec3f axis = actionDelta.unit();

            originalObject->rotation *= Quaternion::fromAxisAngle(axis, angle);
          }
        }

        originalObject->color = editor.selectedObject.color;
        editor.selectedObject = *originalObject;

        commit(*originalObject);

        if (editor.mode == EditorMode::LIGHTS && editor.selectedLight != nullptr) {
          syncLightWithObject(*editor.selectedLight, editor.selectedObject);
        }
      } else if (Gm_IsWindowFocused()) {
        auto& camera = get_camera();

        camera.orientation.yaw += mouseDelta.x / 1500.f;
        camera.orientation.pitch += mouseDelta.y / 1500.f;
        camera.rotation = camera.orientation.toQuaternion();
      }

      if (Gm_IsWindowFocused()) {
        // Handle rotating around selected objects
        if (input.isKeyHeld(Key::SHIFT) && editor.isObjectSelected) {
          auto& selectedObject = editor.selectedObject;
          auto selectedObjectToCamera = camera.position - selectedObject.position;
          auto selectedObjectDistance = selectedObjectToCamera.magnitude();

          // @todo gradually_point_camera_at()
          Vec3f lookAtTarget = Vec3f::lerp(camera.position + camera.orientation.getDirection() * selectedObjectDistance, selectedObject.position, 25.f * dt);

          point_camera_at(lookAtTarget);

          // @todo create_object_third_person_camera()
          ThirdPersonCamera objectCamera;

          auto azimuth = atan2f(selectedObjectToCamera.z, selectedObjectToCamera.x);
          auto altitude = atan2f(selectedObjectToCamera.y, selectedObjectToCamera.xz().magnitude());

          objectCamera.altitude = altitude;
          objectCamera.azimuth = azimuth;
          objectCamera.radius = selectedObjectDistance;

          if (mouseDelta.x != 0 || mouseDelta.y != 0) {
            objectCamera.altitude += mouseDelta.y / 500.f;
            objectCamera.azimuth -= mouseDelta.x / 500.f;

            camera.position = selectedObject.position + objectCamera.calculatePosition();

            point_camera_at(selectedObject);
          }
        } else {
          // Handle WASD inputs
          float speed =
            input.isKeyHeld(Key::SPACE) ? 20000.f :
            input.isKeyHeld(Key::SHIFT) ? 800.f :
            4000.f;

          Gm_HandleFreeCameraMode(context, speed, dt);
        }
      }

      if (input.didReleaseMouse() && editor.isObjectSelected) {
        updateCollisionPlanes(context, state);
      }
    }

    // Highlight the observed/selected objects
    {
      if (editor.isObservingObject) {
        highlightObject(context, editor.observedObject, Vec3f(1.f, 0.f, 1.f));
      }

      if (editor.isObjectSelected) {
        auto highlightColor = input.isMouseHeld() ? Vec3f(0.7f, 0, 0) : Vec3f(1.f, 0, 0);

        highlightObject(context, editor.selectedObject, highlightColor);
      }
    }

    // Display editor info
    {
      add_debug_message("Mode: " + getEditorModeName(editor.mode));
      add_debug_message("Action: " + getActionTypeName(editor.currentActionType));

      if (editor.mode == EditorMode::OBJECTS) {
        auto& meshName = World::meshAssets[editor.currentSelectedMeshIndex].name;
        auto* mesh = mesh(meshName);
        u32 totalVertices = mesh->vertices.size();
        u16 totalInstances = mesh->objects.totalActive();

        add_debug_message("Mesh: " + meshName + " (" + std::to_string(totalVertices) + " vertices, " + std::to_string(totalInstances) + " instances)");
      }

      if (editor.isObjectSelected) {
        // @todo clean this up
        #define String(x) std::to_string(x)

        auto& p = editor.selectedObject.position;
        auto& s = editor.selectedObject.scale;
        auto& r = editor.selectedObject.rotation;

        // @todo Gm_ToString(const Vec3f&), Gm_ToString(const Quaternion&)
        auto position = String(p.x) + ", " + String(p.y) + ", " + String(p.z);
        auto scale = String(s.x) + ", " + String(s.y) + ", " + String(s.z);
        auto rotation = String(r.w) + ", " + String(r.x) + ", " + String(r.y) + ", " + String(r.z);

        add_debug_message("Active object:");
        add_debug_message("Position: " + position);
        add_debug_message("Scale: " + scale);
        add_debug_message("Rotation: " + rotation);
      }
    }

    LOG_TIME();
  }
}