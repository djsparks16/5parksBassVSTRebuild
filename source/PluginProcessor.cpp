#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <algorithm>
#include <limits>

BadlineDnBAudioProcessor::BadlineDnBAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createParameters())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout BadlineDnBAudioProcessor::createParameters()
{
    using APF = juce::AudioParameterFloat;
    using APC = juce::AudioParameterChoice;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto norm = [](const juce::String& id,
                   const juce::String& name,
                   float min,
                   float max,
                   float def,
                   float skew = 1.0f)
    {
        return std::make_unique<APF>(
            id,
            name,
            juce::NormalisableRange<float>(min, max, 0.0f, skew),
            def);
    };

    params.push_back(std::make_unique<APC>("play_mode", "Play Mode", juce::StringArray { "Mono", "Legato", "Poly" }, 0));
    params.push_back(std::make_unique<APC>("note_priority", "Note Priority", juce::StringArray { "Last", "Low", "High" }, 0));
    params.push_back(norm("glide_ms", "Glide", 0.0f, 500.0f, 36.0f, 0.35f));

    params.push_back(norm("oscA_level", "OscA Level", 0.0f, 1.0f, 0.86f));
    params.push_back(norm("oscA_wave", "OscA Wave", 0.0f, 1.0f, 0.24f));
    params.push_back(norm("oscA_warp", "OscA Warp", 0.0f, 1.0f, 0.20f));
    params.push_back(norm("oscA_spread", "OscA Spread", 0.0f, 1.0f, 0.25f));

    params.push_back(norm("oscB_level", "OscB Level", 0.0f, 1.0f, 0.74f));
    params.push_back(norm("oscB_wave", "OscB Wave", 0.0f, 1.0f, 0.61f));
    params.push_back(norm("oscB_warp", "OscB Warp", 0.0f, 1.0f, 0.16f));
    params.push_back(norm("oscB_spread", "OscB Spread", 0.0f, 1.0f, 0.28f));

    params.push_back(norm("oscC_level", "OscC Level", 0.0f, 1.0f, 0.48f));
    params.push_back(norm("oscC_wave", "OscC Wave", 0.0f, 1.0f, 0.14f));
    params.push_back(norm("oscC_warp", "OscC Warp", 0.0f, 1.0f, 0.12f));

    params.push_back(norm("sub_level", "Sub Level", 0.0f, 1.0f, 0.78f));
    params.push_back(norm("sub_drive", "Sub Drive", 0.0f, 1.0f, 0.10f));
    params.push_back(norm("sub_octave", "Sub Octave", 0.0f, 1.0f, 0.28f));
    params.push_back(norm("noise_level", "Noise Level", 0.0f, 1.0f, 0.04f));

    params.push_back(norm("filter_cutoff", "Filter 1 Cutoff", 20.0f, 18000.0f, 1350.0f, 0.30f));
    params.push_back(norm("filter_res", "Filter 1 Res", 0.1f, 1.25f, 0.26f));
    params.push_back(norm("filter_drive", "Filter 1 Drive", 0.0f, 1.0f, 0.20f));

    params.push_back(norm("filter2_cutoff", "Filter 2 Cutoff", 20.0f, 18000.0f, 4200.0f, 0.30f));
    params.push_back(norm("filter2_res", "Filter 2 Res", 0.1f, 1.25f, 0.24f));

    params.push_back(norm("dist_drive", "Distortion Drive", 0.0f, 1.0f, 0.33f));
    params.push_back(norm("dist_mix", "Distortion Mix", 0.0f, 1.0f, 0.62f));
    params.push_back(norm("output_clip", "Output Clip", 0.0f, 1.0f, 0.22f));

    params.push_back(norm("lfo1_rate", "LFO 1 Rate", 0.01f, 20.0f, 3.1f, 0.35f));
    params.push_back(norm("lfo1_amt", "LFO 1 Amount", 0.0f, 1.0f, 0.25f));
    params.push_back(norm("envamt", "Env Amount", 0.0f, 2.0f, 0.78f));
    params.push_back(norm("envA", "Attack", 0.001f, 2.0f, 0.004f, 0.40f));
    params.push_back(norm("envD", "Decay", 0.001f, 2.0f, 0.20f, 0.40f));
    params.push_back(norm("envS", "Sustain", 0.0f, 1.0f, 0.72f));
    params.push_back(norm("envR", "Release", 0.001f, 3.0f, 0.16f, 0.40f));

    params.push_back(norm("reese_detune", "Reese Detune", 0.0f, 1.0f, 0.32f));
    params.push_back(norm("reese_width", "Reese Width", 0.0f, 1.0f, 0.42f));
    params.push_back(norm("reese_drift", "Reese Drift", 0.0f, 1.0f, 0.18f));

    params.push_back(norm("horn_bend", "Horn Bend", 0.0f, 1.0f, 0.18f));
    params.push_back(norm("horn_formant", "Horn Formant", 0.0f, 1.0f, 0.22f));
    params.push_back(norm("horn_body", "Horn Body", 0.0f, 1.0f, 0.28f));
    params.push_back(norm("warhorn_mix", "Warhorn Mix", 0.0f, 1.0f, 0.18f));

    params.push_back(norm("screech_fm", "Screech FM", 0.0f, 1.0f, 0.14f));
    params.push_back(norm("screech_drive", "Screech Drive", 0.0f, 1.0f, 0.18f));

    params.push_back(norm("harm_mix", "Harmony Mix", 0.0f, 1.0f, 0.0f));
    params.push_back(norm("harm_interval", "Harmony Interval", -24.0f, 24.0f, 7.0f));
    params.push_back(norm("harm_spread", "Harmony Spread", 0.0f, 1.0f, 0.20f));
    params.push_back(norm("sub_harmony", "Sub Harmony", 0.0f, 1.0f, 0.0f));

    params.push_back(norm("buzz_amt", "Buzz", 0.0f, 1.0f, 0.16f));
    params.push_back(norm("talk_amt", "Talk", 0.0f, 1.0f, 0.18f));
    params.push_back(norm("bite_amt", "Bite", 0.0f, 1.0f, 0.12f));
    params.push_back(norm("punch_amt", "Punch", 0.0f, 1.0f, 0.20f));
    params.push_back(norm("stereo_spin", "Stereo Spin", 0.0f, 1.0f, 0.24f));
    params.push_back(norm("fm_grit", "FM Grit", 0.0f, 1.0f, 0.14f));
    params.push_back(norm("wobble_amt", "Wobble", 0.0f, 1.0f, 0.18f));
    params.push_back(norm("notch_amt", "Notch", 0.0f, 1.0f, 0.12f));
    params.push_back(norm("air_amt", "Air", 0.0f, 1.0f, 0.08f));
    params.push_back(norm("womp_amt", "Womp", 0.0f, 1.0f, 0.18f));
    params.push_back(norm("combo_stack", "Combo Stack", 0.0f, 1.0f, 0.24f));
    params.push_back(norm("mono_blend", "Mono Blend", 0.0f, 1.0f, 0.18f));

    for (int i = 1; i <= 8; ++i)
        params.push_back(norm("macro" + juce::String(i), "Macro " + juce::String(i), 0.0f, 1.0f, 0.0f));

    params.push_back(norm("master_gain", "Output", 0.0f, 1.0f, 0.84f));

    return { params.begin(), params.end() };
}

void BadlineDnBAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);

    for (auto& voice : voices)
        voice.prepare(sampleRate);

    keyDown.fill(false);
    sustained.fill(false);
    noteVelocity.fill(0.0f);
    monoStack.clear();
    sustainPedalDown = false;
    voiceCounter = 1;

    meterLevel.store(0.0f);
    subMeterLevel.store(0.0f);
    scopeWritePos.store(0);
    fftFifoIndex = 0;
    analyzerBins.fill(0.0f);
}

void BadlineDnBAudioProcessor::releaseResources() {}

bool BadlineDnBAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void BadlineDnBAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    const auto choice = [this](const char* id)
    {
        return juce::roundToInt(apvts.getRawParameterValue(id)->load());
    };

    const PlayMode playMode = static_cast<PlayMode>(choice("play_mode"));

    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();

        if (msg.isController() && msg.getControllerNumber() == 64)
        {
            handleSustainPedal(msg.getControllerValue() >= 64, playMode);
            continue;
        }

        if (msg.isNoteOn())
            handleNoteOn(msg.getNoteNumber(), msg.getFloatVelocity(), playMode);
        else if (msg.isNoteOff())
            handleNoteOff(msg.getNoteNumber(), playMode);

        if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            for (auto& voice : voices)
                voice.stop();

            keyDown.fill(false);
            sustained.fill(false);
            noteVelocity.fill(0.0f);
            monoStack.clear();
            sustainPedalDown = false;
        }
    }

    const auto param = [this](const char* id)
    {
        return apvts.getRawParameterValue(id)->load();
    };

    const float glideMs = param("glide_ms");
    const float envA = param("envA");
    const float envD = param("envD");
    const float envS = param("envS");
    const float envR = param("envR");
    const float monoBlend = param("mono_blend");
    const float master = param("master_gain");

    VoiceRenderSettings settings;
    settings.oscALevel = param("oscA_level");
    settings.oscAWave = param("oscA_wave");
    settings.oscAWarp = param("oscA_warp");
    settings.oscASpread = param("oscA_spread");

    settings.oscBLevel = param("oscB_level");
    settings.oscBWave = param("oscB_wave");
    settings.oscBWarp = param("oscB_warp");
    settings.oscBSpread = param("oscB_spread");

    settings.oscCLevel = param("oscC_level");
    settings.oscCWave = param("oscC_wave");
    settings.oscCWarp = param("oscC_warp");

    settings.subLevel = param("sub_level");
    settings.subDrive = param("sub_drive");
    settings.subOctave = param("sub_octave");
    settings.noiseLevel = param("noise_level");

    settings.filterCutoff = param("filter_cutoff");
    settings.filterRes = param("filter_res");
    settings.filterDrive = param("filter_drive");
    settings.filter2Cutoff = param("filter2_cutoff");
    settings.filter2Res = param("filter2_res");

    settings.distDrive = param("dist_drive");
    settings.distMix = param("dist_mix");
    settings.outputClip = param("output_clip");

    settings.lfoRate = param("lfo1_rate");
    settings.lfoAmount = param("lfo1_amt");
    settings.envAmount = param("envamt");

    settings.macro1 = param("macro1");
    settings.macro2 = param("macro2");
    settings.macro3 = param("macro3");
    settings.macro4 = param("macro4");

    settings.reeseDetune = param("reese_detune");
    settings.reeseWidth = param("reese_width");
    settings.reeseDrift = param("reese_drift");

    settings.hornBend = param("horn_bend");
    settings.hornFormant = param("horn_formant");
    settings.hornBody = param("horn_body");
    settings.warhornMix = param("warhorn_mix");

    settings.screechFm = param("screech_fm");
    settings.screechDrive = param("screech_drive");

    settings.harmMix = param("harm_mix");
    settings.harmInterval = param("harm_interval");
    settings.harmSpread = param("harm_spread");
    settings.subHarmony = param("sub_harmony");

    settings.buzzAmount = param("buzz_amt");
    settings.talkAmount = param("talk_amt");
    settings.biteAmount = param("bite_amt");
    settings.punchAmount = param("punch_amt");
    settings.stereoSpin = param("stereo_spin");
    settings.fmGrit = param("fm_grit");
    settings.wobbleAmount = param("wobble_amt");
    settings.notchAmount = param("notch_amt");
    settings.airAmount = param("air_amt");
    settings.wompAmount = param("womp_amt");
    settings.comboStack = param("combo_stack");

    for (auto& voice : voices)
    {
        voice.setGlideMs(glideMs);
        voice.updateADSR(envA, envD, envS, envR);
    }

    float peak = 0.0f;
    float subPeak = 0.0f;

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float left = 0.0f;
        float right = 0.0f;
        float sub = 0.0f;

        if (playMode == PlayMode::poly)
        {
            for (auto& voice : voices)
            {
                const auto frame = voice.render(settings);
                left += frame.left;
                right += frame.right;
                sub += frame.subEnergy;
            }

            left *= 0.22f;
            right *= 0.22f;
            sub *= 0.22f;
        }
        else
        {
            const auto frame = voices[0].render(settings);
            left = frame.left;
            right = frame.right;
            sub = frame.subEnergy;
        }

        const float mono = 0.5f * (left + right);
        left = juce::jmap(monoBlend, left, mono);
        right = juce::jmap(monoBlend, right, mono);

        left *= master;
        right *= master;

        peak = juce::jmax(peak, juce::jmax(std::abs(left), std::abs(right)));
        subPeak = juce::jmax(subPeak, sub * master);

        const float scopeSample = 0.5f * (left + right);
        pushScopeSample(scopeSample);
        pushAnalyzerSample(scopeSample);

        buffer.setSample(0, sample, left);
        if (buffer.getNumChannels() > 1)
            buffer.setSample(1, sample, right);
    }

    meterLevel.store(juce::jmax(peak, meterLevel.load() * 0.90f));
    subMeterLevel.store(juce::jmax(subPeak, subMeterLevel.load() * 0.90f));
    midi.clear();
}

void BadlineDnBAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void BadlineDnBAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* BadlineDnBAudioProcessor::createEditor()
{
    return new BadlineDnBAudioProcessorEditor(*this);
}

void BadlineDnBAudioProcessor::copyScopeData(std::array<float, 512>& dest) const noexcept
{
    const int write = scopeWritePos.load();
    const int size = (int) scopeRing.size();

    for (int i = 0; i < (int) dest.size(); ++i)
    {
        const int index = (write - (int) dest.size() + i + size) % size;
        dest[(size_t) i] = scopeRing[(size_t) index];
    }
}

void BadlineDnBAudioProcessor::copyAnalyzerData(std::array<float, 128>& dest) const noexcept
{
    dest = analyzerBins;
}

void BadlineDnBAudioProcessor::handleNoteOn(int note, float velocity, PlayMode mode)
{
    if (note < 0 || note >= 128)
        return;

    keyDown[(size_t) note] = true;
    sustained[(size_t) note] = false;
    noteVelocity[(size_t) note] = velocity;
    removeNoteFromMonoStack(note);
    monoStack.push_back(note);

    if (mode == PlayMode::poly)
    {
        startPolyVoice(note, velocity, static_cast<NotePriority>(juce::roundToInt(apvts.getRawParameterValue("note_priority")->load())));
        return;
    }

    const bool legatoMode = (mode == PlayMode::legato);
    if (legatoMode && voices[0].active)
    {
        voices[0].legatoTo(note, velocity);
        voices[0].age = voiceCounter++;
    }
    else
    {
        startMonoVoice(note, velocity, true);
    }
}

