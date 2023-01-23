#pragma once

#define export constexpr static

/**
 * Camera constants
 * ----------------
 */
export float CAMERA_FIRST_PERSON_RADIUS  = 5.f;
export float CAMERA_NORMAL_BASE_RADIUS = 300.f;
export float CAMERA_ZOOM_OUT_BASE_RADIUS = 600.f;
export float CAMERA_RADIUS_ALTITUDE_MULTIPLIER = 200.f;

export float CAMERA_BASE_FOV = 45.f;
export float CAMERA_FOV_VELOCITY_MULTIPLIER = 20.f;
export float CAMERA_FOV_MINIMUM_SPEED = 800.f;
export float CAMERA_FOV_SPEED_INTERVAL = 500.f;

/**
 * Entity constants
 * ----------------
 */
export float NPC_RADIUS = 20.f;
export float NPC_HEIGHT = 70.f;
export float NPC_INTERACTION_TRIGGER_DISTANCE = 120.f;
export float NPC_INTERACTION_CAMERA_RADIUS = 150.f;

export float SLINGSHOT_INTERACTION_TRIGGER_DISTANCE = 150.f;
export float SLINGSHOT_WIND_UP_CAMERA_RADIUS = 150.f;
export float SLINGSHOT_WIND_UP_DURATION_SECONDS = 0.5f;

export float LANTERN_HORIZONTAL_DRIFT = 15.f;
export float LANTERN_VERTICAL_DRIFT = 5.f;