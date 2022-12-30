#include <string>
#include <vector>

#include "editor.h"
#include "world.h"
#include "collision.h"
#include "macros.h"

using namespace Gamma;

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
  ActionType currentActionType = ActionType::POSITION;
  // @todo limit size?
  std::vector<HistoryAction> history;
} editor;

inline std::string getActionTypeName(ActionType type) {
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

  editor.selectedObject = editor.observedObject;
  editor.isObjectSelected = true;
}

internal void cycleCurrentActionType() {
  // @todo define a list of current action types, find the
  // current one, select the next one, looping back to 0
  if (editor.currentActionType == ActionType::POSITION) {
    editor.currentActionType = ActionType::SCALE;
  } else if (editor.currentActionType == ActionType::SCALE) {
    editor.currentActionType = ActionType::ROTATE;
  } else if (editor.currentActionType == ActionType::ROTATE) {
    editor.currentActionType = ActionType::COLOR;
  } else if (editor.currentActionType == ActionType::COLOR) {
    editor.currentActionType = ActionType::CREATE;
  } else if (editor.currentActionType == ActionType::CREATE) {
    editor.currentActionType = ActionType::POSITION;
  } else {
    editor.currentActionType = ActionType::POSITION;
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
      areObjectPropertiesIdentical(*liveLastActionObject, lastAction.initialObject)
    ) {
      // No modifications were made to the last object in the history queue,
      // so replace the object in the last history action with this one
      auto& lastAction = editor.history.back();

      lastAction.type = type;
      lastAction.initialObject = object;

      return;
    }

    if (
      isSameObject(object, lastAction.initialObject) &&
      areObjectPropertiesIdentical(object, lastAction.initialObject)
    ) {
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

      World::rebuildCollisionPlanes(context, state);

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

      World::rebuildCollisionPlanes(context, state);

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

  auto& platform = create_object_from("platform");

  platform.position = spawnPosition;
  platform.scale = Vec3f(50.f, 20.f, 50.f);
  platform.rotation = Quaternion(1.f, 0, 0, 0);
  platform.color = Vec3f(0, 0, 1.f);
  
  commit(platform);

  HistoryAction action;

  action.type = ActionType::CREATE;
  action.initialObject = platform;

  editor.history.push_back(action);

  World::rebuildCollisionPlanes(context, state);
}

internal void deleteObject(GmContext* context, GameState& state, Object& object) {
  auto* originalObject = Gm_GetObjectByRecord(context, object._record);

  if (originalObject != nullptr) {
    createObjectHistoryAction(context, ActionType::DELETE, object);

    remove_object(*originalObject);

    World::rebuildCollisionPlanes(context, state);

    editor.isObjectSelected = false;
  }
}

internal void respawnPlayer(GmContext* context, GameState& state) {
  auto& player = get_player();
  auto& camera = get_camera();

  player.position = camera.position + camera.orientation.getDirection() * 300.f;

  state.velocity = Vec3f(0.f);

  commit(player);
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

    World::rebuildCollisionPlanes(context, state);
  }

  void initializeGameEditor(GmContext* context, GameState& state) {
    auto& commander = context->commander;

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
        Vec3f lineOfSightEnd = camera.position + cameraDirection * 2000.f;
        Vec3f inverseCameraDirection = cameraDirection.invert();
        float closestDistance = Gm_FLOAT_MAX;

        for (auto& plane : state.collisionPlanes) {
          float nDotC = Vec3f::dot(plane.normal, inverseCameraDirection);

          if (nDotC > 0.f) {
            auto collision = getLinePlaneCollision(camera.position, lineOfSightEnd, plane);
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

      if (input.didClickMouse()) {
        if (editor.isObservingObject) {
          selectObject(context, editor.observedObject);
          createObjectHistoryAction(context, editor.currentActionType, editor.observedObject);
        } else if (editor.currentActionType == ActionType::CREATE) {
          // @todo show an object placement preview
          createNewObject(context, state);
        }
      }

      if (input.isKeyHeld(Key::CONTROL) && input.didPressKey(Key::Z)) {
        undoLastHistoryAction(context, state);
      } else if (input.didPressKey(Key::SPACE)) {
        cycleCurrentActionType();

        if (
          editor.currentActionType == ActionType::CREATE &&
          editor.isObjectSelected
        ) {
          restoreObject(context, editor.selectedObject);

          editor.isObjectSelected = false;
        }
      } else if (input.didPressKey(Key::R)) {
        respawnPlayer(context, state);
      } else if (input.didPressKey(Key::BACKSPACE) && editor.isObjectSelected) {
        deleteObject(context, state, editor.selectedObject);
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
      add_debug_message("Action: " + getActionTypeName(editor.currentActionType));

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