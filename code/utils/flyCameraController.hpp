#ifndef __FLY_CAMERA_CONTROLLER_HPP__
#define __FLY_CAMERA_CONTROLLER_HPP__

#include "../rendering/renderer.hpp"

struct FlyCameraController
{
    Camera3D* cam;
    vec2 sphereCoords;
    float sensitivity;
    float speed;
    float speedBoost;
};

void init(FlyCameraController* f, Camera3D* cam, float sensitivity, float speed, float speedBoost) 
{
    f->cam = cam;    
    f->sphereCoords = vec2(0.0f);
    f->sensitivity = sensitivity;
    f->speed = speed;
    f->speedBoost = speedBoost;
}

void update(FlyCameraController* f, GameState* state) 
{
    Input* input = &state->input;
    // Update sphereCoords
    f->sphereCoords -= vec2(input->deltaX, input->deltaY) * f->sensitivity;
    f->sphereCoords = sphericalNorm(f->sphereCoords);
    f->cam->dir = sp2eu(f->sphereCoords);

    // Update position correctly
    mat4 view = lookInDir(vec3(0), f->cam->dir, vec3(0, 1, 0)); 
    view = transpose(view);
    vec3 moveDir = vec3(0);
    if (input->keyDown[KEY_W]) {
        moveDir += vec3(0, 0, -1);
    }
    if (input->keyDown[KEY_A]) {
        moveDir += vec3(-1, 0, 0);
    }
    if (input->keyDown[KEY_S]) {
        moveDir += vec3(0, 0, 1);
    }
    if (input->keyDown[KEY_D]) {
        moveDir += vec3(1, 0, 0);
    }
    moveDir *= f->speed;
    if (input->keyDown[KEY_SHIFT]) {
        moveDir *= f->speedBoost;
    }
    moveDir = view * moveDir;
    f->cam->pos += moveDir * (float)gameState->time.tslf;

    // Update camera
    f->cam->view = lookInDir(f->cam->pos, f->cam->dir, vec3(0.0f, 1.0f, 0.0f));
}








#endif
