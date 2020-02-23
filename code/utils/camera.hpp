#ifndef __CAMERA_HPP__
#define __CAMERA_HPP__

struct Camera3D
{
    vec3 pos;
    vec3 dir;
    mat4 view;
    mat4 projection;
};

void init(Camera3D* cam, int width, int height) {
    cam->projection = projection(0.01f, 100.0, d2r(90), (float)width / height);
    cam->pos = vec3(0);
    cam->dir = vec3(0);
}








#endif
