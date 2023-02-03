#define USE_DEV_LIGHT_DISCS 0

vec4 frag_color_and_depth = texture(texColorAndDepth, fragUv);
vec3 position = getWorldPosition(frag_color_and_depth.w, fragUv, matInverseProjection, matInverseView);
vec3 surface_to_light = light.position - position;
float light_distance = length(surface_to_light);

if (light_distance > light.radius) {
  #if USE_DEV_LIGHT_DISCS == 0
    discard;
  #endif
}

vec3 normalized_surface_to_light = surface_to_light / light_distance;

// @optimize normalize light direction outside of the shader
float fragment_alignment = dot(normalized_surface_to_light * -1, normalize(light.direction));
float cone_edge_alignment = 1.0 - (light.fov / 180.0);

if (fragment_alignment < cone_edge_alignment) {
  #if USE_DEV_LIGHT_DISCS == 0
    // Discard any fragments outside of the light cone
    discard;
  #else
    fragment_alignment = 0.0;
    cone_edge_alignment = 0.0;
  #endif
}

vec4 frag_normal_and_material = texture(texNormalAndMaterial, fragUv);
vec3 frag_normal = frag_normal_and_material.xyz;
vec3 frag_color = frag_color_and_depth.rgb;

// @todo refactor
float material = frag_normal_and_material.w;
float emissivity = floor(material) / 10.0;
float roughness = fract(material);

float cone_edge_range = 1.0 - cone_edge_alignment;
float cone_edge_proximity = fragment_alignment - cone_edge_alignment;
float spot_factor = pow(cone_edge_proximity / cone_edge_range, 0.7);

vec3 normalized_surface_to_camera = normalize(cameraPosition - position);
vec3 half_vector = normalize(normalized_surface_to_light + normalized_surface_to_camera);
float incidence = max(dot(normalized_surface_to_light, frag_normal), 0.0);
float attenuation = pow(1.0 / light_distance, 2);
float specularity = pow(max(dot(half_vector, frag_normal), 0.0), 50) * (1.0 - roughness);

// Have light intensity 'fall off' toward radius boundary
float hack_radial_influence = max(1.0 - light_distance / light.radius, 0.0);
// Taper light intensity more softly to preserve light with distance
float hack_soft_tapering = (20.0 * (light_distance / light.radius));

vec3 radiant_flux = light.color * light.power * light.radius;
vec3 diffuse_term = frag_color * radiant_flux * incidence * attenuation * hack_radial_influence * hack_soft_tapering * (1.0 - specularity) * sqrt(roughness);
vec3 specular_term = 5.0 * radiant_flux * specularity * attenuation;

#if USE_DEV_LIGHT_DISCS == 1
  vec3 illuminated_color = light.color * vec3(0.3) + (diffuse_term + specular_term) * (1.0 - min(1.0, emissivity)) * spot_factor;
#else
  vec3 illuminated_color = (diffuse_term + specular_term) * (1.0 - min(1.0, emissivity)) * spot_factor;
#endif