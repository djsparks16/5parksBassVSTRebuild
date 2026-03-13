#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include <array>
#include <cmath>

struct BlacksideOscillator
{
    double sampleRate = 44100.0;
    float phase = 0.0f;
    float frequency = 110.0f;
    float wavePos = 0.0f;
    float warp = 0.0f;
    float pulseWidth = 0.5f;

    void prepare(double sr)
    {
        sampleRate = sr;
        phase = 0.0f;
    }

    void setFrequency(float hz) { frequency = hz; }
    void setWavePos(float p)    { wavePos = juce::jlimit(0.0f, 1.0f, p); }
    void setWarp(float w)       { warp = juce::jlimit(0.0f, 1.0f, w); }
    void setPulseWidth(float p) { pulseWidth = juce::jlimit(0.05f, 0.95f, p); }

    float process(float phaseOffset = 0.0f)
    {
        const float dt = juce::jlimit(0.0f, 0.49f, frequency / (float) sampleRate);
        phase += dt;
        if (phase >= 1.0f)
            phase -= 1.0f;

        float p = phase + phaseOffset;
        if (p >= 1.0f) p -= 1.0f;
        if (p < 0.0f)  p += 1.0f;

        const float sine   = std::sin(p * juce::MathConstants<float>::twoPi);
        const float saw    = 2.0f * p - 1.0f;
        const float tri    = 4.0f * std::abs(p - 0.5f) - 1.0f;
        const float square = p < pulseWidth ? 1.0f : -1.0f;
        const float metal  = std::sin((p + warp * 0.2f) * juce::MathConstants<float>::twoPi * (2.0f + warp * 7.0f));
        const float vowel  = std::tanh((saw * 0.55f + sine * 0.45f) * (1.5f + warp * 6.0f));

        std::array<float, 6> waves { sine, saw, tri, square, metal, vowel };
        const float sel = juce::jlimit(0.0f, 0.999f, wavePos) * (float) (waves.size() - 1);
        const int i0 = (int) sel;
        const int i1 = juce::jmin((int) waves.size() - 1, i0 + 1);
        const float frac = sel - (float) i0;

        float out = juce::jmap(frac, waves[(size_t) i0], waves[(size_t) i1]);
        out = std::tanh(out * (1.0f + warp * 4.0f));
        return out;
    }
};

struct BlacksideVoice
{
    double sampleRate = 44100.0;
    bool active = false;
    bool heldBySustain = false;
    int midiNote = -1;
    float velocity = 0.0f;

    float currentHz = 110.0f;
    float targetHz  = 110.0f;
    float glideMs   = 45.0f;
    float lfoPhase1 = 0.0f;
    float lfoPhase2 = 0.0f;
    float hornEnvState = 0.0f;

    BlacksideOscillator oscA;
    BlacksideOscillator oscB;
    BlacksideOscillator oscC;
    BlacksideOscillator subOsc;
    BlacksideOscillator noiseOsc;
    BlacksideOscillator harmonyOsc;

    juce::ADSR ampEnv;
    juce::ADSR::Parameters ampParams;
    juce::ADSR modEnv;
    juce::ADSR::Parameters modParams;

    juce::dsp::StateVariableTPTFilter<float> filter1;
    juce::dsp::StateVariableTPTFilter<float> filter2;
    juce::dsp::StateVariableTPTFilter<float> ladder;

    float last = 0.0f;
    float dc = 0.0f;

    void prepare(double sr)
    {
        sampleRate = sr;
        oscA.prepare(sr);
        oscB.prepare(sr);
        oscC.prepare(sr);
        subOsc.prepare(sr);
        noiseOsc.prepare(sr);
        harmonyOsc.prepare(sr);

        juce::dsp::ProcessSpec spec { sr, 512, 1 };
        filter1.prepare(spec);
        filter2.prepare(spec);
        ladder.prepare(spec);
        filter1.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        filter2.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
        ladder.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        reset();
    }

