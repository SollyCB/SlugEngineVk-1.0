#version 450

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec3 out_color;

layout(binding = 0) uniform UBO {
  mat4 model;
  mat4 view;
  mat4 projection;
} ubo;

void main() 
{
  gl_Position = ubo.projection * ubo.view * ubo.model * vec4(in_pos, 0.0, 1.0);
  out_color = in_color;
}
