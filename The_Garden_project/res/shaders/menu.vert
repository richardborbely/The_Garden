#version 440

// Model transformation matrix
uniform mat4 M;
// Transformation matrix
uniform mat4 MVP;
// Normal matrix
uniform mat3 N;

// Incoming
layout(location = 0) in vec3 position;
layout(location = 2) in vec3 normal;
layout(location = 10) in vec2 tex_coord_in;

// Outgoing
layout(location = 0) out vec3 vertex_position;
layout(location = 1) out vec3 transformed_normal;
layout(location = 2) out vec2 tex_coord_out;

void main()
{
  gl_Position = MVP * vec4(position, 1);

  vertex_position = position;
  tex_coord_out = tex_coord_in;
  transformed_normal = N * normal;
}
