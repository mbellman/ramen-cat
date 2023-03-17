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

vec4 frag_normal_and_material = texture(texNormalAndMaterial, fragUv);
vec3 frag_normal = frag_normal_and_material.xyz;
vec3 frag_color = frag_color_and_depth.rgb;

// @todo refactor
float material = frag_normal_and_material.w;
float emissivity = floor(material) / 10.0;
float roughness = fract(material);

vec3 normalized_surface_to_light = surface_to_light / light_distance;
vec3 normalized_surface_to_camera = normalize(cameraPosition - position);
vec3 half_vector = normalize(normalized_surface_to_light + normalized_surface_to_camera);
float incidence = max(dot(normalized_surface_to_light, frag_normal), 0.0);
float attenuation = pow(1.0 / light_distance, 2);
float specularity = pow(max(dot(half_vector, frag_normal), 0.0), 50) * (1.0 - roughness);

if (incidence == 0.0) specularity = 0.0;

// Define a non-linear light intensity fall-off toward the radius boundary
float hack_diffuse_radial_influence = (1.0 - pow(clamp(light_distance / light.radius, 0.0, 1.0), 2));
float hack_specular_radial_influence = (1.0 - pow(clamp(light_distance / light.radius, 0.0, 1.0), 10));
// Taper light intensity more softly to preserve light with distance
float hack_soft_tapering = (20.0 * (light_distance / light.radius));

vec3 radiant_flux = light.color * light.power * light.radius;
vec3 diffuse_term = frag_color * radiant_flux * incidence * attenuation * hack_diffuse_radial_influence * hack_soft_tapering * (1.0 - specularity) * sqrt(roughness);
vec3 specular_term = 5.0 * radiant_flux * specularity * attenuation * hack_specular_radial_influence;

// @todo bring this back in later
// vec3 volumetric_term = light.color * incidence * intensity;

#if USE_DEV_LIGHT_DISCS == 1
  vec3 illuminated_color = light.color * vec3(0.3) + (diffuse_term + specular_term) * (1.0 - min(1.0, emissivity));
#else
  vec3 illuminated_color = (diffuse_term + specular_term) * (1.0 - min(1.0, emissivity));
#endif