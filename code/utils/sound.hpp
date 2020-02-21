#ifndef __SOUND_HPP__
#define __SOUND_HPP__

// Includes
#include "datatypes.hpp"

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




#endif
