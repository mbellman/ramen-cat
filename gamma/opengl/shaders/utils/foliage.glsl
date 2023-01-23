#define NONE 0
#define FLOWER 1
#define BRANCH 2
#define LEAF 3

struct FoliageBehavior {
  int type;
  float speed;
  float factor;
};

uniform FoliageBehavior foliage;
uniform float time;

vec3 getFlowerFoliageOffset(vec3 vertex_position, vec3 world_position) {
  float vertex_distance_from_ground = abs(vertex_position.y * 2.0);
  float rate = time * foliage.speed;
  float x_3 = world_position.x / 3.0;
  float z_3 = world_position.z / 3.0;
  float displacement_factor = foliage.factor * min(1.0, pow(vertex_distance_from_ground / 1.5, 2));

  // @todo allow some of these magic numbers to be configurable
  float offset_x = sin(rate + x_3 + z_3) + sin(rate * 3.1 + z_3) * 0.2 + cos(rate * 4.0 + x_3 + z_3) * 0.1;
  float offset_y = 0.0;
  float offset_z = sin(rate + x_3 + z_3) + cos(rate * 1.3 + x_3) * 0.2 + sin(rate * 4.0 + x_3 + z_3) * 0.1;

  return displacement_factor * vec3(offset_x, offset_y, offset_z);
}

vec3 getBranchFoliageOffset(vec3 vertex_position, vec3 world_position) {
  // @todo
  return vec3(0);
}

vec3 getLeafFoliageOffset(vec3 vertex_position, vec3 world_position) {
  float rate = time * foliage.speed;

  float vy = vertex_position.y;

  float distance_factor = pow(vy / (1.0 + vy), 2);
  float displacement_factor = foliage.factor * 15.0 * distance_factor;
  float y_offset = displacement_factor * (2.0 * sin(rate + gl_InstanceID) + sin(rate + 2.0 * vy));

  return vec3(0, y_offset, 0);
}