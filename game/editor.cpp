#include <vector>

#include "editor.h"
#include "world.h"
#include "macros.h"

using namespace Gamma;

struct HistoryAction {
  Object initialObject;
};

bool isObservingObject = false;
bool isObjectSelected = false;
Object observedObject;
Object selectedObject;
static std::vector<HistoryAction> history;

internal bool isSameObject(Object& a, Object& b) {
  return (
    a._record.meshIndex == b._record.meshIndex &&
    a._record.id == b._record.id
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
  if (isObservingObject) {
    restoreObject(context, observedObject);
  }

  if (!isObservingObject || !isSameObject(object, observedObject)) {
    observedObject = object;
    isObservingObject = true;
  }
}

internal void selectObject(GmContext* context, Object& object) {
  if (isObjectSelected) {
    restoreObject(context, selectedObject);
  }

  selectedObject = observedObject;
  isObjectSelected = true;
}

internal void createObjectHistoryAction(GmContext* context, Object& object) {
  if (history.size() > 0) {
    auto& lastTransaction = history.back();
    auto* lastObject = Gm_GetObjectByRecord(context, lastTransaction.initialObject._record);

    if (lastObject != nullptr) {
      // @todo consider other properties about the object
      if ((*lastObject).position == lastTransaction.initialObject.position) {
        // No modifications were made to the last object in the history queue,
        // so replace the object in the last history action with this one
        history.back().initialObject = object;

        return;
      }
    }

    if (
      isSameObject(object, lastTransaction.initialObject) &&
      // @todo consider other properties about the object
      object.position == lastTransaction.initialObject.position
    ) {
      return;
    }
  }

  HistoryAction action;

  action.initialObject = object;

  history.push_back(action);
}

internal void undoLastHistoryAction(GmContext* context) {
  if (history.size() == 0) {
    return;
  }

  auto& action = history.back();
  auto* liveObject = Gm_GetObjectByRecord(context, action.initialObject._record);

  if (liveObject != nullptr) {
    *liveObject = action.initialObject;

    commit(*liveObject);

    Console::log("[Editor] Action reverted");

    selectedObject = *liveObject;
    isObjectSelected = true;
  }

  history.pop_back();
}

namespace Editor {
  void enableGameEditor(GmContext* context, GameState& state) {
    state.isEditorEnabled = true;
  }

  void disableGameEditor(GmContext* context, GameState& state) {
    if (isObservingObject) {
      restoreObject(context, observedObject);
    }

    if (isObjectSelected) {
      restoreObject(context, selectedObject);
    }

    isObservingObject = false;
    isObjectSelected = false;

    state.isEditorEnabled = false;

    World::rebuildCollisionPlanes(context, state);
  }

  void handleGameEditor(GmContext* context, GameState& state, float dt) {
    auto& camera = getCamera();
    auto& input = getInput();

    // Find and focus the observed object
    {
      Vec3f cameraDirection = camera.orientation.getDirection().unit();

      // Reset the observed/selected objects to
      // prevent corruption of original state
      {
        if (isObservingObject) {
          restoreObject(context, observedObject);

          isObservingObject = false;
        }

        if (isObjectSelected) {
          restoreObject(context, selectedObject);
        }
      }

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

    // Handle inputs
    {
      if (input.didClickMouse() && isObservingObject) {
        selectObject(context, observedObject);
        createObjectHistoryAction(context, observedObject);
      }

      if (input.isKeyHeld(Key::CONTROL) && input.didPressKey(Key::Z)) {
        undoLastHistoryAction(context);
      }

      auto& mouseDelta = input.getMouseDelta();

      if (input.isMouseHeld() && isObjectSelected) {
        float dx = (float)mouseDelta.x;
        float dy = (float)mouseDelta.y;
        auto* originalObject = Gm_GetObjectByRecord(context, selectedObject._record);
        Vec3f move;

        if (Gm_Absf(dy) > Gm_Absf(dx)) {
          Vec3f moveDirection = camera.orientation.getUpDirection().alignToAxis();

          move = moveDirection * -dy * 20.f * dt;
        } else {
          Vec3f moveDirection = camera.orientation.getRightDirection().alignToAxis();

          move = moveDirection * dx * 20.f * dt;
        }

        originalObject->position += move;
        originalObject->color = selectedObject.color;
        selectedObject = *originalObject;

        commit(*originalObject);
      } else if (SDL_GetRelativeMouseMode()) {
        auto& camera = getCamera();

        Gm_HandleFreeCameraMode(context, 4.f, dt);

        camera.orientation.yaw += mouseDelta.x / 1000.f;
        camera.orientation.pitch += mouseDelta.y / 1000.f;
        camera.rotation = camera.orientation.toQuaternion();
      }
    }

    // Highlight the observed/selected objects
    {
      if (isObservingObject) {
        highlightObject(context, observedObject, Vec3f(1.f));
      }

      if (isObjectSelected) {
        auto highlightColor = input.isMouseHeld() ? Vec3f(0.7f, 0, 0) : Vec3f(1.f, 0, 0);

        highlightObject(context, selectedObject, highlightColor);

        // @todo clean this up
        #define String(x) std::to_string(x)

        auto& p = selectedObject.position;
        auto value = String(p.x) + ", " + String(p.y) + ", " + String(p.z);

        addDebugMessage("Active object:");
        addDebugMessage("Position: " + value);
      }
    }
  }
}