#version 440

// Model transformation matrix
uniform mat4 M;
// Transformation matrix
uniform mat4 MVP;
// Normal matrix
uniform mat3 N;
// The light transformation matrix
uniform mat4 lightMVP;
// Eye pos
uniform vec3 eye_pos;


// Incoming position
layout (location = 0) in vec3 position;
// Incoming normal
layout (location = 2) in vec3 normal;
// Incoming texture coordinate
layout (location = 10) in vec2 tex_coord_in;

// Outgoing position
layout (location = 0) out vec3 vertex_position;
// Outgoing texture coordinate
layout (location = 1) out vec2 tex_coord_out;
// Outgoing transformed normal
layout (location = 2) out vec3 transformed_normal;
// Outgoing position in light space
layout (location = 3) out vec4 vertex_light;
// Outgoing reflection vector
layout (location = 4) out vec3 reflected_vector;
// Outgoing reflection vector
layout (location = 5) out vec3 refracted_vector;
// Outgoing reflection vector
layout(location = 6) out vec3 fresnel_vector;

void main()
{
	// Calculate screen position
	gl_Position = MVP * vec4(position, 1.0);

	// Output other values to fragment shader
	// Output world position of vertex
	vertex_position = (M * vec4(position, 1.0f)).xyz;
	tex_coord_out = tex_coord_in;
	transformed_normal = N * normal;
	// Transform position into light space
	vertex_light = lightMVP * vec4(position, 1.0f);

	// Optional renders
	vec3 viewVector = normalize(vertex_position - eye_pos);
		// Reflection
		reflected_vector = reflect(viewVector, transformed_normal);
		// Refraction
		refracted_vector = refract(viewVector, transformed_normal, 1.0/1.33);
		// Fresnel
		fresnel_vector = normalize(eye_pos - vertex_position); // opposite of viewvector
}
