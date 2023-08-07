#version 450

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec3 out_color;

layout(set = 0, binding = 0) uniform UBO {
  mat4 model;
  mat4 view;
  mat4 projection;
} ubo;
layout(set = 1, binding = 1) uniform sampler2D tex_sampler;

void main()
{
  gl_Position = ubo.projection * ubo.view * ubo.model * vec4(in_pos, 0.0, 1.0);
  out_color = in_color;
}

// binding 0 = Uniform of 3 mat4
// location 0 = vec2float
// location 1 = vec3float
