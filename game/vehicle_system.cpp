#include "vehicle_system.h"
#include "macros.h"

using namespace Gamma;

struct VehicleConfig {
  std::string name;
  float speed;
};

static std::vector<VehicleConfig> vehicleConfigs = {
  {
    .name = "small-train",
    .speed = 3000.f
  },
  {
    .name = "area-train",
    .speed = 4000.f
  },
  {
    .name = "cable-car",
    .speed = 500.f
  }
};

// @todo @cleanup the code here is a first-pass and a little messy
void VehicleSystem::rebuildVehicleTracks(GmContext* context, GameState& state) {
  auto start = Gm_GetMicroseconds();

  state.vehicleTracks.clear();

  for (auto& spawn : objects("vehicle-spawn")) {
    VehicleTrack track;

    for (auto& config : vehicleConfigs) {
      for (auto& object : objects(config.name)) {
        if ((object.position - spawn.position).magnitude() < 2000.f) {
          Vehicle vehicle = {
            .object = object._record,
            .trackPointTarget = 0,
            .speed = config.speed
          };

          object.position = spawn.position;

          commit(object);

          track.vehicles.push_back(vehicle);
        }
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

    for (auto& vehicle : track.vehicles) {
      if (track.points.size() > 1) {
        auto firstTrackSectionDirection = (track.points[1] - track.points[0]).unit();
        auto initialAngle = atan2f(firstTrackSectionDirection.x, firstTrackSectionDirection.z);

        vehicle.trackSectionStartAngle = initialAngle;
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
        auto objectDirection = object->rotation.getDirection();
        float currentAngle = atan2f(objectDirection.x, objectDirection.z);

        if (targetDistance < 20.f) {
          vehicle.trackPointTarget++;
          vehicle.trackSectionStartAngle = currentAngle;
        }

        if (vehicle.trackPointTarget > track.points.size() - 1) {
          // Reset vehicle to start
          auto direction = (track.points[1] - track.points[0]).unit();
          auto initialAngle = atan2f(direction.x, direction.z);

          vehicle.trackPointTarget = 0;
          vehicle.trackSectionStartAngle = initialAngle;

          object->position = track.points[0];
          object->rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), initialAngle);
        } else {
          auto previousTarget = vehicle.trackPointTarget > 0
            ? track.points[vehicle.trackPointTarget - 1]
            : track.points[0];

          auto nextTarget = track.points[vehicle.trackPointTarget];
          auto previousToNextTarget = nextTarget - previousTarget;
          auto objectToNextTarget = nextTarget - object->position;
          auto objectToPreviousTarget = previousTarget - object->position;
          auto objectDirection = object->rotation.getDirection();
          float targetAngle = atan2f(objectToNextTarget.x, objectToNextTarget.z);
          float turnProgress = 1.f - objectToNextTarget.magnitude() / previousToNextTarget.magnitude();
          float angle = Gm_LerpCircularf(vehicle.trackSectionStartAngle, targetAngle, turnProgress, Gm_PI);

          // @todo use proper curved interpolation between points (cubic spline will probably work)
          object->position += objectToNextTarget.unit() * vehicle.speed * dt;
          object->rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), angle);
        }

        commit(*object);
      }
    }
  }

  LOG_TIME();
}