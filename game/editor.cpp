#include <string>
#include <vector>

#include "editor.h"
#include "world.h"
#include "macros.h"

using namespace Gamma;

enum ActionType {
  CREATE,
  REMOVE,
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

  if (originalObject) {
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
  float alpha = 0.8f + std::sinf(getRunningTime() * 4.f) * 0.2f;

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

internal void createNewObject(GmContext* context) {
  auto& camera = getCamera();
  Vec3f spawnPosition = camera.position + camera.orientation.getDirection() * 150.f;

  auto& platform = createObjectFrom("platform");

  platform.position = spawnPosition;
  platform.scale = Vec3f(50.f, 20.f, 50.f);
  platform.color = Vec3f(0, 0, 1.f);
  
  commit(platform);

  HistoryAction action;

  action.type = ActionType::CREATE;
  action.initialObject = platform;

  editor.history.push_back(action);
}

internal Vec3f getCurrentActionDelta(GmContext* context, float mouseDx, float mouseDy, float dt) {
  auto& camera = getCamera();
  bool isVerticalMotion = Gm_Absf(mouseDy) > Gm_Absf(mouseDx);
  Vec3f direction;
  float multiplier = 1.f;

  switch (editor.currentActionType) {
    case ActionType::SCALE:
    case ActionType::POSITION: {
      multiplier = 20.f;

      if (isVerticalMotion) {
        direction = camera.orientation.getUpDirection().alignToAxis();
      } else {
        direction = camera.orientation.getRightDirection().alignToAxis();
      }

      break;
    }
    case ActionType::ROTATE: {
      multiplier = 0.2f;

      if (isVerticalMotion) {
        direction = camera.orientation.getRightDirection();
      } else {
        direction = camera.orientation.getUpDirection().invert();
      }

      break;
    }
  }

  return isVerticalMotion
    ? direction * -mouseDy * multiplier * dt 
    : direction * mouseDx * multiplier * dt;
}

internal void createObjectHistoryAction(GmContext* context, Object& object) {
  if (editor.history.size() > 0) {
    auto& lastTransaction = editor.history.back();
    auto* liveObject = Gm_GetObjectByRecord(context, lastTransaction.initialObject._record);

    if (
      liveObject != nullptr &&
      areObjectPropertiesIdentical(*liveObject, lastTransaction.initialObject)
    ) {
      // No modifications were made to the last object in the history queue,
      // so replace the object in the last history action with this one
      editor.history.back().initialObject = object;

      return;
    }

    if (
      isSameObject(object, lastTransaction.initialObject) &&
      areObjectPropertiesIdentical(object, lastTransaction.initialObject)
    ) {
      return;
    }
  }

  HistoryAction action;

  action.type = editor.currentActionType;
  action.initialObject = object; 

  editor.history.push_back(action);
}

internal void undoLastHistoryAction(GmContext* context) {
  if (editor.history.size() == 0) {
    return;
  }

  auto& action = editor.history.back();

  switch (action.type) {
    case ActionType::CREATE:
      removeObject(action.initialObject);
      break;
    case ActionType::REMOVE:
      break;
    default: {
      auto* liveObject = Gm_GetObjectByRecord(context, action.initialObject._record);

      if (liveObject != nullptr) {
        *liveObject = action.initialObject;

        commit(*liveObject);

        editor.selectedObject = *liveObject;
        editor.isObjectSelected = true;
      }
    }
  }

  editor.currentActionType = action.type;

  editor.history.pop_back();

  Console::log("[Editor] " + getActionTypeName(editor.currentActionType) + " action reverted");
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

    World::rebuildCollisionPlanes(context, state);
  }

  void handleGameEditor(GmContext* context, GameState& state, float dt) {
    auto& camera = getCamera();
    auto& input = getInput();

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
        float closestDistance = Gm_INFINITY;

        // @temporary
        for (auto& platform : objects("platform")) {
          Vec3f cameraToObject = platform.position - camera.position;
          float distance = cameraToObject.magnitude();
          Vec3f normalizedCameraToObject = cameraToObject / distance;
          float dot = Vec3f::dot(normalizedCameraToObject, cameraDirection);

          if (dot > 0.95f && distance < closestDistance) {
            observeObject(context, platform);

            closestDistance = distance;
          }
        }
      }
    }

    // Handle inputs
    {
      if (input.didClickMouse()) {
        if (editor.isObservingObject) {
          selectObject(context, editor.observedObject);
          createObjectHistoryAction(context, editor.observedObject);
        } else if (editor.currentActionType == ActionType::CREATE) {
          // @todo show an object placement preview
          createNewObject(context);
        }
      }

      if (input.isKeyHeld(Key::CONTROL) && input.didPressKey(Key::Z)) {
        undoLastHistoryAction(context);
      } else if (input.didPressKey(Key::SPACE)) {
        cycleCurrentActionType();

        if (
          editor.currentActionType == ActionType::CREATE &&
          editor.isObjectSelected
        ) {
          restoreObject(context, editor.selectedObject);

          editor.isObjectSelected = false;
        }
      }

      auto& mouseDelta = input.getMouseDelta();

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
            Quaternion r = Quaternion::fromAxisAngle(angle, axis.x, axis.y, axis.z);

            originalObject->rotation = r * originalObject->rotation;
          }
        }

        originalObject->color = editor.selectedObject.color;
        editor.selectedObject = *originalObject;

        commit(*originalObject);
      } else if (SDL_GetRelativeMouseMode()) {
        auto& camera = getCamera();

        Gm_HandleFreeCameraMode(context, 4.f, dt);

        camera.orientation.yaw += mouseDelta.x * dt * 0.08f;
        camera.orientation.pitch += mouseDelta.y * dt * 0.08f;
        camera.rotation = camera.orientation.toQuaternion();
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
      addDebugMessage("Action: " + getActionTypeName(editor.currentActionType));

      if (editor.isObjectSelected) {
        // @todo clean this up
        #define String(x) std::to_string(x)

        auto& p = editor.selectedObject.position;
        auto value = String(p.x) + ", " + String(p.y) + ", " + String(p.z);

        addDebugMessage("Active object:");
        addDebugMessage("Position: " + value);
      }
    }
  }
}