#version 430 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;

layout (location = 0) uniform mat4 u_VP;
layout (location = 1) uniform vec3 u_pos;

out vec3 fColor;

void main()
{
    gl_Position = u_VP * vec4(pos*0.3 + u_pos, 1.0); 
    fColor = color;
}
