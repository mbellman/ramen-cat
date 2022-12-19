#include "editor.h"
#include "macros.h"

using namespace Gamma;

static bool hasActiveObject = false;
static Object activeObject;

internal bool isSameObject(Object& a, Object& b) {
  return (
    a._record.meshIndex == b._record.meshIndex &&
    a._record.id == b._record.id
  );
}

internal void restoreActiveObject(GmContext* context) {
  auto* originalObject = Gm_GetObjectByRecord(context, activeObject._record);

  if (originalObject) {
    *originalObject = activeObject;

    commit(*originalObject);
  }

  hasActiveObject = false;
}

internal void makeActiveObject(GmContext* context, Object& object) {
  if (!hasActiveObject || !isSameObject(object, activeObject)) {
    if (hasActiveObject) {
      restoreActiveObject(context);
    }

    activeObject = object;
    hasActiveObject = true;
  }
}

internal void highlightActiveObject(GmContext* context) {
  if (!hasActiveObject) {
    return;
  }

  Object* originalObject = Gm_GetObjectByRecord(context, activeObject._record);

  if (originalObject == nullptr) {
    return;
  }

  Vec3f originalColor = activeObject.color.toVec3f();
  float alpha = 0.8f + std::sinf(getRunningTime() * 4.f) * 0.2f;

  originalObject->color = Vec3f::lerp(originalColor, Vec3f(1.f), alpha);

  commit(*originalObject);
}

namespace Editor {
  void enableGameEditor(GmContext* context, GameState& state) {
    state.isEditorEnabled = true;
  }

  void disableGameEditor(GmContext* context, GameState& state) {
    if (hasActiveObject) {
      restoreActiveObject(context);
    }

    state.isEditorEnabled = false;
    hasActiveObject = false;
  }

  void handleGameEditor(GmContext* context, GameState& state, float dt) {
    Gm_HandleFreeCameraMode(context, 4.f, dt);

    // Find and focus the active object
    {
      auto& camera = getCamera();
      Vec3f cameraDirection = camera.orientation.getDirection().unit();

      if (hasActiveObject) {
        restoreActiveObject(context);
      }

      // @temporary
      for (auto& platform : objects("platform")) {
        Vec3f normalizedCameraToObject = (platform.position - camera.position).unit();
        float dot = Vec3f::dot(normalizedCameraToObject, cameraDirection);

        if (dot > 0.95f) {
          makeActiveObject(context, platform);

          break;
        }
      }
    }

    highlightActiveObject(context);
  }
}