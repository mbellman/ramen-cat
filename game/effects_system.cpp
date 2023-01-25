#include "effects_system.h"
#include "macros.h"
#include "game_constants.h"

using namespace Gamma;

internal void initializePlayerParticles(GmContext* context) {
  add_mesh("player-particle", TOTAL_PARTICLES, Mesh::Sphere(6));

  mesh("player-particle")->emissivity = 0.5f;

  // @todo createAll() ?
  for (u8 i = 0; i < TOTAL_PARTICLES; i++) {
    create_object_from("player-particle");
  }

  for (auto& particle : objects("player-particle")) {
    particle.scale = Vec3f(0.f);

    commit(particle);
  }
}

internal void handlePlayerParticles(GmContext* context, GameState& state, float dt) {
  auto& particles = objects("player-particle");

  // Find the next available particle to spawn behind the player
  {
    auto& player = get_player();
    auto runningTime = get_running_time();

    for (auto& particle : particles) {
      if (
        particle.scale.magnitude() < 0.001f &&
        runningTime - state.lastParticleSpawnTime > PARTICLE_SPAWN_DELAY
      ) {
        if (state.isOnSolidGround) {
          particle.position = player.position - Vec3f(0, player.scale.y * 0.5f, 0);
          particle.scale = Vec3f(3.f);
          particle.color = Vec3f(0.5f);
        } else {
          particle.position = player.position;
          particle.scale = Vec3f(5.f);
          particle.color = Vec3f(1.f, 1.f, 0);
        }

        state.lastParticleSpawnTime = runningTime;

        commit(particle);

        break;
      }
    }
  }

  // Gradually shrink all particles until they disappear.
  // When scaled down to 0, particles will become available
  // to respawn behind the player again.
  {
    for (auto& particle : particles) {
      particle.scale -= Vec3f(5.f * dt);

      if (particle.scale.x < 0.f) {
        particle.scale = Vec3f(0.f);
      }

      commit(particle);
    }
  }
}

internal void handleDayNightCycle(GmContext* context, GameState& state, float dt) {
  state.dayNightCycleTime += dt * 0.0025f;
  state.dayNightCycleTime = Gm_Modf(state.dayNightCycleTime, Gm_PI);

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