void BadlineDnBAudioProcessor::handleNoteOff(int note, PlayMode mode)
{
    if (note < 0 || note >= 128)
        return;

    keyDown[(size_t) note] = false;

    if (sustainPedalDown)
    {
        sustained[(size_t) note] = true;

        if (mode == PlayMode::poly)
        {
            const int idx = findVoiceForNote(note);
            if (idx >= 0)
                voices[(size_t) idx].heldBySustain = true;
        }

        return;
    }

    if (mode == PlayMode::poly)
    {
        releasePolyNote(note);
        return;
    }

    removeNoteFromMonoStack(note);
    retriggerMonoFromStack();
}

void BadlineDnBAudioProcessor::handleSustainPedal(bool down, PlayMode mode)
{
    sustainPedalDown = down;
    if (down)
        return;

    for (int note = 0; note < 128; ++note)
    {
        if (sustained[(size_t) note] && ! keyDown[(size_t) note])
        {
            sustained[(size_t) note] = false;

            if (mode == PlayMode::poly)
                releasePolyNote(note);
        }
    }

    pruneReleasedMonoNotes();

    if (mode != PlayMode::poly)
        retriggerMonoFromStack();
}

void BadlineDnBAudioProcessor::startMonoVoice(int note, float velocity, bool retrigger)
{
    voices[0].start(note, velocity, retrigger);
    voices[0].age = voiceCounter++;
}

void BadlineDnBAudioProcessor::retriggerMonoFromStack()
{
    pruneReleasedMonoNotes();

    if (monoStack.empty())
    {
        releaseMonoIfIdle();
        return;
    }

    const int nextNote = monoStack.back();
    const float nextVelocity = noteVelocity[(size_t) nextNote] > 0.0f ? noteVelocity[(size_t) nextNote] : 1.0f;

    if (voices[0].active)
        voices[0].legatoTo(nextNote, nextVelocity);
    else
        voices[0].start(nextNote, nextVelocity, true);

    voices[0].age = voiceCounter++;
}