    void reset()
    {
        ampEnv.reset();
        modEnv.reset();
        filter1.reset();
        filter2.reset();
        ladder.reset();
        last = 0.0f;
        dc = 0.0f;
        lfoPhase1 = 0.0f;
        lfoPhase2 = 0.0f;
        hornEnvState = 0.0f;
        active = false;
        heldBySustain = false;
        midiNote = -1;
    }

    void setGlideMs(float ms) { glideMs = juce::jlimit(0.0f, 500.0f, ms); }

    void updateADSR(float a, float d, float s, float r)
    {
        ampParams.attack  = a;
        ampParams.decay   = d;
        ampParams.sustain = s;
        ampParams.release = r;
        modParams = ampParams;
        ampEnv.setParameters(ampParams);
        modEnv.setParameters(modParams);
    }

    void start(int note, float vel, bool retriggerEnvelope)
    {
        midiNote  = note;
        velocity  = vel;
        targetHz  = (float) juce::MidiMessage::getMidiNoteInHertz(note);

        if (! active)
            currentHz = targetHz;

        if (retriggerEnvelope || ! active)
        {
            ampEnv.noteOn();
            modEnv.noteOn();
            hornEnvState = 1.0f;
        }

        active = true;
        heldBySustain = false;
    }

    void legatoTo(int note, float vel)
    {
        midiNote = note;
        velocity = vel;
        targetHz = (float) juce::MidiMessage::getMidiNoteInHertz(note);
        active = true;
    }

    void stop()
    {
        ampEnv.noteOff();
        modEnv.noteOff();
        heldBySustain = false;
    }

