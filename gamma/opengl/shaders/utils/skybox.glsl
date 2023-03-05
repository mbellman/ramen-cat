const float CLOUD_MOVEMENT_RATE = 0.001;

vec4 getSkyColor(vec3 sky_direction, vec3 sun_direction, vec3 base_sun_color, vec3 atmosphere_color, float altitude) {
  float sunlight_factor = 1.5;
  float moonlight_factor = 2.0;
  float altitude_y_offset = altitude / 50000.0;

  float y = sky_direction.y + altitude_y_offset;
  float cos_y = cos(y);

  // @todo make configurable
  vec3 BASE_DAYLIGHT_COLOR = vec3(0.25, 0.35, 1.0);
  vec3 BASE_NIGHTTIME_COLOR = vec3(0.1, 0.2, 0.6);

  float daylight_factor = dot(sun_direction, vec3(0, 1, 0));
  float altitude_factor = pow(cos_y, 3);
  vec3 ambient_sky_color = mix(BASE_NIGHTTIME_COLOR, BASE_DAYLIGHT_COLOR, daylight_factor) * altitude_factor;

  float sun_radius_factor = dot(sky_direction, sun_direction);
  float moon_radius_factor = dot(sky_direction, sun_direction * -1.0);
  vec3 sun_color = base_sun_color * sunlight_factor * pow(max(sun_radius_factor, 0.0), 100);
  vec3 moon_color = vec3(1) * moonlight_factor * pow(max(moon_radius_factor, 0.0), 500);

  vec3 haze_color = atmosphere_color * pow(cos_y, 100);
  vec3 sky_color = ambient_sky_color + haze_color + sun_color + moon_color;

  float base_sky_brightness = 0.2;
  float sun_brightness = pow(max(0.0, sun_radius_factor), 50);
  float moon_brightness = pow(max(0.0, moon_radius_factor), 50);
  float sky_brightness = (base_sky_brightness + sun_brightness + moon_brightness) / (base_sky_brightness + 1.0);

  return vec4(sky_color, sky_brightness);
}