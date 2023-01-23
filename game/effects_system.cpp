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

internal void updatePlayerParticles(GmContext* context, GameState& state, float dt) {
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

void EffectsSystem::initializeGameEffects(GmContext* context, GameState& state) {
  initializePlayerParticles(context);
}

void EffectsSystem::handleGameEffects(GmContext* context, GameState& state, float dt) {
  START_TIMING("handleGameEffects");

  updatePlayerParticles(context, state, dt);

  LOG_TIME();
}