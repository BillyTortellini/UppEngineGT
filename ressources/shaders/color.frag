#version 430 core

in vec3 f_Color;
in vec2 f_uv;

out vec4 o_Color;

float N21(vec2 v)
{
    return fract(sin(v.x*2334.22) + sin(v.y*-3432.23));
}

void main()
{
    //o_Color = vec4(f_uv, 0, 1.0);
    //float val = N21(f_uv);
    //o_Color = vec4(vec3(val)*f_Color, 1.0);
    o_Color = vec4(f_Color, 1);
    //o_Color = vec4(0, 1, 0, 1);
}
