#include "vehicle_system.h"
#include "macros.h"

using namespace Gamma;

// @todo @cleanup the code here is a first-pass and a little messy
void VehicleSystem::rebuildVehicleTracks(GmContext* context, GameState& state) {
  auto start = Gm_GetMicroseconds();

  state.vehicleTracks.clear();

  for (auto& spawn : objects("vehicle-spawn")) {
    VehicleTrack track;

    // @todo do this for all vehicle meshes (e.g. define a list of vehicle mesh names + speed)
    for (auto& object : objects("small-train")) {
      if ((object.position - spawn.position).magnitude() < 2000.f) {
        Vehicle vehicle = {
          .object = object._record,
          .trackPointTarget = 0,
          .speed = 3000.f
        };

        object.position = spawn.position;

        commit(object);

        track.vehicles.push_back(vehicle);
      }
    }

    // @todo do this for all vehicle meshes (e.g. define a list of vehicle mesh names + speed)
    for (auto& object : objects("cable-car")) {
      if ((object.position - spawn.position).magnitude() < 2000.f) {
        Vehicle vehicle = {
          .object = object._record,
          .trackPointTarget = 0,
          .speed = 500.f
        };

        object.position = spawn.position;

        commit(object);

        track.vehicles.push_back(vehicle);
      }
    }

    track.points.push_back(spawn.position);

    Vec3f start = spawn.position;
    Vec3f next = start;
    float closest = Gm_FLOAT_MAX;

    for (auto& t : objects("vehicle-track")) {
      float distance = (t.position - start).magnitude();

      if (distance < closest) {
        next = t.position;
        closest = distance;
      }
    }

    track.points.push_back(next);

    bool ended = false;

    while (true) {
      Vec3f direction = (track.points.back() - track.points[track.points.size() - 2]).unit();
      Vec3f checkStart = track.points.back() + (direction * 600.f);

      bool found = false;

      for (u8 i = 0; i < 50; i++) {
        Vec3f check = checkStart + direction * 200.f * float(i);

        for (auto& t : objects("vehicle-track")) {
          if ((t.position - check).magnitude() < 500.f) {
            track.points.push_back(t.position);

            found = true;

            break;
          }
        }

        if (found) {
          break;
        }
      }

      if (!found) {
        break;
      }
    }

    state.vehicleTracks.push_back(track);
  }

  #if GAMMA_DEVELOPER_MODE
    Console::log("Rebuilt vehicle tracks in", Gm_GetMicroseconds() - start, "us");
  #endif
}

void VehicleSystem::handleVehicles(GmContext* context, GameState& state, float dt) {
  START_TIMING("handleVehicles");

  for (auto& track : state.vehicleTracks) {
    for (auto& vehicle : track.vehicles) {
      auto object = get_object_by_record(vehicle.object);

      if (object != nullptr) {
        auto target = track.points[vehicle.trackPointTarget];
        auto objectToTarget = target - object->position;
        auto targetDistance = objectToTarget.magnitude();

        if (targetDistance < 20.f) {
          vehicle.trackPointTarget++;
        }

        if (vehicle.trackPointTarget > track.points.size() - 1) {
          auto direction = (track.points[1] - track.points[0]).unit();
          auto angle = atan2f(direction.x, direction.z);

          vehicle.trackPointTarget = 0;

          object->position = track.points[0];
          object->rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), angle);
        } else {
          auto finalTarget = track.points[vehicle.trackPointTarget];
          auto objectToFinalTarget = finalTarget - object->position;
          auto objectDirection = object->rotation.getDirection();
          float currentAngle = atan2f(objectDirection.x, objectDirection.z);
          float targetAngle = atan2f(objectToFinalTarget.x, objectToFinalTarget.z);
          float angle = Gm_LerpCircularf(currentAngle, targetAngle, dt, Gm_PI);

          object->position += objectToFinalTarget.unit() * vehicle.speed * dt;
          object->rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), angle);
        }

        commit(*object);
      }
    }
  }

  LOG_TIME();
}