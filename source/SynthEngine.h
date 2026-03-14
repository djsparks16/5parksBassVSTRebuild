#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <cmath>

struct StereoFrame
{
    float left = 0.0f;
    float right = 0.0f;
    float subEnergy = 0.0f;
};

struct VoiceRenderSettings
{
    float oscALevel = 0.0f;
    float oscAWave = 0.0f;
    float oscAWarp = 0.0f;
    float oscASpread = 0.0f;

    float oscBLevel = 0.0f;
    float oscBWave = 0.0f;
    float oscBWarp = 0.0f;
    float oscBSpread = 0.0f;

    float oscCLevel = 0.0f;
    float oscCWave = 0.0f;
    float oscCWarp = 0.0f;

    float subLevel = 0.0f;
    float subDrive = 0.0f;
    float subOctave = 0.0f;
    float noiseLevel = 0.0f;

    float filterCutoff = 1000.0f;
    float filterRes = 0.2f;
    float filterDrive = 0.0f;
    float filter2Cutoff = 3000.0f;
    float filter2Res = 0.2f;

    float distDrive = 0.0f;
    float distMix = 0.0f;
    float outputClip = 0.0f;

    float lfoRate = 2.0f;
    float lfoAmount = 0.0f;
    float envAmount = 0.0f;

    float macro1 = 0.0f;
    float macro2 = 0.0f;
    float macro3 = 0.0f;
    float macro4 = 0.0f;

    float reeseDetune = 0.0f;
    float reeseWidth = 0.0f;
    float reeseDrift = 0.0f;

    float hornBend = 0.0f;
    float hornFormant = 0.0f;
    float hornBody = 0.0f;
    float warhornMix = 0.0f;

    float screechFm = 0.0f;
    float screechDrive = 0.0f;

    float harmMix = 0.0f;
    float harmInterval = 0.0f;
    float harmSpread = 0.0f;
    float subHarmony = 0.0f;

    float buzzAmount = 0.0f;
    float talkAmount = 0.0f;
    float biteAmount = 0.0f;
    float punchAmount = 0.0f;
    float stereoSpin = 0.0f;
    float fmGrit = 0.0f;
    float wobbleAmount = 0.0f;
    float notchAmount = 0.0f;
    float airAmount = 0.0f;
    float wompAmount = 0.0f;
    float comboStack = 0.0f;
};

namespace blackside
{
    static inline float wrap01(float x) noexcept
    {
        while (x >= 1.0f)
            x -= 1.0f;
        while (x < 0.0f)
            x += 1.0f;
        return x;
    }

