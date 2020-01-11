#include <cstring>
#include <cstdio>
#include <cmath>
#include <cstdio>

// Own file includes
#include "game.hpp"
#include "umath.hpp"
#include "datatypes.hpp"

// What i want to do:
// * Play sampled sounds at a given time
// * Play sounds when a key is pressed

struct SoundSample
{
    int32 length;
    int32 sampleFreq;
    int16* samples;
};

struct ADSREnvelope
{
    double attack;
    double decay;
    double sustain;
    double release;
    double sustainTime;
};

namespace Waveform
{
    enum ENUM
    {
        SINUS = 0,
        SQUARE = 1,
        TRIANGLE = 2
    };
};

struct Oscillator
{
    Waveform::ENUM waveform;   
    double frequency;
};

double oscillatorGetAmp(Oscillator* osc, double t) 
{
    switch (osc->waveform)
    {
        case Waveform::SINUS:
        {
            return sin(t * 2 * PI * osc->frequency);
        }
        case Waveform::SQUARE:
        {
            int q = t * osc->frequency * 2;
            return q % 2 == 0 ? 1.0 : -1.0;
        }
        case Waveform::TRIANGLE:
        {
            double help = t * osc->frequency;
            int whole = help;
            double rest = help - whole;
            return whole % 2 == 0 ? rest : 1.0 - rest;
        }
    }

    return 0.0;
}

double envelopeGetAmp(ADSREnvelope* env, double t) 
{
    double& a = env->attack;
    double& d = env->decay;
    double& s = env->sustain;
    double& r = env->release;
    double& st = env->sustainTime;

    if (t < a)
    {
        return t / a;
    }
    else if (t < a + d)
    {
        double alpha = (t - a) / d;
        return alpha * s + (1.0-alpha) * 1.0;
    }
    else if (t < a + d + st)
    {
        return s;
    }
    else
    {
        double alpha = 1.0 - (t-a-d-st) / r;
        alpha = max(0.0, alpha);
        return alpha * s;
    }

    return 0.0;
}

void fillSoundEnvelope(SoundSample* sound, ADSREnvelope* env, Oscillator* osc)
{
    for (int i = 0; i < sound->length; i++)
    {
        double t = i / (double)sound->sampleFreq;

        int16* sample = &(sound->samples[i]);
        *sample = oscillatorGetAmp(osc, t) * envelopeGetAmp(env, t) * (INT16_MAX - 5);
    }
}

int envelopeSampleCount(ADSREnvelope* env, int sampleFreq) {
    int count = ((env->attack + env->decay + env->sustainTime + env->release) * sampleFreq) + 10;
    return count;
}

struct QueuedSound
{
    SoundSample* sound;
    uint64 startTick;
    bool playing;
};

#define MAX_QUEUED_SOUNDS 16
struct GameData
{
    vec2 pos;
    uint64 runningSampleIndex;
    QueuedSound queuedSounds[MAX_QUEUED_SOUNDS];
    SoundSample sample1;
    SoundSample sample2;
    int16 sample1Data[48000 * 5];
    int16 sample2Data[48000 * 5];
};
GameState* gameState;
GameData* gameData;

void playSoundSample(SoundSample* sample) 
{
    gameState->platformFunctions.lockSound();

    for (int i = 0; i < MAX_QUEUED_SOUNDS; i++) {
        QueuedSound* queuedSound = &gameData->queuedSounds[i];
        if (!queuedSound->playing)
        {
            queuedSound->playing = true;
            queuedSound->startTick = gameData->runningSampleIndex;
            queuedSound->sound = sample;
            break;
        }
    }

    gameState->platformFunctions.unlockSound();
}

void renderScene(byte* pixelData, int width, int height)
{
    vec2 playerPos = gameData->pos;
    vec2 dim(width, height);

    memset(pixelData, 0, width * height * 4);
    for (int yPos = 0; yPos < height; yPos++)
    {
        for (int xPos = 0; xPos < width; xPos++)
        {
            byte* pixel = pixelData + (xPos + yPos*width)*4;
            byte* red = pixel + 2;
            byte* green = pixel + 1;
            byte* blue = pixel + 0;

            vec2 pos = vec2(xPos, yPos) / dim;
            pos = pos*2-1;

            if (dim.x > dim.y)
                pos.x *= dim.x/dim.y;
            else 
                pos.y *= dim.y/dim.x;

            float d = dist(playerPos, pos);
            float r = .1f;
            uint8 i = d < r ? 255 : 0;

            //*blue = i;
            *red = i;
            //*green = i;
        }
    }
}