    float nextSample(
        float oscALevel, float oscAWave, float oscAWarp, float oscASpread,
        float oscBLevel, float oscBWave, float oscBWarp, float oscBSpread,
        float oscCLevel, float oscCWave, float oscCWarp,
        float subLevel, float subDrive, float noiseLevel,
        float cutoff1, float res1, float drive1,
        float cutoff2, float res2,
        float distDrive, float distMix,
        float lfoRate, float lfoAmount, float envAmount,
        float macro1, float macro2,
        float reeseDetune, float reeseWidth, float reeseDrift,
        float hornBend, float hornFormant, float hornBody,
        float screechFm, float screechDrive,
        float harmMix, float harmInterval, float harmSpread,
        float subOctaveMix, float outputClip)
    {
        if (! active)
            return 0.0f;

        const float glideCoeff = (glideMs <= 0.0f)
            ? 1.0f
            : (1.0f - std::exp(-1.0f / (0.001f * glideMs * (float) sampleRate)));
        currentHz += (targetHz - currentHz) * glideCoeff;

        lfoPhase1 += lfoRate / (float) sampleRate;
        lfoPhase2 += (lfoRate * 0.31f + 0.17f) / (float) sampleRate;
        if (lfoPhase1 >= 1.0f) lfoPhase1 -= 1.0f;
        if (lfoPhase2 >= 1.0f) lfoPhase2 -= 1.0f;

        const float lfo1 = std::sin(lfoPhase1 * juce::MathConstants<float>::twoPi) * lfoAmount;
        const float lfo2 = std::sin(lfoPhase2 * juce::MathConstants<float>::twoPi) * (lfoAmount * 0.5f);
        const float env = modEnv.getNextSample();

        hornEnvState *= 0.9994f;
        const float hornPitchMod = hornEnvState * hornBend * 0.7f;

        const float driftA = std::sin((lfoPhase2 + 0.13f) * juce::MathConstants<float>::twoPi) * reeseDrift * 0.01f;
        const float driftB = std::sin((lfoPhase2 + 0.41f) * juce::MathConstants<float>::twoPi) * reeseDrift * 0.01f;

        oscA.setFrequency(currentHz * (1.0f - reeseDetune * 0.02f + driftA + hornPitchMod));
        oscB.setFrequency(currentHz * (1.0f + reeseDetune * 0.02f + driftB + hornPitchMod));
        oscC.setFrequency(currentHz * (0.5f + macro2 * 0.35f));
        subOsc.setFrequency(currentHz * 0.5f);
        noiseOsc.setFrequency(1800.0f + currentHz * 0.2f + screechFm * 3000.0f);

        const float intervalRatio = std::pow(2.0f, harmInterval / 12.0f);
        harmonyOsc.setFrequency(currentHz * intervalRatio);

        oscA.setWavePos(oscAWave);
        oscB.setWavePos(oscBWave);
        oscC.setWavePos(juce::jlimit(0.0f, 1.0f, oscCWave + hornFormant * 0.2f));
        harmonyOsc.setWavePos(juce::jlimit(0.0f, 1.0f, oscBWave + 0.2f));
        subOsc.setWavePos(0.15f);
        noiseOsc.setWavePos(0.8f);

        oscA.setWarp(oscAWarp + macro1 * 0.22f + screechFm * 0.15f);
        oscB.setWarp(oscBWarp + macro1 * 0.16f + screechFm * 0.12f);
        oscC.setWarp(oscCWarp + macro2 * 0.14f + hornBody * 0.18f);
        harmonyOsc.setWarp(0.12f + harmMix * 0.25f);
        subOsc.setWarp(subDrive * 0.3f);
        noiseOsc.setWarp(screechDrive * 0.7f);

        const float a = oscA.process(-(oscASpread + reeseWidth * 0.5f) * 0.02f) * oscALevel;
        const float b = oscB.process( (oscBSpread + reeseWidth * 0.5f) * 0.02f) * oscBLevel;
        const float c = oscC.process() * oscCLevel;
        const float h = harmonyOsc.process(harmSpread * 0.02f) * harmMix * 0.55f;
        const float sub = std::sin(subOsc.phase * juce::MathConstants<float>::twoPi) * subLevel;
        const float sub2 = std::sin((subOsc.phase * 0.5f) * juce::MathConstants<float>::twoPi) * subOctaveMix * 0.5f;
        const float noise = noiseOsc.process() * noiseLevel * (0.12f + screechDrive * 0.25f);

        float x = a + b + c + h + noise;
        x += std::tanh((sub + sub2) * (1.0f + subDrive * 5.0f));

        const float dynamicCutoff1 = juce::jlimit(30.0f, 18000.0f,
            cutoff1 * (1.0f + env * envAmount + lfo1 * 0.8f + macro2 * 0.25f));
        const float dynamicCutoff2 = juce::jlimit(30.0f, 18000.0f,
            cutoff2 * (1.0f + lfo2 * 0.6f + hornFormant * 0.5f + macro1 * 0.2f));

        filter1.setCutoffFrequency(dynamicCutoff1);
        filter1.setResonance(juce::jlimit(0.1f, 1.4f, res1 + reeseWidth * 0.15f));
        filter2.setCutoffFrequency(dynamicCutoff2);
        filter2.setResonance(juce::jlimit(0.1f, 1.2f, res2 + hornBody * 0.12f));
        ladder.setCutoffFrequency(juce::jlimit(40.0f, 16000.0f, dynamicCutoff1 * (0.7f + hornBody * 0.4f)));
        ladder.setResonance(juce::jlimit(0.1f, 0.95f, 0.2f + screechFm * 0.35f));

        x = std::tanh(x * (1.0f + drive1 * 5.5f + screechDrive * 2.5f));
        x = filter1.processSample(0, x);
        x += filter2.processSample(0, x) * (0.28f + hornFormant * 0.24f);
        x = 0.72f * x + 0.28f * ladder.processSample(0, x);

        const float dirty = std::tanh(x * (1.0f + distDrive * 7.0f + screechDrive * 5.0f));
        x = juce::jmap(distMix, x, dirty);

        x = 0.86f * x + 0.14f * last;
        last = x;

        const float hp = x - dc + 0.995f * dc;
        dc = x;
        x = hp;

        x *= ampEnv.getNextSample() * velocity;
        x = std::tanh(x * (1.0f + outputClip * 3.0f));

        if (! ampEnv.isActive())
        {
            active = false;
            heldBySustain = false;
            midiNote = -1;
        }

        return x;
    }
};
