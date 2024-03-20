#include "effects_system.h"
#include "easing.h"
#include "macros.h"
#include "game_constants.h"

using namespace Gamma;

internal void initializePlayerParticles(GmContext* context) {
  // Ground particles
  {
    add_mesh("ground-particle", TOTAL_GROUND_PARTICLES, Mesh::Sphere(6));
    mesh("ground-particle")->emissivity = 0.5f;

    for (u8 i = 0; i < TOTAL_GROUND_PARTICLES; i++) {
      auto& particle = create_object_from("ground-particle");
      
      particle.scale = Vec3f(0.f);

      commit(particle);
    }
  }

  // Dash particles
  {
    add_mesh("dash-particle", TOTAL_DASH_PARTICLES, Mesh::Particles());

    for (u8 i = 0; i < TOTAL_DASH_PARTICLES; i++) {
      auto& particle = create_object_from("dash-particle");

      particle.scale = Vec3f(0.f);

      commit(particle);
    }
  }
}

internal void initializeToriiGateZoneParticles(GmContext* context) {
  add_mesh("torii-particle", TOTAL_TORII_GATE_ZONE_PARTICLES, Mesh::Sphere(4));
  mesh("torii-particle")->emissivity = 0.5f;
  mesh("torii-particle")->canCastShadows = false;

  for (u16 i = 0; i < TOTAL_TORII_GATE_ZONE_PARTICLES; i++) {
    auto& particle = create_object_from("torii-particle");

    particle.color = Vec3f(0.f);
    particle.scale = Vec3f(0.f);

    commit(particle);
  }
}

internal void handlePlayerParticles(GmContext* context, GameState& state, float dt) {
  // Find the next available particle to spawn behind the player
  {
    auto& player = get_player();

    // Ground particles
    constexpr static float TOTAL_RUNNING_PARTICLES = TOTAL_GROUND_PARTICLES - TOTAL_HARD_LANDING_PARTICLES;

    for (auto& particle : objects("ground-particle")) {
      if (particle._record.id >= TOTAL_RUNNING_PARTICLES) {
        continue;
      }

      if (
        particle.scale.x == 0.f &&
        time_since(state.lastGroundParticleSpawnTime) > GROUND_PARTICLE_SPAWN_DELAY
      ) {
        if (state.isOnSolidGround) {
          float scale = GROUND_PARTICLE_SIZE * state.velocity.magnitude() / MAXIMUM_HORIZONTAL_GROUND_SPEED;
          if (scale > GROUND_PARTICLE_SIZE) scale = GROUND_PARTICLE_SIZE;

          scale += Gm_Randomf(0.f, 0.2f);

          particle.position = player.position - Vec3f(0, PLAYER_RADIUS * 0.8f, 0);
          particle.scale = Vec3f(scale);
          particle.color = Vec3f(0.7f);
        } else {
          particle.scale = Vec3f(0.f);
        }

        state.lastGroundParticleSpawnTime = get_scene_time();

        commit(particle);

        break;
      }
    }

    float timeSinceLastHardLanding = time_since(state.lastHardLandingTime);

    if (state.lastHardLandingTime != 0.f && timeSinceLastHardLanding < 1.f) {      
      float alpha = easeOutQuint(timeSinceLastHardLanding);
      float radius = alpha * 50.f;
      float scale = sqrtf(1.f - timeSinceLastHardLanding) * 5.f;

      for (auto& particle : objects("ground-particle")) {
        if (particle._record.id < TOTAL_RUNNING_PARTICLES) {
          continue;
        }

        float p = Gm_TAU * float(particle._record.id - TOTAL_RUNNING_PARTICLES) / TOTAL_HARD_LANDING_PARTICLES;

        particle.position = state.lastHardLandingPosition + Vec3f(sinf(p), 0, cosf(p)) * radius;
        particle.scale = Vec3f(scale);
        particle.color = Vec3f(0.6f);

        commit(particle);
      }
    }

    // Dash particles
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
              r = fractf(get_scene_time());
              r *= 2.f;
              if (r > 1.f) r = 2.f - r;

              g = fractf(get_scene_time() + 0.33f);
              g *= 2.f;
              if (g > 1.f) g = 2.f - g;

              b = fractf(get_scene_time() + 0.66f);
              b *= 2.f;
              if (b > 1.f) b = 2.f - b;

              break;
            }
          }

          if (state.lastBoostRingLaunchTime != 0.f && time_since(state.lastBoostRingLaunchTime) < BOOST_RING_DURATION) {
            // Temporarily change the color of dash particles when launching through boost rings
            r = 1.f;
            g = 0.9f;
            b = 0.4f;
          }

          particle.position = state.previousPlayerPosition + randomPositionWithinUnitSphere * PLAYER_RADIUS;
          particle.scale = Vec3f(DASH_PARTICLE_SIZE);
          particle.color = Vec3f(r, g, b);
        } else {
          particle.scale = Vec3f(0.f);
        }

        state.lastAirParticleSpawnTime = get_scene_time();

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

      // Apply a visual-only scaling factor to the particle so that
      // particles spawn smaller, grow to full size, and then continue
      // shrinking until they vanish. We use the current absolute scale
      // of the particle to determine where it is in its current cycle,
      // proportionally adjust its scale, and then reset it.
      float currentScale = particle.scale.x;
      float alpha = 1.f - currentScale / DASH_PARTICLE_SIZE;
      float scalingFactor = alpha < 0.1f ? 0.2f + 0.8f * alpha / 0.1f : 1.f - alpha;

      particle.scale *= scalingFactor;

      commit(particle);

      // Reset the particle scale so that the next time we operate on it,
      // we're starting from its proper transitioned scale
      particle.scale = Vec3f(currentScale);
    }
  }
}

