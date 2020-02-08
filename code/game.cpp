#include <cstring>
#include <cstdio>
#include <cmath>
#include <cstdio>

// Own file includes
#include "platform.hpp"
#include "umath.hpp"
#include "datatypes.hpp"
//#include "sound.hpp"

// What i want to do:
// * Play sampled sounds at a given time
// * Play sounds when a key is pressed

#define MAX_QUEUED_SOUNDS 16
struct GameData
{
    vec2 pos;
};
GameState* gameState;
GameData* gameData;

#define print(format, ...) { \
        char buf[256]; \
        snprintf(buf, 256, format, ##__VA_ARGS__); \
        gameState->services.debugPrint(buf); \
    };

void toScreenCoords(vec2 v, int height, int width, int& x, int& y)
{
    v = v * 0.5f + 0.5f;
    x = (int)(v.x * width);
    y = (int)((1.0f - v.y) * height);
    return;
}

void trim(int& x, int min, int max) {
    if (x < min) x = min;
    if (x > max) x = max;
    return;
}

void renderScene(VideoData* videoData, float time)
{
    int width = videoData->width;
    int height = videoData->height;
    vec2 dim(width, height);
    vec2 stretch(1.0f);
    if (dim.x > dim.y) {
        stretch.x = stretch.x * dim.x/dim.y;
    }
    else {
        stretch.y = stretch.y * dim.y/dim.x;
    }

    u8 colRed = (u8) fmodf(time, 0.5);

    memset(videoData->pixels, 0, width * height * sizeof(Pixel));

    vec2 circlePos = vec2(sinf(time) * 0.5f, 0.0f);
    float radius = 0.1f;
    vec2 min = circlePos - radius;
    vec2 max = circlePos + radius;
    int minX, minY, maxX, maxY;
    toScreenCoords(min, width, height, minX, minY);
    trim(minX, 0, width);
    trim(minY, 0, height);
    toScreenCoords(min, width, height, maxX, maxY);
    trim(maxX, 0, width);
    trim(maxY, 0, height);

    for (int y = minY; y < maxY; y++) {
        for (int x = minX; x < maxX; x++) {
            vec2 p(x, y);
            p = p / dim;
            p = p * 2 - 1;
            p = p * stretch;

            if (dist(p, circlePos) < radius) {
                videoData->pixels[x + y * width].r = 255;
                videoData->pixels[x + y * width].g = 255;
                videoData->pixels[x + y * width].b = 255;
                videoData->pixels[x + y * width].a = 0;
            }
        }
    }
}

void updatePlayerPos(GameState* state) 
{
    gameData = (GameData*) state->memory.memory;     
    vec2 dir(0.0f);
    if (state->input.keyDown[KEY_A]) {
        dir += vec2(-1.0f, 0.0f);
    }
    if (state->input.keyDown[KEY_D]) {
        dir += vec2(1.0f, 0.0f);
    }
    if (state->input.keyDown[KEY_W]) {
        dir += vec2(0.0f, 1.0f);
    }
    if (state->input.keyDown[KEY_S]) {
        dir += vec2(0.0f, -1.0f);
    }

    dir.y *= -1;
    float speed = 2.0f;
    vec2 change = dir * (float)state->time.tslf * speed;
    gameData->pos = gameData->pos + change;
}

extern "C"
{
    DECLARE_EXPORT void gameAudio(GameState* state, byte* stream, int length) {}

    DECLARE_EXPORT void gameInit(GameState* state) 
    {
        gameData = (GameData*) state->memory.memory;
        gameData->pos = vec2(0.0f);
    }

    bool freshLoad = true;
    DECLARE_EXPORT void gameTick(GameState* state) 
    {
        gameState = state;
        gameData = (GameData*) state->memory.memory;
        gameState->windowState.continuousDraw = true;
        gameState->windowState.fps = 30;

        if (freshLoad) {
            print("Hello there\n");
            freshLoad = false;
        }

        float time = (float) gameState->time.now;

        //updatePlayerPos(state);

        Input* input = &state->input;
        if (input->keyPressed[KEY_ESCAPE]) {
            state->windowState.quit = true;
        }
        if (input->keyPressed[KEY_F11]) {
            state->windowState.fullscreen = !state->windowState.fullscreen;
        }

        VideoData* vid = &state->videoData;
        renderScene(vid, time);
    }

    DECLARE_EXPORT void gameShutdown(GameState* state)
    {

    }
}

