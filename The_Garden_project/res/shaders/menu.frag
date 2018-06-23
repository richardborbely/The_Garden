#version 440

// Texture captured from the scene
uniform sampler2D tex;

// Incoming position
layout(location = 0) in vec3 vertex_position;
// Incoming normal
layout(location = 1) in vec3 normal;
// Incoming texture coordinate
layout(location = 2) in vec2 tex_coord;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main()
{
  // Sample texture
	vec4 tex_colour = texture(tex, tex_coord);
  colour = tex_colour;

  colour.a = 1.0f;
}