internal void handlePlayerLight(GmContext* context) {
  auto& light = get_light("player-light");

  light.position = get_player().position + Vec3f(0, PLAYER_RADIUS * 2.f, 0);
  light.color = Vec3f(1.f, 0.8f, 0.6f);
  light.radius = 300.f;
  light.power = 1.f;
}

internal void handleDayNightCycle(GmContext* context, GameState& state, float dt) {
  if (state.gameStartTime != 0.f) {
    // state.dayNightCycleTime += dt * 0.001f;
    state.dayNightCycleTime = Gm_Modf(state.dayNightCycleTime, Gm_TAU);
  }

  EffectsSystem::updateDayNightCycleLighting(context, state);
}

internal void handleToriiGateEffects(GmContext* context, GameState& state, float dt) {
  auto& player = get_player();

  // Zone transition effects
  {
    auto& fx = context->scene.fx;

    if (state.isInToriiGateZone) {
      fx.redshiftInProgress = time_since(state.toriiGateTransitionTime);
      fx.redshiftOutProgress = 0.f;
    } else if (state.toriiGateTransitionTime != 0.f) {
      fx.redshiftOutProgress = time_since(state.toriiGateTransitionTime);
    }

    mesh("air-dash-target")->emissivity = state.isInToriiGateZone ? 1.f : 0.5f;
  }

  // Zone effects
  {
    #define MIN(n, m) n > m ? m : n
    #define MAX(n, m) n < m ? m : n

    float timeSinceToriiGateTransition = time_since(state.toriiGateTransitionTime);
    float particleScalingFactor = state.isInToriiGateZone ? MIN(timeSinceToriiGateTransition, 1.0f) : MAX(1.f - timeSinceToriiGateTransition, 0.f);

    mesh("torii-particle")->disabled = !(state.isInToriiGateZone || timeSinceToriiGateTransition < 1.f);

    for (auto& particle : objects("torii-particle")) {
      if (particle.scale.x < 0.5f) {
        float azimuth = Gm_Randomf(0.f, Gm_TAU);
        float altitude = Gm_Randomf(-Gm_HALF_PI, Gm_HALF_PI);
        float radius = Gm_Randomf(0.1f, 1.f);

        radius *= radius;
        radius = 1.f - radius;

        Vec3f spawn = Vec3f(
          cosf(altitude) * cosf(azimuth) * radius,
          sinf(altitude) * radius,
          cosf(altitude) * sinf(azimuth) * radius
        );

        spawn *= 3000.f;

        particle.position = player.position + spawn;
        particle.scale = Vec3f(Gm_Randomf(1.f, TORII_GATE_ZONE_PARTICLE_SIZE));
        particle.rotation = Quaternion::fromAxisAngle(Vec3f(1.f, 0, 0), Gm_Randomf(0.f, Gm_PI));
      } else {
        float lifetime = particle.scale.x / TORII_GATE_ZONE_PARTICLE_SIZE;
        Vec3f targetScale = Vec3f(TORII_GATE_ZONE_PARTICLE_SIZE * lifetime + 5.f * dt);
        float lifetimeScale = 2.f * (lifetime < 0.5f ? lifetime : 1.f - lifetime);

        particle.scale = targetScale * particleScalingFactor * lifetimeScale;
        particle.color.r = u8(255.f * lifetimeScale);

        commit(particle);

        if (lifetime < 1.f) {
          // Store the base scale without the scaling factor applied after the commit
          // so we can read it on the next cycle to determine the particle's lifetime
          particle.scale = targetScale;
        } else {
          // Reset the particle
          particle.scale = Vec3f(0.f);
        }
      }
    }
  }
}

