#define NONE 0
#define FLOWER 1
#define BRANCH 2
#define LEAF 3

struct FoliageBehavior {
  int type;
  float speed;
};

uniform FoliageBehavior foliage;
uniform float time;

vec3 getFlowerFoliageOffset(vec3 world_position) {
  float vertex_distance_from_ground = abs(vertexPosition.y);
  float rate = time * foliage.speed;
  float x_3 = world_position.x / 3.0;
  float z_3 = world_position.z / 3.0;
  float displacement_factor = min(1.0, pow(vertex_distance_from_ground / 1.5, 2));

  // @todo allow some of these magic numbers to be configurable
  float offset_x = sin(rate + x_3 + z_3) + sin(rate * 3.1 + z_3) * 0.2 + cos(rate * 4.0 + x_3 + z_3) * 0.1;
  float offset_y = 0.0;
  float offset_z = sin(rate + x_3 + z_3) + cos(rate * 1.3 + x_3) * 0.2 + sin(rate * 4.0 + x_3 + z_3) * 0.1;

  return vec3(offset_x, offset_y, offset_z) * displacement_factor;
}

vec3 getBranchFoliageOffset(vec3 world_position) {
  // @todo
  return vec3(0);
}

vec3 getLeafFoliageOffset(vec3 world_position) {
  float x_3 = world_position.x / 3.0;
  float y_3 = world_position.y / 3.0;
  float z_3 = world_position.z / 3.0;
  float rate = time * foliage.speed;

  float offset_x = sin(rate + z_3) + sin(rate * 0.5 + z_3) * 0.5;
  float offset_y = cos(rate + x_3) + cos(rate * 0.7 + x_3) * 0.5;
  float offset_z = sin(rate + y_3) + sin(rate * 0.9 + y_3) * 0.5;

  return vec3(offset_x, offset_y, offset_z) * 0.1;
}