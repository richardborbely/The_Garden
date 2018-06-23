#version 440

// Directional light structure
#ifndef DIRECTIONAL_LIGHT
#define DIRECTIONAL_LIGHT
struct directional_light {
  vec4 ambient_intensity;
  vec4 light_colour;
  vec3 light_dir;
};
#endif

// Point light information
#ifndef POINT_LIGHT
#define POINT_LIGHT
struct point_light {
  vec4 light_colour;
  vec3 position;
  float constant;
  float linear;
  float quadratic;
};
#endif

// Spot light data
#ifndef SPOT_LIGHT
#define SPOT_LIGHT
struct spot_light {
  vec4 light_colour;
  vec3 position;
  vec3 direction;
  float constant;
  float linear;
  float quadratic;
  float power;
};
#endif

// A material structure
#ifndef MATERIAL
#define MATERIAL
struct material {
  vec4 emissive;
  vec4 diffuse_reflection;
  vec4 specular_reflection;
  float shininess;
};
#endif

// Forward declarations of functions
vec4 calculate_direction(in directional_light light, in material mat, in vec3 normal, in vec3 view_dir, in vec4 tex_colour);
vec4 calculate_point(in point_light points, in material mat, in vec3 position, in vec3 normal, in vec3 view_dir, in vec4 tex_colour);
vec4 calculate_spot(in spot_light spots, in material mat, in vec3 position, in vec3 normal, in vec3 view_dir, in vec4 tex_colour);
float calculate_shadow(in sampler2D shadow_map, in vec4 light_space_pos);

// Directional light information
uniform directional_light light;
// Point lights being used in the scene
uniform point_light points[7];
// Spot lights being used in the scene
uniform spot_light spots[5];
// Material of the object being rendered
uniform material mat;
// Position of the eye
uniform vec3 eye_pos;
// Texture to sample from
uniform sampler2D tex;
// Shadow map to sample from
uniform sampler2D shadowMap;
// Cube map texture
uniform samplerCube cubeMap;
// Optional renders
uniform float fresnelIntensity;

// Incoming position
layout(location = 0) in vec3 position;
// Incoming texture coordinate
layout(location = 1) in vec2 tex_coord;
// Incoming normal
layout(location = 2) in vec3 normal;
// Incoming light space position
layout(location = 3) in vec4 light_space_pos;
// Incoming reflected vector
layout(location = 4) in vec3 reflected_vector;
// Incoming refracted vector
layout(location = 5) in vec3 refracted_vector;
// Outgoing reflection vector
layout(location = 6) in vec3 fresnel_vector;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main() {
colour = vec4(0.0, 0.0, 0.0, 1.0);
	// Calculate view direction
	vec3 view_dir = normalize(eye_pos - position);
	// Sample texture
	vec4 tex_colour = texture(tex, tex_coord);

  // Calculate directional light
	colour += calculate_direction(light, mat, normal, view_dir, tex_colour);
	// Calculate point lights
	for (int i = 0; i < points.length(); i++)
	{
		colour += calculate_point(points[i], mat, position, normal, view_dir, tex_colour);
	}
	// Calculate spot lights
	for (int i = 0; i < spots.length(); i++)
	{
    if(i == 4){// If spotlight 4 -> add shade
      // Calculate shade factor
      float shade = calculate_shadow(shadowMap, light_space_pos);
      // Scale colour by shade
      colour *= shade;
    }
    colour += calculate_spot(spots[i], mat, position, normal, view_dir, tex_colour);
	}

  // FRESNEL
  // (uncomment /// lines and remove clamp to have proper fresnel effect)
  vec4 reflectedColour = texture(cubeMap, reflected_vector);
  ///vec4 refractedColour = texture(cubeMap, refracted_vector);
    // Factor = dot product of vector and surface normal (object points upwards on scene)
  float fresnelFactor = clamp(dot(fresnel_vector, vec3(0.0f, 1.0f, 0.0f)), 0.4f, 1.0f);
  ///vec4 fresnelColour = mix(reflectedColour, refractedColour, fresnelFactor);

  ///colour = mix(colour, reflectedColour, fresnelIntensity);
  colour = mix(reflectedColour, colour, fresnelFactor); // only mix with reflection, this looks much nicer on the scene

	colour.a = 1.0f;
}
