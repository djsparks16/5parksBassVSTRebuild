#include "PluginProcessor.h"
#include "PluginEditor.h"

BadlineDnBAudioProcessor::BadlineDnBAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createParameters())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout
BadlineDnBAudioProcessor::createParameters()
{
    using APF = juce::AudioParameterFloat;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    auto norm = [](const juce::String& id, const juce::String& name,
                   float min, float max, float def, float skew = 1.0f)
    {
        return std::make_unique<APF>(
            id, name,
            juce::NormalisableRange<float>(min, max, 0.0f, skew),
            def);
    };

    p.push_back(norm("oscA_level","OscA Level",0.0f,1.0f,0.85f));
    p.push_back(norm("oscA_wave","OscA Wave",0.0f,1.0f,0.30f));
    p.push_back(norm("oscA_warp","OscA Warp",0.0f,1.0f,0.15f));
    p.push_back(norm("oscA_spread","OscA Spread",0.0f,1.0f,0.18f));

    p.push_back(norm("oscB_level","OscB Level",0.0f,1.0f,0.65f));
    p.push_back(norm("oscB_wave","OscB Wave",0.0f,1.0f,0.60f));
    p.push_back(norm("oscB_warp","OscB Warp",0.0f,1.0f,0.10f));
    p.push_back(norm("oscB_spread","OscB Spread",0.0f,1.0f,0.24f));

    p.push_back(norm("oscC_level","OscC Level",0.0f,1.0f,0.45f));
    p.push_back(norm("oscC_wave","OscC Wave",0.0f,1.0f,0.0f));
    p.push_back(norm("oscC_warp","OscC Warp",0.0f,1.0f,0.0f));

    p.push_back(norm("sub_level","Sub Level",0.0f,1.0f,0.70f));
    p.push_back(norm("sub_drive","Sub Drive",0.0f,1.0f,0.05f));
    p.push_back(norm("noise_level","Noise Level",0.0f,1.0f,0.04f));

    p.push_back(norm("filter_cutoff","Filter 1 Cutoff",20.0f,18000.0f,1200.0f,0.30f));
    p.push_back(norm("filter_res","Filter 1 Res",0.1f,1.2f,0.28f));
    p.push_back(norm("filter_drive","Filter 1 Drive",0.0f,1.0f,0.15f));

    p.push_back(norm("filter2_cutoff","Filter 2 Cutoff",20.0f,18000.0f,5200.0f,0.30f));
    p.push_back(norm("filter2_res","Filter 2 Res",0.1f,1.2f,0.22f));

    p.push_back(norm("dist_drive","Distortion Drive",0.0f,1.0f,0.25f));
    p.push_back(norm("dist_mix","Distortion Mix",0.0f,1.0f,0.55f));

    p.push_back(norm("lfo1_rate","LFO 1 Rate",0.01f,20.0f,3.8f,0.35f));
    p.push_back(norm("lfo1_amt","LFO 1 Amount",0.0f,1.0f,0.22f));

    p.push_back(norm("envamt","Env Amount",0.0f,2.0f,0.70f));
    p.push_back(norm("envA","Attack",0.001f,2.0f,0.005f,0.40f));
    p.push_back(norm("envD","Decay",0.001f,2.0f,0.22f,0.40f));
    p.push_back(norm("envS","Sustain",0.0f,1.0f,0.75f));
    p.push_back(norm("envR","Release",0.001f,3.0f,0.16f,0.40f));

    for (int i = 1; i <= 8; ++i)
        p.push_back(norm("macro" + juce::String(i), "Macro " + juce::String(i), 0.0f, 1.0f, 0.0f));

    p.push_back(norm("master_gain","Output",0.0f,1.0f,0.82f));

    return { p.begin(), p.end() };
}

void BadlineDnBAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    voice.prepare(sampleRate);
    meterLevel.store(0.0f);
    subMeterLevel.store(0.0f);
}

void BadlineDnBAudioProcessor::releaseResources()
{
}

bool BadlineDnBAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void BadlineDnBAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn()) voice.start(msg.getNoteNumber(), msg.getFloatVelocity());
        if (msg.isNoteOff()) voice.stop();
    }

    const float oscALevel = apvts.getRawParameterValue("oscA_level")->load();
    const float oscAWave = apvts.getRawParameterValue("oscA_wave")->load();
    const float oscAWarp = apvts.getRawParameterValue("oscA_warp")->load();
    const float oscASpread = apvts.getRawParameterValue("oscA_spread")->load();

    const float oscBLevel = apvts.getRawParameterValue("oscB_level")->load();
    const float oscBWave = apvts.getRawParameterValue("oscB_wave")->load();
    const float oscBWarp = apvts.getRawParameterValue("oscB_warp")->load();
    const float oscBSpread = apvts.getRawParameterValue("oscB_spread")->load();

    const float oscCLevel = apvts.getRawParameterValue("oscC_level")->load();
    const float oscCWave = apvts.getRawParameterValue("oscC_wave")->load();
    const float oscCWarp = apvts.getRawParameterValue("oscC_warp")->load();

    const float subLevel = apvts.getRawParameterValue("sub_level")->load();
    const float subDrive = apvts.getRawParameterValue("sub_drive")->load();
    const float noiseLevel = apvts.getRawParameterValue("noise_level")->load();

    const float cutoff1 = apvts.getRawParameterValue("filter_cutoff")->load();
    const float res1 = apvts.getRawParameterValue("filter_res")->load();
    const float drive1 = apvts.getRawParameterValue("filter_drive")->load();
    const float cutoff2 = apvts.getRawParameterValue("filter2_cutoff")->load();
    const float res2 = apvts.getRawParameterValue("filter2_res")->load();

    const float distDrive = apvts.getRawParameterValue("dist_drive")->load();
    const float distMix = apvts.getRawParameterValue("dist_mix")->load();

    const float lfoRate = apvts.getRawParameterValue("lfo1_rate")->load();
    const float lfoAmt = apvts.getRawParameterValue("lfo1_amt")->load();
    const float envAmt = apvts.getRawParameterValue("envamt")->load();

    const float envA = apvts.getRawParameterValue("envA")->load();
    const float envD = apvts.getRawParameterValue("envD")->load();
    const float envS = apvts.getRawParameterValue("envS")->load();
    const float envR = apvts.getRawParameterValue("envR")->load();

    const float macro1 = apvts.getRawParameterValue("macro1")->load();
    const float macro2 = apvts.getRawParameterValue("macro2")->load();
    const float master = apvts.getRawParameterValue("master_gain")->load();

    voice.updateADSR(envA, envD, envS, envR);

    float peak = 0.0f;
    float subPeak = 0.0f;

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        float s = voice.nextSample(
            oscALevel, oscAWave, oscAWarp, oscASpread,
            oscBLevel, oscBWave, oscBWarp, oscBSpread,
            oscCLevel, oscCWave, oscCWarp,
            subLevel, subDrive, noiseLevel,
            cutoff1, res1, drive1,
            cutoff2, res2,
            distDrive, distMix,
            lfoRate, lfoAmt, envAmt,
            macro1, macro2);

        s *= master;
        peak = juce::jmax(peak, std::abs(s));
        subPeak = juce::jmax(subPeak, std::abs(std::sin(voice.subOsc.phase * juce::MathConstants<float>::twoPi) * subLevel));

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

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BadlineDnBAudioProcessor();
}
