#include "effects_system.h"
#include "macros.h"

using namespace Gamma;

const u16 TOTAL_PARTICLES = 30;

void EffectsSystem::initializeGameEffects(GmContext* context, GameState& state) {
  addMesh("player-particle", TOTAL_PARTICLES, Mesh::Sphere(6));

  mesh("player-particle")->emissivity = 0.5f;

  // @todo createAll() ?
  for (u8 i = 0; i < TOTAL_PARTICLES; i++) {
    createObjectFrom("player-particle");
  }

  for (auto& particle : objects("player-particle")) {
    particle.scale = Vec3f(0.f);

    commit(particle);
  }
}

void EffectsSystem::handleGameEffects(GmContext* context, GameState& state, float dt) {
  auto& particles = objects("player-particle");

  for (auto& particle : particles) {
    if (
      particle.scale.magnitude() < 0.001f &&
      (getRunningTime() - state.lastParticleSpawnTime) > (1.f / float(TOTAL_PARTICLES))
    ) {
      if (state.velocity.y == 0.f) {
        particle.position = getPlayer().position - Vec3f(0, getPlayer().scale.y * 0.5f, 0);
        particle.scale = Vec3f(3.f);
        particle.color = Vec3f(0.5f);
      } else {
        particle.position = getPlayer().position;
        particle.scale = Vec3f(5.f);
        particle.color = Vec3f(1.f, 1.f, 0);
      }

      state.lastParticleSpawnTime = getRunningTime();

      commit(particle);

      break;
    }
  }

  for (auto& particle : particles) {
    particle.scale -= Vec3f(5.f * dt);

    if (particle.scale.x < 0.f) {
      particle.scale = Vec3f(0.f);
    }

    commit(particle);
  }
}