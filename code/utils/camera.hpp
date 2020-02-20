#ifndef __CAMERA_HPP__
#define __CAMERA_HPP__

struct Camera3D
{
    vec3 pos;
    vec3 dir;
    mat4 view;
    mat4 projection;
    mat4 vp;
    int lastUpdateFrame;
};

void init(Camera3D* cam, GameState* gameState) {
    cam->projection = projection(0.01f, 100.0, d2r(90), (float)gameState->windowState.width / gameState->windowState.height);
    cam->lastUpdateFrame = -1;
    cam->pos = vec3(0);
    cam->dir = vec3(0);
}

void update(Camera3D* cam, int frame)
{
    if (cam->lastUpdateFrame == frame)
        return;
    cam->vp = cam->projection * cam->view;
    cam->lastUpdateFrame = frame;
}








#endif
