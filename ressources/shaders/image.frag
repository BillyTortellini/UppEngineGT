#version 430 core

in vec2 f_uv;
out vec4 o_color;

uniform sampler2D image;

void main()
{
    vec3 color = vec3(texture(image, f_uv));
    o_color = vec4(color, 1);
}
