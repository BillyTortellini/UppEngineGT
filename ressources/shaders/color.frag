#version 430 core

in vec3 f_Color;
out vec4 o_Color;

void main()
{
    o_Color = vec4(f_Color, 1.0);
    //oColor = vec4(0.0, 1.0, 0.0, 1.0);
}
