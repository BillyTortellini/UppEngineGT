#version 430 core

in vec3 a_Pos;
in vec2 a_UV;

out vec2 f_uv;

uniform mat4 u_VP;

void main() {
    gl_Position = u_VP * vec4(a_Pos*5-vec3(0,4.0,0), 1);
    f_uv = a_UV;
}


