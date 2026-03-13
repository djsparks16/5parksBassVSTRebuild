#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <vector>

#include "PluginProcessor.h"

class ScopeComponent : public juce::Component, private juce::Timer
{
public:
    explicit ScopeComponent(BadlineDnBAudioProcessor& p) : processor(p) { startTimerHz(30); }
    void paint(juce::Graphics& g) override;
    void resized() override {}

private:
    void timerCallback() override;

    BadlineDnBAudioProcessor& processor;
    std::array<float, 512> scopeData {};
};

class SpectrumComponent : public juce::Component, private juce::Timer
{
public:
    explicit SpectrumComponent(BadlineDnBAudioProcessor& p) : processor(p) { startTimerHz(24); }
    void paint(juce::Graphics& g) override;
    void resized() override {}

private:
    void timerCallback() override;

    BadlineDnBAudioProcessor& processor;
    std::array<float, 128> bins {};
};

class LabeledKnob : public juce::Component
{
public:
    LabeledKnob();

    juce::Slider slider;
    juce::Label title;
    juce::Label footer;

    void resized() override;
};

class BadlineDnBAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                        private juce::Timer
{
public:
    explicit BadlineDnBAudioProcessorEditor (BadlineDnBAudioProcessor&);
    ~BadlineDnBAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    enum class Page { Osc, Motion, Tone, Perform, Global };

    void timerCallback() override;
    void switchPage(Page newPage);
    void setupKnob(LabeledKnob& k, const juce::String& header, const juce::String& footer);
    void styleTab(juce::TextButton& b);
    void styleCombo(juce::ComboBox& c);
    void attach(LabeledKnob& knob, const juce::String& paramID);

    BadlineDnBAudioProcessor& audioProcessor;
    Page currentPage = Page::Osc;

    juce::TextButton oscBtn     { "OSC" };
    juce::TextButton motionBtn  { "MOTION" };
    juce::TextButton toneBtn    { "TONE" };
    juce::TextButton performBtn { "PERFORM" };
    juce::TextButton globalBtn  { "GLOBAL" };

    juce::Component oscPage;
    juce::Component motionPage;
    juce::Component tonePage;
    juce::Component performPage;
    juce::Component globalPage;

    ScopeComponent scope;
    SpectrumComponent spectrum;

    juce::Label oscPageHeader, motionPageHeader, tonePageHeader, performPageHeader, globalPageHeader;
    juce::Label playModeLabel;
    juce::Label notePriorityLabel;

    LabeledKnob macro1, macro2, macro3, macro4;
    LabeledKnob reeseDetune, reeseWidth, hornBend, hornFormant;
    LabeledKnob oscAWave, oscAWarp, oscBWave, oscBWarp, oscCWave, oscCWarp;
    LabeledKnob subLevel, subDrive, subOctave, noiseLevel;
    LabeledKnob cutoff, res, filterDrive, filter2Cutoff, filter2Res;
    LabeledKnob distDrive, distMix, screechDrive, harmMix;
    LabeledKnob lfoRate, lfoAmt, envAmt, glide;
    LabeledKnob masterGain, outputClip;

    juce::ComboBox playMode;
    juce::ComboBox notePriority;

    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::unique_ptr<ComboBoxAttachment> playModeAtt;
    std::unique_ptr<ComboBoxAttachment> notePriorityAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BadlineDnBAudioProcessorEditor)
};
