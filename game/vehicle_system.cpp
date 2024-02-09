#include "vehicle_system.h"

using namespace Gamma;

// @todo @cleanup the code here is a first-pass and a little messy
void VehicleSystem::rebuildVehicleTracks(GmContext* context, GameState& state) {
  auto start = Gm_GetMicroseconds();

  state.vehicleTracks.clear();

  for (auto& spawn : objects("vehicle-spawn")) {
    VehicleTrack track;

    // @todo do this for all vehicle meshes (e.g. define a list of vehicle mesh names)
    for (auto& object : objects("small-train")) {
      if ((object.position - spawn.position).magnitude() < 2000.f) {
        Vehicle vehicle = {
          .object = object._record,
          .trackPointTarget = 0,
          .speed = 1000.f
        };

        object.position = spawn.position;

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

}