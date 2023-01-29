#include "effects_system.h"
#include "macros.h"
#include "game_constants.h"

using namespace Gamma;

internal void initializePlayerParticles(GmContext* context) {
  add_mesh("ground-particle", TOTAL_GROUND_PARTICLES, Mesh::Sphere(6));
  add_mesh("air-particle", TOTAL_AIR_PARTICLES, Mesh::Sphere(4));

  mesh("ground-particle")->emissivity = 0.5f;
  mesh("air-particle")->emissivity = 0.8f;

  for (u8 i = 0; i < TOTAL_GROUND_PARTICLES; i++) {
    auto& particle = create_object_from("ground-particle");
    
    particle.scale = Vec3f(0.f);

    commit(particle);
  }

  for (u8 i = 0; i < TOTAL_AIR_PARTICLES; i++) {
    auto& particle = create_object_from("air-particle");

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

    for (auto& particle : objects("air-particle")) {
      if (
        particle.scale.x == 0.f &&
        time_since(state.lastAirParticleSpawnTime) > AIR_PARTICLE_SPAWN_DELAY
      ) {
        if (state.isOnSolidGround) {
          particle.scale = Vec3f(0.f);
        } else {
          #define fractf(v) (v - float(int(v)))

          Vec3f randomPositionWithinUnitSphere = Vec3f(Gm_Randomf(-1.f, 1.f), Gm_Randomf(-1.f, 1.f), Gm_Randomf(-1.f, 1.f)).unit();

          float r = fractf(state.frameStartTime);
          r *= 2.f;
          if (r > 1.f) r = 2.f - r;

          float g = fractf(state.frameStartTime + 0.33f);
          g *= 2.f;
          if (g > 1.f) g = 2.f - g;

          float b = fractf(state.frameStartTime + 0.66f);
          b *= 2.f;
          if (b > 1.f) b = 2.f - b;

          particle.position = state.previousPlayerPosition + randomPositionWithinUnitSphere * PLAYER_RADIUS;
          particle.scale = Vec3f(2.f);
          particle.color = Vec3f(r, g, b);
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

    for (auto& particle : objects("air-particle")) {
      particle.scale -= Vec3f(2.f * dt);

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

  float daytimeFactor = sqrt(sinf(state.dayNightCycleTime));
  auto& light = light("day-night-light");

  light.direction.y = 0.05f + -1.f * (0.5f + 0.5f * sinf(state.dayNightCycleTime * 2.f - Gm_HALF_PI));
  light.direction.z = cosf(state.dayNightCycleTime);
  light.power = daytimeFactor;
  light.color.y = daytimeFactor;
  light.color.z = daytimeFactor;

  context->scene.sunDirection = light.direction.invert().unit();
  context->scene.sunColor = light.color * Vec3f(1.f, 0.95f, 0.4f);
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