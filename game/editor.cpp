#include <string>
#include <vector>

#include "editor.h"
#include "world.h"
#include "collisions.h"
#include "macros.h"

using namespace Gamma;

struct MeshAsset {
  std::string name;
  std::string objFilePath;
};

static std::vector<MeshAsset> meshAssets = {
  { "lamp", "./game/assets/lamp.obj" },
  { "da-vinci", "./game/assets/da-vinci.obj" }
};

enum ModeType {
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
  bool isObservingObject = false;
  bool isObjectSelected = false;
  ModeType mode = ModeType::COLLISION_PLANES;
  ActionType currentActionType = ActionType::POSITION;
  u8 currentSelectedMeshIndex = 0;
  // @todo limit size?
  std::vector<HistoryAction> history;

  // @todo see if we end up having to use a more optimized lookup structure/world chunks/etc.
  std::vector<Plane> objectCollisionPlanes;
} editor;

internal std::string getModeTypeName(ModeType mode) {
  switch (mode) {
    case ModeType::COLLISION_PLANES:
      return "COLLISION PLANES";
    case ModeType::OBJECTS:
      return "OBJECTS";
    case ModeType::LIGHTS:
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
  auto* originalObject = Gm_GetObjectByRecord(context, object._record);

  if (originalObject != nullptr) {
    *originalObject = object;

    commit(*originalObject);
  }
}

internal void highlightObject(GmContext* context, const Object& object, const Vec3f& highlightColor) {
  Object* originalObject = Gm_GetObjectByRecord(context, object._record);

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

internal void selectObject(GmContext* context, Object& object) {
  if (editor.isObjectSelected) {
    restoreObject(context, editor.selectedObject);
  }

  editor.selectedObject = object;
  editor.isObjectSelected = true;
}

internal void updateCollisionPlanes(GmContext* context, GameState& state) {
  Collisions::rebuildCollisionPlanes(objects("platform"), state.collisionPlanes);

  editor.objectCollisionPlanes.clear();

  for (auto& asset : meshAssets) {
    for (auto& object : mesh(asset.name)->objects) {
      // @todo
    }
  }
}

internal void cycleCurrentMode(GmContext* context, s8 delta) {
  const std::initializer_list<ModeType> modeOrder = {
    ModeType::COLLISION_PLANES,
    ModeType::OBJECTS,
    ModeType::LIGHTS
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
}

internal void cycleCurrentActionType(GmContext* context, s8 delta) {
  const std::initializer_list<ActionType> actionOrder = {
    ActionType::POSITION,
    ActionType::SCALE,
    ActionType::ROTATE,
    ActionType::COLOR,
    ActionType::CREATE
  };

  s8 totalActionTypes = (s8)actionOrder.size();
  s8 cycleIndex = 0;

  for (u8 i = 0; i < totalActionTypes; i++) {
    if (*(actionOrder.begin() + i) == editor.currentActionType) {
      cycleIndex = i + delta;

      if (cycleIndex < 0) {
        cycleIndex = totalActionTypes - 1;
      } else if (cycleIndex > totalActionTypes - 1) {
        cycleIndex = 0;
      }

      break;
    }
  }

  editor.currentActionType = *(actionOrder.begin() + cycleIndex);

  if (
    editor.currentActionType == ActionType::CREATE &&
    editor.isObjectSelected
  ) {
    restoreObject(context, editor.selectedObject);

    editor.isObjectSelected = false;
  }
}

internal Vec3f getObjectAlignedActionAxis(GmContext* context, Object& object) {
  auto& camera = get_camera();

  Vec3f cameraRight = camera.orientation.getRightDirection();
  Vec3f objectRight = object.rotation.getLeftDirection().invert();
  Vec3f objectForward = object.rotation.getDirection();
  Vec3f direction;

  float cDotR = Vec3f::dot(cameraRight, objectRight);
  float cDotF = Vec3f::dot(cameraRight, objectForward);

  if (Gm_Absf(cDotR) > Gm_Absf(cDotF)) {
    // The camera's right direction is similar to the object's,
    // so use the object space x axis.
    direction = objectRight;

    if (cDotR < 0) {
      // Preserve axis direction when looking toward -Z
      direction *= -1.f;
    }
  } else {
    // The camera's right direction is similar to the object's
    // forward direction, so use the object space z axis.
    direction = objectForward;

    if (cDotF < 0) {
      // Preserve axis direction when looking toward -X
      direction *= -1.f;
    }
  }

  return direction;
}

internal Vec3f getCurrentActionDelta(GmContext* context, float mouseDx, float mouseDy, float dt) {
  auto& camera = get_camera();
  bool isVerticalMotion = Gm_Absf(mouseDy) > Gm_Absf(mouseDx);
  float multiplier = 1.f;
  Vec3f axis;

  switch (editor.currentActionType) {
    case ActionType::SCALE: {
      multiplier = 20.f;

      if (isVerticalMotion) {
        axis = camera.orientation.getUpDirection().alignToAxis();
      } else {
        axis = camera.orientation.getRightDirection().alignToAxis();
      }

      if (axis.x < 0 || axis.y < 0 || axis.z < 0) {
        axis *= -1.f;
      }

      break;
    }
    case ActionType::POSITION: {
      multiplier = 20.f;

      if (isVerticalMotion) {
        axis = camera.orientation.getUpDirection().alignToAxis();
      } else {
        auto& object = editor.history.back().initialObject;

        axis = getObjectAlignedActionAxis(context, object);
      }

      break;
    }
    case ActionType::ROTATE: {
      multiplier = 0.2f;

      if (isVerticalMotion) {
        auto& object = editor.history.back().initialObject;

        axis = getObjectAlignedActionAxis(context, object);
      } else {
        axis = camera.orientation.getUpDirection().alignToAxis().invert();
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
    auto* liveLastActionObject = Gm_GetObjectByRecord(context, lastAction.initialObject._record);

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
    case ActionType::CREATE:
      remove_object(action.initialObject);

      editor.isObjectSelected = false;

      updateCollisionPlanes(context, state);

      break;
    case ActionType::DELETE: {
      auto& object = action.initialObject;
      // @todo retrieve mesh name from history action
      auto& platform = create_object_from("platform");

      platform.position = object.position;
      platform.scale = object.scale;
      platform.rotation = object.rotation;
      platform.color = object.color;

      commit(platform);

      updateCollisionPlanes(context, state);

      break;
    }
    default: {
      auto* liveLastActionObject = Gm_GetObjectByRecord(context, action.initialObject._record);

      if (liveLastActionObject != nullptr) {
        *liveLastActionObject = action.initialObject;

        commit(*liveLastActionObject);

        editor.selectedObject = *liveLastActionObject;
        editor.isObjectSelected = true;
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

internal void createNewObject(GmContext* context, GameState& state) {
  auto& camera = get_camera();
  Vec3f spawnPosition = camera.position + camera.orientation.getDirection() * 300.f;

  // @todo use a common path for collision planes/objects
  if (editor.mode == ModeType::COLLISION_PLANES) {
    auto& platform = create_object_from("platform");

    platform.position = spawnPosition;
    platform.scale = Vec3f(50.f, 20.f, 50.f);
    platform.rotation = Quaternion(1.f, 0, 0, 0);
    platform.color = Vec3f(0, 0, 1.f);

    commit(platform);

    createObjectHistoryAction(context, ActionType::CREATE, platform);
  } else if (editor.mode == ModeType::OBJECTS) {
    auto& meshName = meshAssets[editor.currentSelectedMeshIndex].name;
    auto& object = create_object_from(meshName);

    object.position = spawnPosition;
    object.scale = Vec3f(20.f);
    object.rotation = Quaternion(1.f, 0, 0, 0);
    object.color = Vec3f(1.f);

    commit(object);

    createObjectHistoryAction(context, ActionType::CREATE, object);
  }

  updateCollisionPlanes(context, state);
}

internal void cloneSelectedObject(GmContext* context) {
  auto& camera = get_camera();
  auto cameraDirection = camera.orientation.getDirection();

  // @todo use selected object mesh
  auto& object = create_object_from("platform");

  object.position = camera.position + cameraDirection * 500.f;
  object.scale = editor.selectedObject.scale;
  object.rotation = editor.selectedObject.rotation;
  object.color = editor.selectedObject.color;

  commit(object);

  selectObject(context, object);
  createObjectHistoryAction(context, ActionType::CREATE, object);
}

internal void deleteObject(GmContext* context, GameState& state, Object& object) {
  auto* originalObject = Gm_GetObjectByRecord(context, object._record);

  if (originalObject != nullptr) {
    createObjectHistoryAction(context, ActionType::DELETE, object);

    remove_object(*originalObject);

    updateCollisionPlanes(context, state);

    editor.isObjectSelected = false;
  }
}

internal void respawnPlayer(GmContext* context, GameState& state) {
  auto& player = get_player();
  auto& camera = get_camera();

  player.position = camera.position + camera.orientation.getDirection() * 300.f;

  commit(player);

  state.velocity = Vec3f(0.f);
  state.previousPlayerPosition = player.position;
}

internal void saveGameWorldData(GmContext* context, GameState& state) {
  std::string data;

  data += "@platform\n";

  for (auto& platform : objects("platform")) {
    data += Gm_ToString(platform.position) + ",";
    data += Gm_ToString(platform.scale) + ",";
    data += Gm_ToString(platform.rotation) + ",";
    data += Gm_ToString(platform.color) + "\n";
  }

  Gm_WriteFileContents("./game/data_world.txt", data);
}

namespace Editor {
  void enableGameEditor(GmContext* context, GameState& state) {
    state.isEditorEnabled = true;
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

    state.isEditorEnabled = false;

    saveGameWorldData(context, state);
    updateCollisionPlanes(context, state);
  }

  void initializeGameEditor(GmContext* context, GameState& state) {
    auto& input = get_input();
    auto& commander = context->commander;

    for (auto& asset : meshAssets) {
      add_mesh(asset.name, 100, Mesh::Model(asset.objFilePath.c_str()));
    }

    input.on<MouseWheelEvent>("mousewheel", [context, &state](const MouseWheelEvent& event) {
      if (event.direction == MouseWheelEvent::Direction::UP) {
        cycleCurrentActionType(context, -1);
      } else {
        cycleCurrentActionType(context, +1);
      }
    });

    context->commander.on<std::string>("command", [&state, context](std::string command) {
      if (state.isEditorEnabled) {
        if (Gm_StringStartsWith(command, "color ")) {
          // @todo allow color edits to be undone
          using namespace std;

          auto colorString = Gm_SplitString(command, " ")[1];
          auto parts = Gm_SplitString(colorString, ",");
          auto color = Vec3f(stof(parts[0]), stof(parts[1]), stof(parts[2]));

          if (editor.isObjectSelected) {
            auto* liveSelectedObject = Gm_GetObjectByRecord(context, editor.selectedObject._record);

            liveSelectedObject->color = color;
            editor.selectedObject = *liveSelectedObject;

            commit(*liveSelectedObject);
          }
        }
      }
    });
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

        auto collisionPlanes = editor.mode == ModeType::OBJECTS
          ? editor.objectCollisionPlanes
          : state.collisionPlanes;

        for (auto& plane : collisionPlanes) {
          float nDotC = Vec3f::dot(plane.normal, inverseCameraDirection);

          if (nDotC > 0.f) {
            auto collision = Collisions::getLinePlaneCollision(camera.position, lineOfSightEnd, plane);
            auto distance = (camera.position - collision.point).magnitude();

            if (collision.hit && distance < closestDistance) {
              // @todo none of the editor features should be enabled when not in developer mode anyway
              #if GAMMA_DEVELOPER_MODE
                auto* object = Gm_GetObjectByRecord(context, plane.sourceObjectRecord);
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

      if (input.didPressMouse()) {
        if (editor.isObservingObject || editor.isObjectSelected) {
          // Check to ensure that we're observing an object before
          // we select it. If we don't, we're liable to select a
          // stale version of editor.observedObject, which can produce
          // a corrupted editor history.
          if (editor.isObservingObject) {
            selectObject(context, editor.observedObject);
          }

          createObjectHistoryAction(context, editor.currentActionType, editor.selectedObject);
        } else if (editor.currentActionType == ActionType::CREATE) {
          // @todo show an object placement preview
          createNewObject(context, state);
        }
      }

      #define CTRL_Z input.isKeyHeld(Key::CONTROL) && input.didPressKey(Key::Z)
      #define CTRL_V input.isKeyHeld(Key::CONTROL) && input.didPressKey(Key::V)

      if (CTRL_Z) {
        undoLastHistoryAction(context, state);
      } else if (CTRL_V && editor.isObjectSelected) {
        cloneSelectedObject(context);
      } else if (input.didPressKey(Key::R)) {
        respawnPlayer(context, state);
      } else if (input.didPressKey(Key::BACKSPACE) && editor.isObjectSelected) {
        deleteObject(context, state, editor.selectedObject);
      } else if (input.didPressKey(Key::ARROW_RIGHT)) {
        cycleCurrentMode(context, +1);
      } else if (input.didPressKey(Key::ARROW_LEFT)) {
        cycleCurrentMode(context, -1);
      } else if (input.didPressKey(Key::ARROW_UP)) {
        // @todo cycleCurrentObject
        editor.currentSelectedMeshIndex++;

        if (editor.currentSelectedMeshIndex > meshAssets.size() - 1) {
          editor.currentSelectedMeshIndex = 0;
        }
      }

      if (input.isMouseHeld() && editor.isObjectSelected) {
        float dx = (float)mouseDelta.x;
        float dy = (float)mouseDelta.y;
        auto* originalObject = Gm_GetObjectByRecord(context, editor.selectedObject._record);
        auto actionDelta = getCurrentActionDelta(context, dx, dy, dt);

        if (editor.currentActionType == ActionType::POSITION) {
          originalObject->position += actionDelta;
        } else if (editor.currentActionType == ActionType::SCALE) {
          originalObject->scale += actionDelta;
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
      } else if (SDL_GetRelativeMouseMode()) {
        auto& camera = get_camera();

        camera.orientation.yaw += mouseDelta.x / 1500.f;
        camera.orientation.pitch += mouseDelta.y / 1500.f;
        camera.rotation = camera.orientation.toQuaternion();        
      }

      if (SDL_GetRelativeMouseMode()) {
        Gm_HandleFreeCameraMode(context, 4.f, dt);
      }

      if (input.didReleaseMouse() && editor.isObjectSelected) {
        updateCollisionPlanes(context, state);
      }
    }

    // Highlight the observed/selected objects
    {
      if (editor.isObservingObject) {
        highlightObject(context, editor.observedObject, Vec3f(1.f));
      }

      if (editor.isObjectSelected) {
        auto highlightColor = input.isMouseHeld() ? Vec3f(0.7f, 0, 0) : Vec3f(1.f, 0, 0);

        highlightObject(context, editor.selectedObject, highlightColor);
      }
    }

    // Display status messages
    {
      add_debug_message("Mode: " + getModeTypeName(editor.mode));
      add_debug_message("Action: " + getActionTypeName(editor.currentActionType));

      if (editor.mode == ModeType::OBJECTS) {
        add_debug_message("Mesh: " + meshAssets[editor.currentSelectedMeshIndex].name);
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