    static inline float clamp01(float x) noexcept
    {
        return juce::jlimit(0.0f, 1.0f, x);
    }
}

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

    void resetPhase(float newPhase) noexcept { phase = blackside::wrap01(newPhase); }
    void setFrequency(float hz) noexcept { frequency = juce::jmax(0.0f, hz); }
    void setWavePos(float p) noexcept { wavePos = blackside::clamp01(p); }
    void setWarp(float w) noexcept { warp = blackside::clamp01(w); }
    void setPulseWidth(float p) noexcept { pulseWidth = juce::jlimit(0.05f, 0.95f, p); }

    float process(float phaseOffset = 0.0f, float phaseMod = 0.0f) noexcept
    {
        const float dt = juce::jlimit(0.0f, 0.49f, frequency / (float) sampleRate);
        phase = blackside::wrap01(phase + dt);

        const float p = blackside::wrap01(phase + phaseOffset + phaseMod * 0.08f);
        const float sine = std::sin(p * juce::MathConstants<float>::twoPi);
        const float saw = 2.0f * p - 1.0f;
        const float tri = 1.0f - 4.0f * std::abs(p - 0.5f);
        const float square = p < pulseWidth ? 1.0f : -1.0f;
        const float folded = std::sin((p + warp * 0.21f) * juce::MathConstants<float>::twoPi * (1.0f + warp * 6.0f));
        const float vocal = std::tanh((saw * 0.58f + sine * 0.42f) * (1.2f + warp * 6.5f));
        const float pinched = std::sin(std::pow(p, 0.68f + warp * 0.25f) * juce::MathConstants<float>::twoPi);

        std::array<float, 7> waves { sine, saw, tri, square, folded, vocal, pinched };
        const float selector = juce::jlimit(0.0f, 0.999f, wavePos) * (float) (waves.size() - 1);
        const int i0 = (int) selector;
        const int i1 = juce::jmin((int) waves.size() - 1, i0 + 1);
        const float frac = selector - (float) i0;

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
    uint64_t age = 0;

    float currentHz = 110.0f;
    float targetHz = 110.0f;
    float glideMs = 45.0f;
    float lfoPhase1 = 0.0f;
    float lfoPhase2 = 0.0f;
    float hornEnvState = 0.0f;
    float biteEnvState = 0.0f;
    float punchEnvState = 0.0f;

    BlacksideOscillator oscA, oscB, oscC, subOsc, sub2Osc, noiseOsc, harmonyOsc;
    juce::ADSR ampEnv;
    juce::ADSR::Parameters ampParams;
    juce::ADSR modEnv;
    juce::ADSR::Parameters modParams;

    juce::dsp::StateVariableTPTFilter<float> filter1, filter2;
    juce::dsp::LadderFilter<float> ladder;

    juce::Random random;
    float last = 0.0f;
    float dcIn = 0.0f;
    float dcOut = 0.0f;
    float airState = 0.0f;
    float wompState = 0.0f;

    void prepare(double sr)
    {
        sampleRate = sr;
        oscA.prepare(sr); oscB.prepare(sr); oscC.prepare(sr); subOsc.prepare(sr); sub2Osc.prepare(sr); noiseOsc.prepare(sr); harmonyOsc.prepare(sr);
        ampEnv.setSampleRate(sr);
        modEnv.setSampleRate(sr);
        juce::dsp::ProcessSpec spec { sr, 512, 1 };
        filter1.prepare(spec);
        filter2.prepare(spec);
        ladder.prepare(spec);
        filter1.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        filter2.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
        ladder.setMode(juce::dsp::LadderFilterMode::LPF24);
        reset();
    }

    void reset()
    {
        ampEnv.reset(); modEnv.reset(); filter1.reset(); filter2.reset(); ladder.reset();
        last = dcIn = dcOut = airState = wompState = 0.0f;
        lfoPhase1 = lfoPhase2 = hornEnvState = biteEnvState = punchEnvState = 0.0f;
        active = false; heldBySustain = false; midiNote = -1; velocity = 0.0f; age = 0;
    }

    void setGlideMs(float ms) noexcept { glideMs = juce::jlimit(0.0f, 500.0f, ms); }

    void updateADSR(float a, float d, float s, float r)
    {
        ampParams.attack = a; ampParams.decay = d; ampParams.sustain = s; ampParams.release = r;
        modParams = ampParams;
        ampEnv.setParameters(ampParams);
        modEnv.setParameters(modParams);
    }

    void retriggerPhases(int note)
    {
        const float base = std::fmod((float) note * 0.173f, 1.0f);
        oscA.resetPhase(base);
        oscB.resetPhase(base + 0.37f);
        oscC.resetPhase(base + 0.61f);
        subOsc.resetPhase(base + 0.11f);
        sub2Osc.resetPhase(base + 0.29f);
        noiseOsc.resetPhase(base + 0.77f);
        harmonyOsc.resetPhase(base + 0.47f);
    }

    void start(int note, float vel, bool retriggerEnvelope)
    {
        midiNote = note;
        velocity = vel;
        targetHz = (float) juce::MidiMessage::getMidiNoteInHertz(note);
        if (! active)
            currentHz = targetHz;
        if (retriggerEnvelope || ! active)
        {
            retriggerPhases(note);
            ampEnv.noteOn();
            modEnv.noteOn();
            hornEnvState = biteEnvState = punchEnvState = 1.0f;
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

    StereoFrame render(const VoiceRenderSettings& s)
    {
        StereoFrame frame;
        if (! active)
            return frame;

        const float glideCoeff = (glideMs <= 0.0f) ? 1.0f : (1.0f - std::exp(-1.0f / (0.001f * glideMs * (float) sampleRate)));
        currentHz += (targetHz - currentHz) * glideCoeff;

        lfoPhase1 = blackside::wrap01(lfoPhase1 + s.lfoRate / (float) sampleRate);
        lfoPhase2 = blackside::wrap01(lfoPhase2 + (s.lfoRate * (0.21f + s.wobbleAmount * 0.37f) + 0.17f) / (float) sampleRate);

        const float lfo1 = std::sin(lfoPhase1 * juce::MathConstants<float>::twoPi) * s.lfoAmount;
        const float lfo2 = std::sin(lfoPhase2 * juce::MathConstants<float>::twoPi) * (s.lfoAmount * (0.4f + s.wobbleAmount * 0.5f));
        const float env = modEnv.getNextSample();
        const float amp = ampEnv.getNextSample();

        hornEnvState *= (0.9963f - s.hornBend * 0.0010f);
        biteEnvState *= (0.9925f - s.biteAmount * 0.0022f);
        punchEnvState *= (0.9890f - s.punchAmount * 0.0030f);

        const float hornPitchMod = hornEnvState * s.hornBend * 0.68f;
        const float bitePitchMod = biteEnvState * s.biteAmount * 0.42f;
        const float punchPitchMod = punchEnvState * s.punchAmount * 0.16f;

        const float driftA = std::sin((lfoPhase2 + 0.13f) * juce::MathConstants<float>::twoPi) * s.reeseDrift * 0.012f;
        const float driftB = std::sin((lfoPhase2 + 0.41f) * juce::MathConstants<float>::twoPi) * s.reeseDrift * 0.012f;

        const float pitchModA = 1.0f - s.reeseDetune * 0.024f + driftA + hornPitchMod + bitePitchMod + punchPitchMod;
        const float pitchModB = 1.0f + s.reeseDetune * 0.024f + driftB + hornPitchMod * 0.82f - bitePitchMod * 0.15f + punchPitchMod * 0.4f;
        const float oscCScale = juce::jmax(0.2f, 0.50f + s.macro2 * 0.32f + s.hornBody * 0.12f + s.warhornMix * 0.10f);

        oscA.setFrequency(currentHz * pitchModA);
        oscB.setFrequency(currentHz * pitchModB);
        oscC.setFrequency(currentHz * oscCScale);
        subOsc.setFrequency(currentHz * 0.5f);
        sub2Osc.setFrequency(currentHz * 0.25f);
        noiseOsc.setFrequency(1600.0f + currentHz * 0.18f + s.screechFm * 3600.0f + s.buzzAmount * 700.0f);

        const float intervalRatio = std::pow(2.0f, s.harmInterval / 12.0f);
        harmonyOsc.setFrequency(currentHz * intervalRatio);

        oscA.setWavePos(blackside::clamp01(s.oscAWave + s.macro1 * 0.06f + s.comboStack * 0.04f));
        oscB.setWavePos(blackside::clamp01(s.oscBWave + s.macro1 * 0.05f + s.comboStack * 0.03f));
        oscC.setWavePos(blackside::clamp01(s.oscCWave + s.hornFormant * 0.18f + s.macro2 * 0.08f + s.warhornMix * 0.08f));
        harmonyOsc.setWavePos(blackside::clamp01(s.oscBWave + 0.18f + s.macro3 * 0.08f + s.subHarmony * 0.05f));
        subOsc.setWavePos(0.03f + s.subDrive * 0.10f);
        sub2Osc.setWavePos(0.00f);
        noiseOsc.setWavePos(0.70f + s.screechDrive * 0.20f);

        oscA.setWarp(blackside::clamp01(s.oscAWarp + s.macro1 * 0.20f + s.screechFm * 0.12f + s.buzzAmount * 0.15f + s.comboStack * 0.12f));
        oscB.setWarp(blackside::clamp01(s.oscBWarp + s.macro1 * 0.15f + s.screechFm * 0.10f + s.buzzAmount * 0.10f + s.comboStack * 0.10f));
        oscC.setWarp(blackside::clamp01(s.oscCWarp + s.macro2 * 0.16f + s.hornBody * 0.18f + s.fmGrit * 0.10f + s.warhornMix * 0.18f));
        harmonyOsc.setWarp(blackside::clamp01(0.12f + s.harmMix * 0.30f + s.macro3 * 0.10f));
        subOsc.setWarp(blackside::clamp01(s.subDrive * 0.32f));
        sub2Osc.setWarp(blackside::clamp01(s.subDrive * 0.18f));
        noiseOsc.setWarp(blackside::clamp01(s.screechDrive * 0.72f + s.talkAmount * 0.10f + s.wompAmount * 0.08f));

        oscA.setPulseWidth(0.50f - s.biteAmount * 0.12f + s.macro4 * 0.05f);
        oscB.setPulseWidth(0.50f + s.hornBody * 0.08f);

        const float cRaw = oscC.process(lfo2 * 0.02f, hornPitchMod * 0.25f);
        const float fmPhase = cRaw * (0.04f + s.fmGrit * 0.36f + s.screechFm * 0.22f + s.buzzAmount * 0.08f + s.wompAmount * 0.12f);

        const float stereoOffset = (s.reeseWidth * 0.60f + s.stereoSpin * 0.35f + s.macro3 * 0.12f + s.comboStack * 0.10f) * 0.018f;
        const float a = oscA.process(-(s.oscASpread * 0.4f) * 0.02f - stereoOffset, fmPhase + bitePitchMod) * s.oscALevel;
        const float b = oscB.process((s.oscBSpread * 0.4f) * 0.02f + stereoOffset, -fmPhase + hornPitchMod * 0.3f) * s.oscBLevel;
        const float c = cRaw * s.oscCLevel * (0.55f + s.hornBody * 0.28f + s.macro2 * 0.20f + s.warhornMix * 0.12f);
        const float h = harmonyOsc.process(s.harmSpread * 0.02f + lfo2 * 0.01f, fmPhase * 0.4f) * s.harmMix * (0.45f + s.macro3 * 0.18f);

        const float subMain = subOsc.process();
        const float subLow = sub2Osc.process();
        const float subHarm = std::sin(subOsc.phase * juce::MathConstants<float>::twoPi * (1.5f + s.subHarmony * 0.5f));
        const float sub = std::tanh((subMain * s.subLevel + subLow * s.subOctave * 0.55f + subHarm * s.subHarmony * 0.35f) * (1.0f + s.subDrive * 6.0f));

        const float white = random.nextFloat() * 2.0f - 1.0f;
        const float pitchedNoise = noiseOsc.process(lfo2 * 0.03f, fmPhase * 0.20f);
        const float noise = (white * 0.72f + pitchedNoise * 0.28f) * s.noiseLevel * (0.15f + s.screechDrive * 0.30f + s.buzzAmount * 0.12f);

        const float body = a + b + c;
        const float reeseLayer = (a - b) * (0.22f + s.reeseWidth * 0.35f + s.comboStack * 0.15f);
        const float buzz = (std::tanh(body * (1.0f + s.buzzAmount * 7.0f)) - std::tanh(body * 0.9f)) * (0.18f + s.buzzAmount * 0.32f);
        const float warhorn = std::tanh((cRaw * (1.5f + s.hornBody * 2.5f + s.warhornMix * 3.0f) + a * 0.2f) * (1.0f + hornEnvState * 0.8f));
        const float hornLayer = warhorn * (0.12f + s.warhornMix * 0.60f + s.hornFormant * 0.18f);
        const float wompSweep = 0.5f + 0.5f * std::sin((lfoPhase1 + 0.17f) * juce::MathConstants<float>::twoPi);
        const float wompDrive = 1.0f + s.wompAmount * (2.8f + 2.0f * wompSweep);
        float womp = std::sin((body + cRaw * 0.6f) * (2.5f + s.wompAmount * 5.0f + s.macro4 * 1.2f));
        womp = std::tanh(womp * wompDrive) * (0.10f + s.wompAmount * 0.38f);

        float x = body + h + noise + buzz + hornLayer + womp + reeseLayer * 0.25f;
        x += sub;
        x *= 1.0f + punchEnvState * s.punchAmount * 0.30f;

        const float cutoffMod = 1.0f + env * s.envAmount + lfo1 * (0.78f + s.wobbleAmount * 0.42f + s.wompAmount * 0.20f) + s.macro4 * 0.22f + s.biteAmount * 0.12f;
        const float formantMod = 1.0f + s.hornFormant * 0.50f + s.talkAmount * 0.60f + s.macro3 * 0.16f + s.warhornMix * 0.25f + lfo2 * 0.40f;

        const float dynamicCutoff1 = juce::jlimit(30.0f, 18000.0f, s.filterCutoff * cutoffMod);
        const float dynamicCutoff2 = juce::jlimit(30.0f, 18000.0f, s.filter2Cutoff * formantMod * (0.8f + s.wompAmount * 0.4f));

        filter1.setCutoffFrequency(dynamicCutoff1);
        filter1.setResonance(juce::jlimit(0.1f, 1.35f, s.filterRes + s.reeseWidth * 0.12f + s.talkAmount * 0.16f + s.wompAmount * 0.08f));
        filter2.setCutoffFrequency(dynamicCutoff2);
        filter2.setResonance(juce::jlimit(0.1f, 1.30f, s.filter2Res + s.hornBody * 0.15f + s.talkAmount * 0.18f + s.warhornMix * 0.12f));
        ladder.setCutoffFrequencyHz(juce::jlimit(40.0f, 16000.0f, dynamicCutoff1 * (0.72f + s.hornBody * 0.36f + s.macro4 * 0.20f)));
        ladder.setResonance(juce::jlimit(0.1f, 0.95f, 0.18f + s.screechFm * 0.28f + s.buzzAmount * 0.12f));

        x = std::tanh(x * (1.0f + s.filterDrive * 6.0f + s.screechDrive * 2.4f + s.biteAmount * 1.8f + s.punchAmount * 1.2f + s.comboStack * 1.2f));
        const float main = filter1.processSample(0, x);
        const float band = filter2.processSample(0, x);
        const float laddered = std::tanh(main * (1.0f + s.hornBody * 1.8f + s.screechFm * 0.8f + s.buzzAmount * 0.4f + s.warhornMix * 0.6f));

        const float talkLayer = band * (0.14f + s.talkAmount * 0.56f + s.hornFormant * 0.24f);
        const float notchLayer = band * (s.notchAmount * 0.55f + s.macro3 * 0.08f);
        const float wompBand = std::sin(band * (3.0f + s.wompAmount * 7.0f)) * (0.04f + s.wompAmount * 0.18f);

        x = juce::jmap(0.22f + s.hornBody * 0.26f + s.warhornMix * 0.12f, main, laddered);
        x += talkLayer;
        x -= notchLayer;
        x += wompBand;

        float dirty = std::tanh(x * (1.0f + s.distDrive * 8.0f + s.screechDrive * 5.0f + s.macro4 * 2.0f + s.comboStack * 1.8f));
        dirty = std::tanh(dirty + buzz * (0.08f + s.buzzAmount * 0.20f) + hornLayer * 0.18f + womp * 0.12f);
        x = juce::jmap(s.distMix, x, dirty);

        airState += 0.05f * (x - airState);
        const float air = (x - airState) * (s.airAmount * 0.45f + s.biteAmount * 0.14f);
        x += air;

        wompState += 0.08f * (x - wompState);
        x += (x - wompState) * (s.comboStack * 0.08f);

        x = 0.84f * x + 0.16f * last;
        last = x;

        const float hp = x - dcIn + 0.995f * dcOut;
        dcIn = x;
        dcOut = hp;
        x = hp;

        const float ampShaped = amp * velocity * (1.0f + punchEnvState * s.punchAmount * 0.28f);
        x *= ampShaped;
        x = std::tanh(x * (1.0f + s.outputClip * 3.6f + s.macro4 * 1.1f));

        const float sideSource = (a - b) + talkLayer * 0.55f + noise * 0.18f + h * 0.22f + hornLayer * 0.18f + womp * 0.22f;
        const float sideAmount = juce::jlimit(0.0f, 1.3f, s.reeseWidth * 0.95f + s.stereoSpin * 0.72f + s.wobbleAmount * 0.22f + s.comboStack * 0.15f);
        const float sideMod = 1.0f + std::sin((lfoPhase2 + 0.25f) * juce::MathConstants<float>::twoPi) * s.stereoSpin * 0.35f;
        const float side = std::tanh(sideSource * (0.5f + sideAmount * 2.0f)) * 0.38f * sideMod;

        frame.left = x - side;
        frame.right = x + side;
        frame.subEnergy = std::abs(sub) * ampShaped;

        if (! ampEnv.isActive())
        {
            active = false;
            heldBySustain = false;
            midiNote = -1;
            velocity = 0.0f;
        }

        return frame;
    }
};
