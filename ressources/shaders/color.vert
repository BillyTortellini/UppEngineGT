#version 430 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec3 color;

layout (location = 0) uniform vec2 u_pos;
layout (location = 1) uniform mat4 u_VP;

out vec3 fColor;

void main()
{
    vec2 scaled = pos * .3;
    gl_Position = u_VP * vec4(scaled + u_pos, 0.0, 1.0); 
    //gl_Position = vec4(pos, 0.0, 1.0); 
    fColor = color;
}
