#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <array>
#include <memory>
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

class BadlineDnBAudioProcessorEditor : public juce::AudioProcessorEditor,
                                      private juce::Timer
{
public:
    explicit BadlineDnBAudioProcessorEditor(BadlineDnBAudioProcessor&);
    ~BadlineDnBAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    enum class Page { Osc, Motion, Tone, Perform, Global };

    void timerCallback() override;
    void switchPage(Page newPage);
    void styleTab(juce::TextButton& b);
    void styleCombo(juce::ComboBox& c);
    void styleHeader(juce::Label& l, const juce::String& text);
    void setupKnob(LabeledKnob& k, const juce::String& header, const juce::String& footer);
    void addKnobToPage(juce::Component& page, LabeledKnob& knob, const juce::String& title, const juce::String& footer, const juce::String& paramID);
    void attach(LabeledKnob& knob, const juce::String& paramID);
    juce::Component& pageComponent(Page p);
    juce::String pageTitle(Page p) const;

    BadlineDnBAudioProcessor& audioProcessor;
    Page currentPage = Page::Osc;

    juce::TextButton oscBtn { "OSC" }, motionBtn { "MOD" }, toneBtn { "FILTER/FX" }, performBtn { "VOICING" }, globalBtn { "MASTER" };
    juce::Component oscPage, motionPage, tonePage, performPage, globalPage;
    juce::Label oscPageHeader, motionPageHeader, tonePageHeader, performPageHeader, globalPageHeader;
    ScopeComponent scope;
    SpectrumComponent spectrum;

    LabeledKnob oscAWave, oscAWarp, oscALevel, oscBWave, oscBWarp, oscBLevel, oscCWave, oscCWarp, oscCLevel, subLevel, subDrive, subOctave, noiseLevel, reeseDetune, reeseWidth, reeseDrift;
    LabeledKnob macro1, macro2, macro3, macro4, lfoRate, lfoAmt, envAmt, buzzAmt, talkAmt, biteAmt, punchAmt, wobbleAmt, hornBend, hornFormant, hornBody, warhornMix, screechDrive, screechFm, fmGrit, wompAmt, comboStack;
    LabeledKnob cutoff, res, filterDrive, filter2Cutoff, filter2Res, notchAmt, distDrive, distMix, outputClip;
    LabeledKnob harmMix, harmInterval, harmSpread, subHarmony, airAmt, stereoSpin, monoBlend, masterGain;
    LabeledKnob glide, attack, decay, sustain, release;

    juce::Label playModeLabel, notePriorityLabel;
    juce::ComboBox playMode, notePriority;

    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::unique_ptr<ComboBoxAttachment> playModeAtt;
    std::unique_ptr<ComboBoxAttachment> notePriorityAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BadlineDnBAudioProcessorEditor)
};
