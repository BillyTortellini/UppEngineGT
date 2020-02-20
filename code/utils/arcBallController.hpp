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

void init(ArcBallController* c, Camera3D* cam, float sensitivity, float wheelSensitivity) 
{
    c->cam = cam;    
    c->sphereCoords = vec2(0.0f);
    c->pos = vec3(0.0f);
    c->distToCenter = 0.0f;
    c->sensitivity = sensitivity;
    c->wheelSensitivity = wheelSensitivity;
}

void update(ArcBallController* c, GameState* state) 
{
    Input* input = &state->input;
    // Update sphereCoords
    c->sphereCoords -= vec2(input->deltaX, input->deltaY) * c->sensitivity;
    c->sphereCoords = sphericalNorm(c->sphereCoords);
    c->cam->dir = sp2eu(c->sphereCoords);

    // Update position correctly
    mat4 view = lookInDir(vec3(0), c->cam->dir, vec3(0, 1, 0)); 
    view = transpose(view);
    vec3 moveDir(0);
    if (input->keyDown[KEY_W]) {
        moveDir += vec3(0.0f, 0.0f, -1.0f);
    }
    if (input->keyDown[KEY_A]) {
        moveDir += vec3(-1.0f, 0.0f, 0.0f);
    }
    if (input->keyDown[KEY_S]) {
        moveDir += vec3(0.0f, 0.0f, 1.0f);
    }
    if (input->keyDown[KEY_D]) {
        moveDir += vec3(1.0f, 0.0f, 0.0f);
    }
    float speed = 3.0f;
    if (input->keyDown[KEY_SHIFT]) {
        speed = 6.0f;
    }
    moveDir = view * moveDir * speed * (float)state->time.tslf; // Test if works
    c->pos += moveDir;

    // Update distance
    c->distToCenter -= input->mouseWheel;
    float dist = powf(2.0f, c->distToCenter * c->wheelSensitivity);
    
    // Update camera
    c->cam->pos = c->pos - c->cam->dir * dist;
    c->cam->view = lookInDir(c->cam->pos, c->cam->dir, vec3(0.0f, 1.0f, 0.0f));
}







#endif
