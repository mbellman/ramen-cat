#version 460 core

#define MAX_PATH_POINTS 10

struct ParticleSystem {
  int total;
  vec3 spawn;
  float spread;
  float minimum_radius;
  float median_speed;
  float speed_variation;
  float median_size;
  float size_variation;
  float deviation;
};

struct ParticlePath {
  int total;
  vec3 points[MAX_PATH_POINTS];
  bool is_circuit;
};

uniform mat4 matProjection;
uniform mat4 matView;
uniform float time;
uniform ParticleSystem particles;
uniform ParticlePath path;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec2 vertexUv;
layout (location = 4) in mat4 modelMatrix;

out vec2 fragUv;
flat out vec3 color;

#include "utils/gl.glsl";

float particle_id = float(gl_InstanceID);

// @todo improve the degree of randomness here + move to utils
float random(vec2 seed){
  return fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453);
}

float random(float seed) {
  return random(vec2(seed));
}

float random(float low, float high, float seed) {
  float a = random(vec2(seed));

  return mix(low, high, a);
}

/**
 * Adapted from http://paulbourke.net/miscellaneous/interpolation/
 *
 * Performs cubic spline interpolation.
 */
float interpolateCubicSpline(float a, float b, float c, float d, float alpha) {
  float m = alpha * alpha;

  float a0 = d - c - a + b;
  float a1 = a - b - a0;
  float a2 = c - a;
  float a3 = b;
  
  return (a0 * alpha * m) + (a1 * m) + (a2 * alpha) + a3;
}

/**
 * Performs cubic spline interpolation between two points
 * p2 and p3, using the neighboring points p1 and p4.
 */
vec3 interpolatePoints(vec3 p1, vec3 p2, vec3 p3, vec3 p4, float alpha) {
  return vec3(
    interpolateCubicSpline(p1.x, p2.x, p3.x, p4.x, alpha),
    interpolateCubicSpline(p1.y, p2.y, p3.y, p4.y, alpha),
    interpolateCubicSpline(p1.z, p2.z, p3.z, p4.z, alpha)
  );
}

/**
 * Returns a path control point by index, wrapping around
 * for out-of-bounds indexes.
 */
vec3 getPathPoint(int index) {
  int wrapped_index = int(mod(float(index), float(path.total)));
  
  return path.points[wrapped_index];
}

/**
 * @todo description
 */
float getParticleSpeed() {
  return particles.median_speed + random(-particles.speed_variation, particles.speed_variation, particle_id);
}

/**
 * Returns the initial particle position, pseudo-randomly
 * determined by its ID.
 */
vec3 getInitialParticlePosition() {
  float radius = particles.minimum_radius + particles.spread * sqrt(0.001 + random(particle_id * 1.255673));

  float x = random(-1, 1, particle_id * 1.1);
  float y = random(-1, 1, particle_id * 1.2);
  float z = random(-1, 1, particle_id * 1.3);

  return radius * normalize(vec3(x, y, z));
}

/**
 * @todo description
 */
vec3 getPathParticlePosition() {
  if (path.total == 0) {
    return vec3(0);
  }

  float particle_speed = getParticleSpeed();
  float path_progress = fract(random(0, 1, particle_id * 1.1) + time * (particle_speed / path.total));

  float path_position = path.is_circuit
    ? path_progress * path.total
    : path_progress * (path.total - 1);

  int path_index = int(floor(path_position));

  vec3 p1 = getPathPoint(path_index - 1);
  vec3 p2 = getPathPoint(path_index);
  vec3 p3 = getPathPoint(path_index + 1);
  vec3 p4 = getPathPoint(path_index + 2);

  float interpolation_factor = fract(path_position);

  return interpolatePoints(p1, p2, p3, p4, interpolation_factor);
}

/**
 * @todo description
 */
vec3 getParticleDeviation() {
  float particle_speed = getParticleSpeed();
  float deviation_factor = particles.deviation * sin(particle_id + time * particle_speed);

  return vec3(
    deviation_factor * random(-1, 1, particle_id * 1.4),
    deviation_factor * random(-1, 1, particle_id * 1.5),
    deviation_factor * random(-1, 1, particle_id * 1.6)
  );
}

/**
 * Returns the particle's current position as a function of time.
 */
vec3 getParticlePosition() {
  return particles.spawn + getInitialParticlePosition() + getPathParticlePosition() + getParticleDeviation();
}

void main() {
  vec3 position = getParticlePosition();

  float r = particle_id / float(particles.total);
  // @todo make size oscillation rate configurable
  float scale = particles.median_size + particles.size_variation * sin(time * 3.0 + r * 500.0);

  // @hack invert Z
  gl_Position = matProjection * matView * glVec4(position);
  gl_PointSize = 50.0 * scale / gl_Position.z;

  fragUv = vertexUv;
  // @todo make color configurable
  color = vec3(sin(r * 500.0) * 0.5 + 0.5, sin(particle_id + time * 2.0) * 0.5 + 0.5, cos(r * 1000.0) * 0.5 + 0.5);
}