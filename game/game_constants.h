#pragma once

#include "Gamma.h"

/**
 * Entity constants
 * ----------------
 */
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

/**
 * Movement/physics constants
 * --------------------------
 */
constexpr static float FORCE_GRAVITY = 750.f;
constexpr static float FORCE_WALL = 1000.f;
constexpr static float MAXIMUM_HORIZONTAL_GROUND_SPEED = 650.f;
constexpr static float JUMP_Y_VELOCITY = 500.f;

constexpr static float WALL_KICK_DELAY_DURATION = 0.3f;

/**
 * Camera constants
 * ----------------
 */
const static Gamma::Vec3f CAMERA_TITLE_SCREEN_POSITION = Gamma::Vec3f(-3985.f, 2235.f, -1320.f);

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
constexpr static u16 TOTAL_PARTICLES = 30;
constexpr static float PARTICLE_SPAWN_DELAY = 1.f / float(TOTAL_PARTICLES);

constexpr static float INITIAL_DAY_NIGHT_CYCLE_TIME = 2.f;