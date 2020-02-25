#ifndef __ARCBALLCONTROLLER_HPP__
#define __ARCBALLCONTROLLER_HPP__

#include "../rendering/renderer.hpp"

struct ArcBallController
{
    Camera3D* cam;
    vec2 sphereCoords;
    float sensitivity;
    float wheelSensitivity;
    float distToCenter; 
    vec3 pos;
};

void print(ArcBallController* c) {
    loggf("c->cam: %p\n", c->cam);
    loggf("c->sphereCoords: %3.2f, %3.2f\n", c->sphereCoords.x, c->sphereCoords.y);
    loggf("c->sensitivity: %f\n", c->sensitivity);
    loggf("c->distToCenter: %f\n", c->distToCenter);
    loggf("c->pos: %3.2f, %3.2f, %3.2f\n", c->pos.x, c->pos.y, c->pos.z);
}

void init(ArcBallController* c, Camera3D* cam, float sensitivity, float wheelSensitivity) 
{
    c->cam = cam;    
    c->sphereCoords = vec2(0.0f);
    c->pos = vec3(0.0f);
    c->distToCenter = 5.0f;
    c->sensitivity = sensitivity;
    c->wheelSensitivity = wheelSensitivity;
}

void update(ArcBallController* c, GameState* state) 
{
    Input* input = &state->input;
    // Update sphereCoords
    if (input->mouseDown[MOUSE_LEFT]) {
        c->sphereCoords -= vec2(input->deltaX, input->deltaY) * c->sensitivity;
    }
    c->sphereCoords = sphericalNorm(c->sphereCoords);
    c->cam->dir = sp2eu(c->sphereCoords);

    // Update distance
    c->distToCenter -= input->mouseWheel;
    float dist = powf(2.0f, c->distToCenter * c->wheelSensitivity);

    // Update position correctly
    if (input->mouseDown[MOUSE_RIGHT]) 
    {
        mat4 view = lookInDir(vec3(0), c->cam->dir, vec3(0, 1, 0)); 
        view = transpose(view);
        vec3 moveDir = vec3(-(float)input->deltaX, (float)input->deltaY, 0);
        moveDir.x /= (float)state->windowState.width*.5f;
        moveDir.y /= (float)state->windowState.height*.5f;
        float speed = 3.0f;
        if (input->keyDown[KEY_SHIFT]) {
            speed = 6.0f;
        }
        moveDir = view * moveDir * dist;
        c->pos += moveDir;
    }


    // Update camera
    c->cam->pos = c->pos - c->cam->dir * dist;
    c->cam->view = lookInDir(c->cam->pos, c->cam->dir, vec3(0.0f, 1.0f, 0.0f));
}







#endif