void updatePlayerPos(GameState* state) 
{
    gameData = (GameData*) state->memory.memory;     
    vec2 dir(0.0f);
    if (state->input.keysDown[KEY_A]) {
        dir += vec2(-1.0f, 0.0f);
    }
    if (state->input.keysDown[KEY_D]) {
        dir += vec2(1.0f, 0.0f);
    }
    if (state->input.keysDown[KEY_W]) {
        dir += vec2(0.0f, 1.0f);
    }
    if (state->input.keysDown[KEY_S]) {
        dir += vec2(0.0f, -1.0f);
    }

    dir.y *= -1;
    float speed = 2.0f;
    vec2 change = dir * state->time.tslf * speed;
    gameData->pos = gameData->pos + change;
}

extern "C"
{
    void gameAudio(GameState* state, byte* stream, int length)
    {
        gameData = (GameData*) state->memory.memory;
        uint64& runningIndex = gameData->runningSampleIndex;

        int sampleCount = length / sizeof(int16);
        int16* samples = (int16*) stream;

        double masterVolume = 0.5;
        memset(stream, 0, length);

        for (int i = 0; i < MAX_QUEUED_SOUNDS; i++)
        {
            QueuedSound* sound = &gameData->queuedSounds[i];
            if (sound->playing)
            {
                int startIndex = sound->startTick - runningIndex;
                int endIndex = startIndex + sound->sound->length;

                if (endIndex < 0) {
                    sound->playing = false;
                    continue;
                }

                if (endIndex > sampleCount) {
                    endIndex = sampleCount;
                }

                int offset = 0;
                if (startIndex < 0)
                {
                    offset = -startIndex;
                    startIndex = 0;
                }

                for (int j = startIndex; j < endIndex; j++) {
                    samples[j] += sound->sound->samples[j + offset] * masterVolume;
                }
            }
        }

        runningIndex += sampleCount;
    }

    void gameInit(GameMemory* memory) 
    {
        gameData = (GameData*) memory->memory;
        gameData->pos = vec2(0.0f);
        gameData->runningSampleIndex = 0;
    }

    bool restarted = true;
    float timer = 0.0f;
    void gameTick(GameState* state) 
    {
        gameState = state;
        gameData = (GameData*) state->memory.memory;
        if (restarted) 
        {
            SoundInfo* info = &state->soundInfo;

            ADSREnvelope env;
            env.attack = 0.2;
            env.decay = 0.2;
            env.sustain = 0.7;
            env.release = 0.1;
            env.sustainTime = 0.0;

            Oscillator osc;
            osc.waveform = Waveform::TRIANGLE;
            osc.frequency = 400;

            gameData->sample1.length = envelopeSampleCount(&env, info->sampleFreq);
            gameData->sample1.samples = gameData->sample1Data;
            gameData->sample1.sampleFreq = info->sampleFreq;

            fillSoundEnvelope(&gameData->sample1, &env, &osc);

            restarted = false;
            timer = 0.0f;
        }
        timer += state->time.tslf;
        if (timer > 1.0f) {
            playSoundSample(&gameData->sample1);

            timer -= 1.0f;
        }

        updatePlayerPos(state);

        GameInput* input = &state->input;
        if (input->keysPressed[KEY_ESCAPE]) {
            state->winState.quitTarget = true;
        }
        if (input->keysPressed[KEY_F11]) {
            state->winState.fullscreenTarget = 
                !state->winState.fullscreen;
        }

        if (input->keysPressed[KEY_X]) {
            playSoundSample(&gameData->sample1);
        }

        int playing = 0;
        for (int i = 0; i < MAX_QUEUED_SOUNDS; i++)
        {
            QueuedSound* s = &gameData->queuedSounds[i];
            if (s->playing) {
                playing++;
            }
        }
        printf("Currently playing: %d\n", playing);

        GameVideo* vid = &state->video;
        renderScene(vid->pixelData, vid->width, vid->height);
    }

    void gameShutdown(GameMemory* memory)
    {

    }
}