internal void handleDashEffects(GmContext* context, GameState& state, float dt) {
  auto& scene = context->scene;

  if (state.dashLevel == 2) {
    auto timeSinceDashLevel2 = time_since(state.lastDashLevel2Time);

    scene.fx.dashRainbowIntensity = timeSinceDashLevel2 / (timeSinceDashLevel2 + 1.f);
  } else {
    scene.fx.dashRainbowIntensity = 0.f;
  }
}

void EffectsSystem::initializeGameEffects(GmContext* context, GameState& state) {
  context->scene.clouds = "./game/assets/clouds.png";

  initializePlayerParticles(context);
  initializeToriiGateZoneParticles(context);
}

void EffectsSystem::handleGameEffects(GmContext* context, GameState& state, float dt) {
  START_TIMING("handleGameEffects");

  handlePlayerParticles(context, state, dt);
  // handlePlayerLight(context);
  handleDayNightCycle(context, state, dt);
  handleToriiGateEffects(context, state, dt);
  handleDashEffects(context, state, dt);

  LOG_TIME();
}

void EffectsSystem::updateDayNightCycleLighting(GmContext* context, GameState& state) {
  auto& scene = context->scene;
  auto& camera = get_camera();
  auto& sceneLight = get_light("scene-light");

  // @todo can we express day/night colors as a continuum, rather than using separate logic for day + night?
  if (state.dayNightCycleTime < Gm_PI) {
    // Daytime
    float sineDayNightCycleTime = sinf(state.dayNightCycleTime);
    float daytimeFactor = sqrtf(sineDayNightCycleTime);

    sceneLight.direction.x = -0.2f;
    sceneLight.direction.y = -1.f * (0.5f + 0.5f * sinf(state.dayNightCycleTime * 2.f - Gm_HALF_PI));
    sceneLight.direction.z = cosf(state.dayNightCycleTime);
    sceneLight.direction = sceneLight.direction.unit();

    sceneLight.color.x = 1.f;
    sceneLight.color.y = sineDayNightCycleTime;
    sceneLight.color.z = sineDayNightCycleTime;
    sceneLight.power = daytimeFactor;

    scene.sky.atmosphereColor = Vec3f::lerp(Vec3f(1.f, 0.1f, 0.5f), Vec3f(1.f), daytimeFactor);
    scene.sky.sunDirection = sceneLight.direction.invert();
    scene.sky.sunColor = sceneLight.color * Vec3f(1.f, 0.95f, 0.4f);
  } else {
    // Nighttime
    float nightProgress = state.dayNightCycleTime - Gm_PI;
    float sineNightProgress = sinf(nightProgress);
    float lightFactor = sqrtf(sineNightProgress);

    sceneLight.direction.x = 0.2f;
    sceneLight.direction.y = -1.f * (0.5f + 0.5f * sinf(state.dayNightCycleTime * 2.f - Gm_HALF_PI));
    sceneLight.direction.z = cosf(nightProgress);
    sceneLight.direction = sceneLight.direction.unit();

    sceneLight.color.x = 1.f - sineNightProgress;
    sceneLight.color.y = sineNightProgress * 0.2f;
    sceneLight.color.z = sineNightProgress * 0.5f;
    sceneLight.power = lightFactor;

    scene.sky.atmosphereColor = Vec3f::lerp(Vec3f(1.f, 0.1f, 0.5f), Vec3f(0.0f, 0.1f, 0.5f), lightFactor);
    scene.sky.sunDirection = sceneLight.direction;
    scene.sky.sunColor = sceneLight.color * Vec3f(1.f, 0.95f, 0.4f);
  }

  scene.sky.altitude = camera.position.y + 2000.f;

  // Change the emissivity of specific meshes by time of day
  {
    float lightSourceEmissivity = sqrtf(Gm_Clampf(0.5f - sinf(state.dayNightCycleTime)));
    float lanternEmissivity = sqrtf(Gm_Clampf(0.5f + Gm_Clampf(-1.f * sinf(state.dayNightCycleTime))));

    mesh("window-1")->emissivity = lightSourceEmissivity;
    mesh("windows-1")->emissivity = lightSourceEmissivity;
    mesh("window-2")->emissivity = lightSourceEmissivity;
    mesh("lamp")->emissivity = lightSourceEmissivity;
    mesh("wall-lamp-bulb")->emissivity = lightSourceEmissivity;
    mesh("umimura-tree-branches")->emissivity = lightSourceEmissivity;
    mesh("b1-windows")->emissivity = lightSourceEmissivity;
    mesh("b2-windows")->emissivity = lightSourceEmissivity;
    mesh("b4-windows")->emissivity = lightSourceEmissivity;
    mesh("mini-building")->emissivity = 0.6f * lightSourceEmissivity;

    mesh("barred-window")->emissivity = 0.3f + 0.7f * lightSourceEmissivity;
    mesh("vertical-sign-1")->emissivity = 0.7f * lightSourceEmissivity;
    mesh("lantern")->emissivity = 0.5f + 0.5f * lightSourceEmissivity;
    mesh("ramen-sign")->emissivity = 0.3f * lightSourceEmissivity;
    mesh("p_ramen-sign")->emissivity = 0.3f + 0.3f * lightSourceEmissivity;
    mesh("p_mini-house-window")->emissivity = lightSourceEmissivity * 0.5f;
    mesh("solar-turbine")->emissivity = lightSourceEmissivity * 0.2f;

    mesh("floating-lantern")->emissivity = lanternEmissivity;
    mesh("streetlamp-light")->emissivity = lanternEmissivity;
    mesh("paper-lantern")->emissivity = 0.2f + 0.6f * lanternEmissivity;
    mesh("ramen-lamp")->emissivity = 0.4f + 0.5f * lanternEmissivity;
    mesh("orange-lantern")->emissivity = 0.4f + 0.3f * lanternEmissivity;
    mesh("bathhouse-balloon")->emissivity = 0.2f + 0.8f * lanternEmissivity;

    mesh("nitamago")->emissivity = 0.3f + 0.4f * lightSourceEmissivity;
    mesh("onigiri")->emissivity = 0.3f + 0.4f * lightSourceEmissivity;
    mesh("narutomaki")->emissivity = 0.3f + 0.4f * lightSourceEmissivity;
    mesh("chashu")->emissivity = 0.3f + 0.4f * lightSourceEmissivity;
    mesh("pepper")->emissivity = 0.3f + 0.4f * lightSourceEmissivity;
    mesh("coin")->emissivity = 0.3f + 0.4f * lightSourceEmissivity;
    mesh("player")->emissivity = 0.5f + 0.5f * lightSourceEmissivity;
  }

  // Adjust (serializable) point + spot light power by time of day
  {
    float lightPowerFactor = powf(Gm_Clampf(0.5f - sinf(state.dayNightCycleTime)), 1.f / 3.f);

    for (auto* light : context->scene.lights) {
      if (
        light->serializable &&
        !light->isStatic &&
        light->type != LightType::DIRECTIONAL &&
        light->type != LightType::DIRECTIONAL_SHADOWCASTER
      ) {
        light->power = light->basePower * lightPowerFactor;
      }
    }
  }
}