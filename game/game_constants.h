#pragma once

#include "Gamma.h"

/**
 * Entity constants
 * ----------------
 */
constexpr static float PLAYER_RADIUS = 25.f;

constexpr static float NPC_RADIUS = 20.f;
constexpr static float NPC_HEIGHT = 70.f;
constexpr static float NPC_INTERACTION_TRIGGER_DISTANCE = 120.f;
constexpr static float NPC_INTERACTION_CAMERA_RADIUS = 150.f;

constexpr static float SLINGSHOT_INTERACTION_TRIGGER_DISTANCE = 150.f;
constexpr static float SLINGSHOT_WIND_UP_CAMERA_RADIUS = 150.f;
constexpr static float SLINGSHOT_WIND_UP_DURATION_SECONDS = 0.5f;
const static Gamma::Vec3f DEFAULT_SLINGSHOT_COLOR = Gamma::Vec3f(0.2f);
const static Gamma::Vec3f HIGHLIGHT_SLINGSHOT_COLOR = Gamma::Vec3f(1.f);

constexpr static float LANTERN_HORIZONTAL_DRIFT = 15.f;
constexpr static float LANTERN_VERTICAL_DRIFT = 5.f;

constexpr static float BIRD_AT_REST_RESPAWN_DISTANCE = 2500.f;

/**
 * Movement/physics constants
 * --------------------------
 */
constexpr static float FORCE_GRAVITY = 750.f;
constexpr static float FORCE_WALL = 1000.f;
constexpr static float MAXIMUM_HORIZONTAL_GROUND_SPEED = 650.f;
constexpr static float MAXIMUM_HORIZONTAL_AIR_SPEED = 1500.f;
constexpr static float MAXIMUM_GLIDER_SPEED = 2250.f;
constexpr static float BASE_AIR_DASH_SPEED = 1000.f;
constexpr static float DASH_LEVEL_1_SPEED_FACTOR = 1.6f;
constexpr static float DASH_LEVEL_2_SPEED_FACTOR = 2.3f;
constexpr static float DEFAULT_JUMP_Y_VELOCITY = 500.f;
constexpr static float WALL_KICK_WINDOW_DURATION = 0.3f;
constexpr static float SUPER_JUMP_WINDOW_DURATION = 0.6f;
constexpr static float AIR_DASH_SPIN_DURATION = 0.7f;
constexpr static float BOOST_RING_DURATION = 0.8f;
constexpr static float GLIDER_BOOST_RING_DURATION = 1.5f;

/**
 * Camera constants
 * ----------------
 */
// const static Gamma::Vec3f CAMERA_TITLE_SCREEN_POSITION = Gamma::Vec3f(-3985.f, 2235.f, -1320.f);
const static Gamma::Vec3f CAMERA_TITLE_SCREEN_POSITION = Gamma::Vec3f(-4500.f, 5000.f, 1050.f);

constexpr static float CAMERA_FIRST_PERSON_RADIUS  = 5.f;
constexpr static float CAMERA_NORMAL_BASE_RADIUS = 300.f;
constexpr static float CAMERA_ZOOM_OUT_BASE_RADIUS = 600.f;
constexpr static float CAMERA_RADIUS_ALTITUDE_MULTIPLIER = 200.f;

constexpr static float CAMERA_BASE_FOV = 45.f;
constexpr static float CAMERA_FOV_VELOCITY_MULTIPLIER = 20.f;
constexpr static float CAMERA_FOV_MINIMUM_SPEED = 800.f;
constexpr static float CAMERA_FOV_SPEED_INTERVAL = 500.f;

/**
 * Effects constants
 * -----------------
 */
constexpr static float OCEAN_PLANE_Y_POSITION = -2000.f;  // @todo change to 0.f!!!
constexpr static float INITIAL_DAY_NIGHT_CYCLE_TIME = 2.f;

constexpr static u16 TOTAL_GROUND_PARTICLES = 42;
constexpr static u16 TOTAL_HARD_LANDING_PARTICLES = 12;
constexpr static u16 TOTAL_DASH_PARTICLES = 100;
constexpr static float GROUND_PARTICLE_SPAWN_DELAY = 1.f / float(TOTAL_GROUND_PARTICLES);
constexpr static float DASH_PARTICLE_SPAWN_DELAY = 1.f / float(TOTAL_DASH_PARTICLES);

constexpr static float GROUND_PARTICLE_SIZE = 3.5f;
constexpr static float DASH_PARTICLE_SIZE = 3.5f;

constexpr static u16 TOTAL_TORII_GATE_ZONE_PARTICLES = 500;
constexpr static float TORII_GATE_ZONE_PARTICLE_SIZE = 10.f;