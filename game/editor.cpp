#include "editor.h"
#include "macros.h"

using namespace Gamma;

bool hasActiveObject = false;
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
}

internal void focusObject(GmContext* context, Object& object) {
  if (!isSameObject(object, activeObject)) {
    if (hasActiveObject) {
      restoreActiveObject(context);
    }

    activeObject = object;
    hasActiveObject = true;
  }

  object.color = Vec3f(1.f);

  commit(object);
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

    Vec3f cameraDirection = getCamera().orientation.getDirection();

    // @temporary
    u16 targetId = (u16)getRunningTime() % objects("platform").totalActive();

    // @temporary
    for (auto& platform : objects("platform")) {
      if (platform._record.id == targetId) {
        focusObject(context, platform);
      }
    }
  }
}