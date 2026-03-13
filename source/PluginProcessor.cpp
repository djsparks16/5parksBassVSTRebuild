#include "PluginProcessor.h"
#include "PluginEditor.h"

BadlineDnBAudioProcessor::BadlineDnBAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createParameters())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout BadlineDnBAudioProcessor::createParameters()
{
    using APF = juce::AudioParameterFloat;
    using APC = juce::AudioParameterChoice;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    auto norm = [](const juce::String& id, const juce::String& name,
                   float min, float max, float def, float skew = 1.0f)
    {
        return std::make_unique<APF>(
            id, name,
            juce::NormalisableRange<float>(min, max, 0.0f, skew),
            def);
    };

    p.push_back(std::make_unique<APC>("play_mode", "Play Mode", juce::StringArray { "Mono", "Legato", "Poly" }, 1));
    p.push_back(std::make_unique<APC>("note_priority", "Note Priority", juce::StringArray { "Last", "Low", "High" }, 0));
    p.push_back(norm("glide_ms", "Glide", 0.0f, 500.0f, 45.0f, 0.35f));

    p.push_back(norm("oscA_level","OscA Level",0.0f,1.0f,0.85f));
    p.push_back(norm("oscA_wave","OscA Wave",0.0f,1.0f,0.32f));
    p.push_back(norm("oscA_warp","OscA Warp",0.0f,1.0f,0.15f));
    p.push_back(norm("oscA_spread","OscA Spread",0.0f,1.0f,0.20f));

    p.push_back(norm("oscB_level","OscB Level",0.0f,1.0f,0.72f));
    p.push_back(norm("oscB_wave","OscB Wave",0.0f,1.0f,0.58f));
    p.push_back(norm("oscB_warp","OscB Warp",0.0f,1.0f,0.12f));
    p.push_back(norm("oscB_spread","OscB Spread",0.0f,1.0f,0.26f));

    p.push_back(norm("oscC_level","OscC Level",0.0f,1.0f,0.45f));
    p.push_back(norm("oscC_wave","OscC Wave",0.0f,1.0f,0.12f));
    p.push_back(norm("oscC_warp","OscC Warp",0.0f,1.0f,0.08f));

    p.push_back(norm("sub_level","Sub Level",0.0f,1.0f,0.72f));
    p.push_back(norm("sub_drive","Sub Drive",0.0f,1.0f,0.06f));
    p.push_back(norm("sub_octave","Sub Octave",0.0f,1.0f,0.30f));
    p.push_back(norm("noise_level","Noise Level",0.0f,1.0f,0.03f));

    p.push_back(norm("filter_cutoff","Filter 1 Cutoff",20.0f,18000.0f,1300.0f,0.30f));
    p.push_back(norm("filter_res","Filter 1 Res",0.1f,1.2f,0.26f));
    p.push_back(norm("filter_drive","Filter 1 Drive",0.0f,1.0f,0.18f));

    p.push_back(norm("filter2_cutoff","Filter 2 Cutoff",20.0f,18000.0f,5200.0f,0.30f));
    p.push_back(norm("filter2_res","Filter 2 Res",0.1f,1.2f,0.20f));

    p.push_back(norm("dist_drive","Distortion Drive",0.0f,1.0f,0.28f));
    p.push_back(norm("dist_mix","Distortion Mix",0.0f,1.0f,0.55f));
    p.push_back(norm("output_clip","Output Clip",0.0f,1.0f,0.25f));

    p.push_back(norm("lfo1_rate","LFO 1 Rate",0.01f,20.0f,3.8f,0.35f));
    p.push_back(norm("lfo1_amt","LFO 1 Amount",0.0f,1.0f,0.22f));
    p.push_back(norm("envamt","Env Amount",0.0f,2.0f,0.70f));
    p.push_back(norm("envA","Attack",0.001f,2.0f,0.005f,0.40f));
    p.push_back(norm("envD","Decay",0.001f,2.0f,0.22f,0.40f));
    p.push_back(norm("envS","Sustain",0.0f,1.0f,0.75f));
    p.push_back(norm("envR","Release",0.001f,3.0f,0.16f,0.40f));

    p.push_back(norm("reese_detune", "Reese Detune", 0.0f, 1.0f, 0.28f));
    p.push_back(norm("reese_width", "Reese Width", 0.0f, 1.0f, 0.36f));
    p.push_back(norm("reese_drift", "Reese Drift", 0.0f, 1.0f, 0.18f));

    p.push_back(norm("horn_bend", "Horn Bend", 0.0f, 1.0f, 0.15f));
    p.push_back(norm("horn_formant", "Horn Formant", 0.0f, 1.0f, 0.18f));
    p.push_back(norm("horn_body", "Horn Body", 0.0f, 1.0f, 0.25f));

    p.push_back(norm("screech_fm", "Screech FM", 0.0f, 1.0f, 0.12f));
    p.push_back(norm("screech_drive", "Screech Drive", 0.0f, 1.0f, 0.10f));

    p.push_back(norm("harm_mix", "Harmony Mix", 0.0f, 1.0f, 0.00f));
    p.push_back(norm("harm_interval", "Harmony Interval", -12.0f, 12.0f, 7.0f));
    p.push_back(norm("harm_spread", "Harmony Spread", 0.0f, 1.0f, 0.20f));

    for (int i = 1; i <= 8; ++i)
        p.push_back(norm("macro" + juce::String(i), "Macro " + juce::String(i), 0.0f, 1.0f, 0.0f));

    p.push_back(norm("master_gain","Output",0.0f,1.0f,0.82f));

    return { p.begin(), p.end() };
}

void BadlineDnBAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    for (auto& v : voices)
        v.prepare(sampleRate);

    keyDown.fill(false);
    sustained.fill(false);
    monoStack.clear();
    sustainPedalDown = false;
    meterLevel.store(0.0f);
    subMeterLevel.store(0.0f);
    scopeWritePos.store(0);
    fftFifoIndex = 0;
    analyzerBins.fill(0.0f);
}

void BadlineDnBAudioProcessor::releaseResources() {}

bool BadlineDnBAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void BadlineDnBAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    const PlayMode playMode = static_cast<PlayMode>((int) apvts.getRawParameterValue("play_mode")->load());
    const NotePriority priority = static_cast<NotePriority>((int) apvts.getRawParameterValue("note_priority")->load());

    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();

        if (msg.isController() && msg.getControllerNumber() == 64)
        {
            handleSustainPedal(msg.getControllerValue() >= 64, playMode);
            continue;
        }

        if (msg.isNoteOn())  handleNoteOn(msg.getNoteNumber(), msg.getFloatVelocity(), playMode);
        if (msg.isNoteOff()) handleNoteOff(msg.getNoteNumber(), playMode);

        if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            for (auto& v : voices) v.stop();
            keyDown.fill(false);
            sustained.fill(false);
            monoStack.clear();
            sustainPedalDown = false;
        }
    }

    const float glideMs = apvts.getRawParameterValue("glide_ms")->load();
    const float oscALevel  = apvts.getRawParameterValue("oscA_level")->load();
    const float oscAWave   = apvts.getRawParameterValue("oscA_wave")->load();
    const float oscAWarp   = apvts.getRawParameterValue("oscA_warp")->load();
    const float oscASpread = apvts.getRawParameterValue("oscA_spread")->load();
    const float oscBLevel  = apvts.getRawParameterValue("oscB_level")->load();
    const float oscBWave   = apvts.getRawParameterValue("oscB_wave")->load();
    const float oscBWarp   = apvts.getRawParameterValue("oscB_warp")->load();
    const float oscBSpread = apvts.getRawParameterValue("oscB_spread")->load();
    const float oscCLevel  = apvts.getRawParameterValue("oscC_level")->load();
    const float oscCWave   = apvts.getRawParameterValue("oscC_wave")->load();
    const float oscCWarp   = apvts.getRawParameterValue("oscC_warp")->load();
    const float subLevel   = apvts.getRawParameterValue("sub_level")->load();
    const float subDrive   = apvts.getRawParameterValue("sub_drive")->load();
    const float subOctave  = apvts.getRawParameterValue("sub_octave")->load();
    const float noiseLevel = apvts.getRawParameterValue("noise_level")->load();
    const float cutoff1    = apvts.getRawParameterValue("filter_cutoff")->load();
    const float res1       = apvts.getRawParameterValue("filter_res")->load();
    const float drive1     = apvts.getRawParameterValue("filter_drive")->load();
    const float cutoff2    = apvts.getRawParameterValue("filter2_cutoff")->load();
    const float res2       = apvts.getRawParameterValue("filter2_res")->load();
    const float distDrive  = apvts.getRawParameterValue("dist_drive")->load();
    const float distMix    = apvts.getRawParameterValue("dist_mix")->load();
    const float outputClip = apvts.getRawParameterValue("output_clip")->load();
    const float lfoRate    = apvts.getRawParameterValue("lfo1_rate")->load();
    const float lfoAmt     = apvts.getRawParameterValue("lfo1_amt")->load();
    const float envAmt     = apvts.getRawParameterValue("envamt")->load();
    const float envA       = apvts.getRawParameterValue("envA")->load();
    const float envD       = apvts.getRawParameterValue("envD")->load();
    const float envS       = apvts.getRawParameterValue("envS")->load();
    const float envR       = apvts.getRawParameterValue("envR")->load();
    const float reeseDetune = apvts.getRawParameterValue("reese_detune")->load();
    const float reeseWidth  = apvts.getRawParameterValue("reese_width")->load();
    const float reeseDrift  = apvts.getRawParameterValue("reese_drift")->load();
    const float hornBend    = apvts.getRawParameterValue("horn_bend")->load();
    const float hornFormant = apvts.getRawParameterValue("horn_formant")->load();
    const float hornBody    = apvts.getRawParameterValue("horn_body")->load();
    const float screechFm   = apvts.getRawParameterValue("screech_fm")->load();
    const float screechDrive= apvts.getRawParameterValue("screech_drive")->load();
    const float harmMix     = apvts.getRawParameterValue("harm_mix")->load();
    const float harmInterval= apvts.getRawParameterValue("harm_interval")->load();
    const float harmSpread  = apvts.getRawParameterValue("harm_spread")->load();
    const float macro1      = apvts.getRawParameterValue("macro1")->load();
    const float macro2      = apvts.getRawParameterValue("macro2")->load();
    const float master      = apvts.getRawParameterValue("master_gain")->load();

    for (auto& v : voices)
    {
        v.setGlideMs(glideMs);
        v.updateADSR(envA, envD, envS, envR);
    }

    float peak = 0.0f;
    float subPeak = 0.0f;

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        float s = 0.0f;

        if (playMode == PlayMode::poly)
        {
            for (auto& v : voices)
            {
                s += v.nextSample(
                    oscALevel, oscAWave, oscAWarp, oscASpread,
                    oscBLevel, oscBWave, oscBWarp, oscBSpread,
                    oscCLevel, oscCWave, oscCWarp,
                    subLevel, subDrive, noiseLevel,
                    cutoff1, res1, drive1,
                    cutoff2, res2,
                    distDrive, distMix,
                    lfoRate, lfoAmt, envAmt,
                    macro1, macro2,
                    reeseDetune, reeseWidth, reeseDrift,
                    hornBend, hornFormant, hornBody,
                    screechFm, screechDrive,
                    harmMix, harmInterval, harmSpread,
                    subOctave, outputClip);
            }
            s *= 0.22f;
        }
        else
        {
            s = voices[0].nextSample(
                oscALevel, oscAWave, oscAWarp, oscASpread,
                oscBLevel, oscBWave, oscBWarp, oscBSpread,
                oscCLevel, oscCWave, oscCWarp,
                subLevel, subDrive, noiseLevel,
                cutoff1, res1, drive1,
                cutoff2, res2,
                distDrive, distMix,
                lfoRate, lfoAmt, envAmt,
                macro1, macro2,
                reeseDetune, reeseWidth, reeseDrift,
                hornBend, hornFormant, hornBody,
                screechFm, screechDrive,
                harmMix, harmInterval, harmSpread,
                subOctave, outputClip);
        }

        s *= master;
        peak = juce::jmax(peak, std::abs(s));
        subPeak = juce::jmax(subPeak, std::abs(s) * subLevel);
        pushScopeSample(s);
        pushAnalyzerSample(s);

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.setSample(ch, i, s);
    }

    meterLevel.store(juce::jmax(peak, meterLevel.load() * 0.90f));
    subMeterLevel.store(juce::jmax(subPeak, subMeterLevel.load() * 0.90f));
}

void BadlineDnBAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void BadlineDnBAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
    keyDown[(size_t) note] = true;
    sustained[(size_t) note] = false;
    monoStack.erase(std::remove(monoStack.begin(), monoStack.end(), note), monoStack.end());
    monoStack.push_back(note);

    if (mode == PlayMode::poly)
    {
        startPolyVoice(note, velocity, static_cast<NotePriority>((int) apvts.getRawParameterValue("note_priority")->load()));
        return;
    }

    const bool legatoMode = (mode == PlayMode::legato);
    if (legatoMode && voices[0].active)
        voices[0].legatoTo(note, velocity);
    else
        startMonoVoice(note, velocity, true);
}

void BadlineDnBAudioProcessor::handleNoteOff(int note, PlayMode mode)
{
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

    monoStack.erase(std::remove(monoStack.begin(), monoStack.end(), note), monoStack.end());
    retriggerMonoFromStack();
}

void BadlineDnBAudioProcessor::handleSustainPedal(bool down, PlayMode mode)
{
    sustainPedalDown = down;
    if (down) return;

    for (int note = 0; note < 128; ++note)
    {
        if (sustained[(size_t) note] && ! keyDown[(size_t) note])
        {
            sustained[(size_t) note] = false;
            if (mode == PlayMode::poly)
                releasePolyNote(note);
        }
    }

    if (mode != PlayMode::poly)
        retriggerMonoFromStack();
}

