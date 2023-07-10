#version 450

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec2 in_tex_coord;

layout(location = 0) out vec4 frag_color;

void main() 
{
  frag_color = vec4(in_tex_coord, 0.0, 1.0);
}