void BadlineDnBAudioProcessor::releaseMonoIfIdle()
{
    bool anyHeld = false;

    for (int note = 0; note < 128; ++note)
    {
        anyHeld = anyHeld || keyDown[(size_t) note] || (sustainPedalDown && sustained[(size_t) note]);
        if (anyHeld)
            break;
    }

    if (! anyHeld)
        voices[0].stop();
}

void BadlineDnBAudioProcessor::removeNoteFromMonoStack(int note)
{
    monoStack.erase(std::remove(monoStack.begin(), monoStack.end(), note), monoStack.end());
}

void BadlineDnBAudioProcessor::pruneReleasedMonoNotes()
{
    monoStack.erase(std::remove_if(monoStack.begin(), monoStack.end(), [this](int note)
    {
        return ! keyDown[(size_t) note] && ! (sustainPedalDown && sustained[(size_t) note]);
    }), monoStack.end());
}

int BadlineDnBAudioProcessor::findFreeVoice() const
{
    for (int i = 0; i < maxVoices; ++i)
        if (! voices[(size_t) i].active)
            return i;

    return -1;
}

int BadlineDnBAudioProcessor::findVoiceForNote(int note) const
{
    for (int i = 0; i < maxVoices; ++i)
        if (voices[(size_t) i].active && voices[(size_t) i].midiNote == note)
            return i;

    return -1;
}

int BadlineDnBAudioProcessor::stealVoice(NotePriority priority) const
{
    int candidate = 0;

    if (priority == NotePriority::low)
    {
        int lowest = std::numeric_limits<int>::max();
        for (int i = 0; i < maxVoices; ++i)
        {
            if (voices[(size_t) i].midiNote < lowest)
            {
                lowest = voices[(size_t) i].midiNote;
                candidate = i;
            }
        }

        return candidate;
    }

    if (priority == NotePriority::high)
    {
        int highest = std::numeric_limits<int>::lowest();
        for (int i = 0; i < maxVoices; ++i)
        {
            if (voices[(size_t) i].midiNote > highest)
            {
                highest = voices[(size_t) i].midiNote;
                candidate = i;
            }
        }

        return candidate;
    }

    uint64_t oldestAge = std::numeric_limits<uint64_t>::max();
    for (int i = 0; i < maxVoices; ++i)
    {
        if (voices[(size_t) i].age < oldestAge)
        {
            oldestAge = voices[(size_t) i].age;
            candidate = i;
        }
    }

    return candidate;
}

void BadlineDnBAudioProcessor::startPolyVoice(int note, float velocity, NotePriority priority)
{
    int idx = findFreeVoice();
    if (idx < 0)
        idx = stealVoice(priority);

    voices[(size_t) idx].start(note, velocity, true);
    voices[(size_t) idx].age = voiceCounter++;
}

void BadlineDnBAudioProcessor::releasePolyNote(int note)
{
    const int idx = findVoiceForNote(note);
    if (idx >= 0)
        voices[(size_t) idx].stop();
}

void BadlineDnBAudioProcessor::pushScopeSample(float s) noexcept
{
    const int size = (int) scopeRing.size();
    const int pos = scopeWritePos.fetch_add(1);
    scopeRing[(size_t) (pos % size)] = s;
}

void BadlineDnBAudioProcessor::pushAnalyzerSample(float s) noexcept
{
    if (fftFifoIndex < (int) fftFifo.size())
        fftFifo[(size_t) fftFifoIndex++] = s;

    if (fftFifoIndex == (int) fftFifo.size())
    {
        computeAnalyzerFrame();
        fftFifoIndex = 0;
    }
}

void BadlineDnBAudioProcessor::computeAnalyzerFrame() noexcept
{
    std::fill(fftData.begin(), fftData.end(), 0.0f);
    std::copy(fftFifo.begin(), fftFifo.end(), fftData.begin());
    window.multiplyWithWindowingTable(fftData.data(), fftFifo.size());
    fft.performFrequencyOnlyForwardTransform(fftData.data());

    for (size_t i = 0; i < analyzerBins.size(); ++i)
    {
        const size_t src = juce::jmin((size_t) 511, i * 4);
        const float mag = juce::Decibels::gainToDecibels(fftData[src] + 1.0e-5f, -80.0f);
        analyzerBins[i] = juce::jlimit(0.0f, 1.0f, juce::jmap(mag, -80.0f, 0.0f, 0.0f, 1.0f));
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BadlineDnBAudioProcessor();
}
