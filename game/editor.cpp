#include "editor.h"
#include "macros.h"

using namespace Gamma;

bool isObservingObject = false;
bool isObjectSelected = false;
Object observedObject;
Object selectedObject;

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
  if (!isObservingObject || !isSameObject(object, observedObject)) {
    if (isObservingObject) {
      restoreObject(context, observedObject);
    }

    observedObject = object;
    isObservingObject = true;
  }
}

internal void selectObservedObject(GmContext* context) {
  if (!isObservingObject) {
    return;
  }

  if (isObjectSelected) {
    restoreObject(context, selectedObject);
  }

  selectedObject = observedObject;
  isObjectSelected = true;
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
  }

  void handleGameEditor(GmContext* context, GameState& state, float dt) {
    Gm_HandleFreeCameraMode(context, 4.f, dt);

    // Find and focus the observed object
    {
      auto& camera = getCamera();
      Vec3f cameraDirection = camera.orientation.getDirection().unit();

      if (isObservingObject) {
        restoreObject(context, observedObject);

        isObservingObject = false;
      }

      // @temporary
      for (auto& platform : objects("platform")) {
        Vec3f normalizedCameraToObject = (platform.position - camera.position).unit();
        float dot = Vec3f::dot(normalizedCameraToObject, cameraDirection);

        if (dot > 0.95f) {
          observeObject(context, platform);

          break;
        }
      }
    }

    // Highlight the observed/selected objects
    {
      if (isObservingObject) {
        highlightObject(context, observedObject, Vec3f(1.f));
      }

      if (isObjectSelected) {
        highlightObject(context, selectedObject, Vec3f(1.f, 0, 0));

        // @todo clean this up
        #define String(x) std::to_string(x)

        auto& p = selectedObject.position;
        auto value = String(p.x) + ", " + String(p.y) + ", " + String(p.z);

        addDebugMessage("Active object:");
        addDebugMessage("Position: " + value);
      }
    }

    // Handle inputs
    {
      auto& input = getInput();

      if (input.didClickMouse()) {
        selectObservedObject(context);
      }
    }
  }
}