void BadlineDnBAudioProcessor::startMonoVoice(int note, float velocity, bool retrigger)
{
    voices[0].start(note, velocity, retrigger);
}

void BadlineDnBAudioProcessor::retriggerMonoFromStack()
{
    if (monoStack.empty())
    {
        releaseMonoIfIdle();
        return;
    }

    const int nextNote = monoStack.back();
    if (sustainPedalDown && sustained[(size_t) nextNote])
        return;

    if (voices[0].active)
        voices[0].legatoTo(nextNote, 1.0f);
    else
        voices[0].start(nextNote, 1.0f, true);
}

void BadlineDnBAudioProcessor::releaseMonoIfIdle()
{
    bool anyHeld = false;
    for (bool k : keyDown) anyHeld = anyHeld || k;
    if (! anyHeld) voices[0].stop();
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
        int lowest = 127;
        for (int i = 0; i < maxVoices; ++i)
            if (voices[(size_t) i].midiNote < lowest) { lowest = voices[(size_t) i].midiNote; candidate = i; }
    }
    else if (priority == NotePriority::high)
    {
        int highest = -1;
        for (int i = 0; i < maxVoices; ++i)
            if (voices[(size_t) i].midiNote > highest) { highest = voices[(size_t) i].midiNote; candidate = i; }
    }
    return candidate;
}

void BadlineDnBAudioProcessor::startPolyVoice(int note, float velocity, NotePriority priority)
{
    int idx = findFreeVoice();
    if (idx < 0) idx = stealVoice(priority);
    voices[(size_t) idx].start(note, velocity, true);
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
