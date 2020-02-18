#version 430 core

in vec3 a_Pos;
in vec2 a_UV;

out vec2 f_uv;

uniform mat4 u_MVP;

void main() {
    gl_Position = u_MVP * vec4(a_Pos, 1);
    f_uv = a_UV;
}


