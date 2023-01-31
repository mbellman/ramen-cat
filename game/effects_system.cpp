#include "effects_system.h"
#include "macros.h"
#include "game_constants.h"

using namespace Gamma;

internal void initializePlayerParticles(GmContext* context) {
  add_mesh("ground-particle", TOTAL_GROUND_PARTICLES, Mesh::Sphere(6));
  mesh("ground-particle")->emissivity = 0.5f;

  add_mesh("dash-particle", TOTAL_DASH_PARTICLES, Mesh::Particles());

  for (u8 i = 0; i < TOTAL_GROUND_PARTICLES; i++) {
    auto& particle = create_object_from("ground-particle");
    
    particle.scale = Vec3f(0.f);

    commit(particle);
  }

  for (u8 i = 0; i < TOTAL_DASH_PARTICLES; i++) {
    auto& particle = create_object_from("dash-particle");

    particle.scale = Vec3f(0.f);

    commit(particle);
  }
}

internal void handlePlayerParticles(GmContext* context, GameState& state, float dt) {
  // Find the next available particle to spawn behind the player
  {
    auto& player = get_player();

    for (auto& particle : objects("ground-particle")) {
      if (
        particle.scale.x == 0.f &&
        time_since(state.lastGroundParticleSpawnTime) > GROUND_PARTICLE_SPAWN_DELAY
      ) {
        if (state.isOnSolidGround) {
          particle.position = player.position - Vec3f(0, player.scale.y * 0.5f, 0);
          particle.scale = Vec3f(3.f);
          particle.color = Vec3f(0.5f);
        } else {
          particle.scale = Vec3f(0.f);
        }

        state.lastGroundParticleSpawnTime = state.frameStartTime;

        commit(particle);

        break;
      }
    }

    for (auto& particle : objects("dash-particle")) {
      if (
        particle.scale.x == 0.f &&
        time_since(state.lastAirParticleSpawnTime) > DASH_PARTICLE_SPAWN_DELAY
      ) {
        if (state.dashLevel > 0) {
          #define fractf(v) (v - float(int(v)))

          Vec3f randomPositionWithinUnitSphere = Vec3f(Gm_Randomf(-1.f, 1.f), Gm_Randomf(-1.f, 1.f), Gm_Randomf(-1.f, 1.f)).unit();
          float r, g, b;

          switch (state.dashLevel) {
            // Dash level 1
            default:
            case 1: {
              r = 1.f;
              g = 0.5f;
              b = 0.8f;

              break;
            }
            // Dash level 2
            case 2: {
              r = fractf(state.frameStartTime);
              r *= 2.f;
              if (r > 1.f) r = 2.f - r;

              g = fractf(state.frameStartTime + 0.33f);
              g *= 2.f;
              if (g > 1.f) g = 2.f - g;

              b = fractf(state.frameStartTime + 0.66f);
              b *= 2.f;
              if (b > 1.f) b = 2.f - b;

              break;
            }
          }

          particle.position = state.previousPlayerPosition + randomPositionWithinUnitSphere * PLAYER_RADIUS;
          particle.scale = Vec3f(DASH_PARTICLE_SIZE);
          particle.color = Vec3f(r, g, b);
        } else {
          particle.scale = Vec3f(0.f);
        }

        state.lastAirParticleSpawnTime = state.frameStartTime;

        commit(particle);

        break;
      }
    }
  }

  // Gradually shrink all particles until they disappear.
  // When scaled down to 0, particles will become available
  // to respawn behind the player again.
  {
    for (auto& particle : objects("ground-particle")) {
      particle.scale -= Vec3f(5.f * dt);

      if (particle.scale.x < 0.f) {
        particle.scale = Vec3f(0.f);
      }

      commit(particle);
    }

    for (auto& particle : objects("dash-particle")) {
      particle.scale -= Vec3f(DASH_PARTICLE_SIZE * dt);

      if (particle.scale.x < 0.f) {
        particle.scale = Vec3f(0.f);
      }

      commit(particle);
    }
  }
}

internal void handleDayNightCycle(GmContext* context, GameState& state, float dt) {
  if (state.gameStartTime != 0.f) {
    state.dayNightCycleTime += dt * 0.001f;
    state.dayNightCycleTime = Gm_Modf(state.dayNightCycleTime, Gm_PI);
  }

  EffectsSystem::updateDayNightCycleLighting(context, state);
}

void EffectsSystem::initializeGameEffects(GmContext* context, GameState& state) {
  context->scene.clouds = "./game/assets/clouds.png";
  state.dayNightCycleTime = INITIAL_DAY_NIGHT_CYCLE_TIME;

  initializePlayerParticles(context);
}

void EffectsSystem::handleGameEffects(GmContext* context, GameState& state, float dt) {
  START_TIMING("handleGameEffects");

  handlePlayerParticles(context, state, dt);
  handleDayNightCycle(context, state, dt);

  LOG_TIME();
}

void EffectsSystem::updateDayNightCycleLighting(GmContext* context, GameState& state) {
  auto& scene = context->scene;
  auto& camera = get_camera();
  auto& sceneLight = light("scene-light");
  float daytimeFactor = sqrt(sinf(state.dayNightCycleTime));

  sceneLight.direction.y = 0.05f + -1.f * (0.5f + 0.5f * sinf(state.dayNightCycleTime * 2.f - Gm_HALF_PI));
  sceneLight.direction.z = cosf(state.dayNightCycleTime);
  sceneLight.power = daytimeFactor;
  sceneLight.color.y = daytimeFactor;
  sceneLight.color.z = daytimeFactor;

  scene.sky.sunDirection = sceneLight.direction.invert().unit();
  scene.sky.sunColor = sceneLight.color * Vec3f(1.f, 0.95f, 0.4f);
  scene.sky.atmosphereColor = Vec3f::lerp(Vec3f(1.f, 0.5f, 0.9f), Vec3f(1.f), daytimeFactor);
  scene.sky.altitude = camera.position.y - -2000.f;
}