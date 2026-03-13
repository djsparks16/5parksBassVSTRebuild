#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include <cmath>

struct BlacksideOscillator
{
    double sampleRate = 44100.0;
    float phase = 0.0f;
    float frequency = 110.0f;
    float wavePos = 0.0f;
    float warp = 0.0f;

    void prepare(double sr) { sampleRate = sr; phase = 0.0f; }
    void setFrequency(float hz) { frequency = hz; }
    void setWavePos(float p) { wavePos = juce::jlimit(0.0f, 1.0f, p); }
    void setWarp(float w) { warp = juce::jlimit(0.0f, 1.0f, w); }

    float process(float phaseOffset = 0.0f)
    {
        const float dt = juce::jlimit(0.0f, 0.49f, frequency / (float) sampleRate);
        phase += dt;
        if (phase >= 1.0f)
            phase -= 1.0f;

        float p = phase + phaseOffset;
        if (p >= 1.0f) p -= 1.0f;
        if (p < 0.0f) p += 1.0f;

        auto sineFn = [p]() { return std::sin(p * juce::MathConstants<float>::twoPi); };
        auto sawFn = [p]() { return 2.0f * p - 1.0f; };
        auto triFn = [p]() { return 4.0f * std::abs(p - 0.5f) - 1.0f; };
        auto squareFn = [p]() { return p < 0.5f ? 1.0f : -1.0f; };

        const float sel = juce::jlimit(0.0f, 0.999f, wavePos) * 3.0f;
        const int mode = (int) sel;
        const float frac = sel - (float) mode;

        auto wave = [&](int m)
        {
            switch (m)
            {
                case 0: return sineFn();
                case 1: return sawFn();
                case 2: return triFn();
                default: return squareFn();
            }
        };

        float out = juce::jmap(frac, wave(mode), wave(juce::jmin(3, mode + 1)));
        out = std::tanh(out * (1.0f + warp * 5.0f));
        return out;
    }
};

struct BlacksideVoice
{
    double sampleRate = 44100.0;
    bool active = false;
    int midiNote = -1;
    float velocity = 0.0f;
    float currentHz = 110.0f;
    float targetHz = 110.0f;
    float lfoPhase = 0.0f;

    BlacksideOscillator oscA;
    BlacksideOscillator oscB;
    BlacksideOscillator oscC;
    BlacksideOscillator subOsc;
    BlacksideOscillator noiseOsc;

    juce::ADSR ampEnv;
    juce::ADSR::Parameters ampParams;
    juce::ADSR modEnv;
    juce::ADSR::Parameters modParams;

    juce::dsp::StateVariableTPTFilter<float> filter1;
    juce::dsp::StateVariableTPTFilter<float> filter2;

    float last = 0.0f;

    void prepare(double sr)
    {
        sampleRate = sr;
        oscA.prepare(sr);
        oscB.prepare(sr);
        oscC.prepare(sr);
        subOsc.prepare(sr);
        noiseOsc.prepare(sr);

        juce::dsp::ProcessSpec spec { sr, 512, 1 };
        filter1.prepare(spec);
        filter2.prepare(spec);
        filter1.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        filter2.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
        reset();
    }

    void reset()
    {
        ampEnv.reset();
        modEnv.reset();
        filter1.reset();
        filter2.reset();
        last = 0.0f;
        lfoPhase = 0.0f;
    }

    void updateADSR(float a, float d, float s, float r)
    {
        ampParams.attack = a;
        ampParams.decay = d;
        ampParams.sustain = s;
        ampParams.release = r;
        modParams = ampParams;
        ampEnv.setParameters(ampParams);
        modEnv.setParameters(modParams);
    }

    void start(int note, float vel)
    {
        midiNote = note;
        velocity = vel;
        targetHz = (float) juce::MidiMessage::getMidiNoteInHertz(note);
        if (! active)
            currentHz = targetHz;
        ampEnv.noteOn();
        modEnv.noteOn();
        active = true;
    }

    void stop()
    {
        ampEnv.noteOff();
        modEnv.noteOff();
    }

    float nextSample(float oscALevel, float oscAWave, float oscAWarp, float oscASpread,
                     float oscBLevel, float oscBWave, float oscBWarp, float oscBSpread,
                     float oscCLevel, float oscCWave, float oscCWarp,
                     float subLevel, float subDrive, float noiseLevel,
                     float cutoff1, float res1, float drive1,
                     float cutoff2, float res2,
                     float distDrive, float distMix,
                     float lfoRate, float lfoAmount, float envAmount,
                     float macro1, float macro2)
    {
        if (! active)
            return 0.0f;

        currentHz += (targetHz - currentHz) * 0.0045f;

        oscA.setFrequency(currentHz);
        oscB.setFrequency(currentHz * (1.0f + oscBSpread * 0.015f));
        oscC.setFrequency(currentHz * 0.5f);
        subOsc.setFrequency(currentHz * 0.5f);
        noiseOsc.setFrequency(1000.0f + currentHz * 0.125f);

        oscA.setWavePos(oscAWave);
        oscB.setWavePos(oscBWave);
        oscC.setWavePos(oscCWave);
        subOsc.setWavePos(0.0f);

        oscA.setWarp(oscAWarp + macro1 * 0.35f);
        oscB.setWarp(oscBWarp + macro1 * 0.25f);
        oscC.setWarp(oscCWarp + macro2 * 0.25f);
        subOsc.setWarp(subDrive * 0.25f);

        lfoPhase += lfoRate / (float) sampleRate;
        if (lfoPhase >= 1.0f)
            lfoPhase -= 1.0f;

        const float lfo = std::sin(lfoPhase * juce::MathConstants<float>::twoPi) * lfoAmount;
        const float env = modEnv.getNextSample();

        const float dynamicCutoff1 = juce::jlimit(30.0f, 18000.0f,
            cutoff1 * (1.0f + env * envAmount + lfo * 0.9f + macro2 * 0.35f));
        const float dynamicCutoff2 = juce::jlimit(40.0f, 18000.0f,
            cutoff2 * (1.0f + lfo * 0.4f + macro1 * 0.2f));

        filter1.setCutoffFrequency(dynamicCutoff1);
        filter1.setResonance(juce::jlimit(0.1f, 1.4f, res1));
        filter2.setCutoffFrequency(dynamicCutoff2);
        filter2.setResonance(juce::jlimit(0.1f, 1.2f, res2));

        const float a = oscA.process(-oscASpread * 0.01f) * oscALevel;
        const float b = oscB.process( oscBSpread * 0.01f) * oscBLevel;
        const float c = oscC.process() * oscCLevel;
        const float sub = std::sin(subOsc.phase * juce::MathConstants<float>::twoPi) * subLevel;
        const float noise = noiseOsc.process() * noiseLevel * 0.2f;

        float x = a + b + c + noise + std::tanh(sub * (1.0f + subDrive * 5.0f));
        x = std::tanh(x * (1.0f + drive1 * 6.0f));
        x = filter1.processSample(0, x);
        x += filter2.processSample(0, x) * 0.35f;

        const float dirty = std::tanh(x * (1.0f + distDrive * 8.0f));
        x = juce::jmap(distMix, x, dirty);
        x = 0.85f * x + 0.15f * last;
        last = x;

        x *= ampEnv.getNextSample() * velocity;

        if (! ampEnv.isActive())
        {
            active = false;
            midiNote = -1;
        }

        return x;
    }
};
