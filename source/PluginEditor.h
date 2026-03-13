#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>

#include "PluginProcessor.h"

class ScopeComponent : public juce::Component, private juce::Timer
{
public:
    explicit ScopeComponent(BadlineDnBAudioProcessor& p) : processor(p)
    {
        startTimerHz(30);
    }

    void paint(juce::Graphics& g) override;
    void resized() override {}

private:
    void timerCallback() override;

    BadlineDnBAudioProcessor& processor;
    std::array<float, 512> scopeData {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScopeComponent)
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

    enum class Page { Sound, Motion, Tone, Perform };

    void timerCallback() override;
    void switchPage(Page newPage);

    BadlineDnBAudioProcessor& audioProcessor;
    Page currentPage = Page::Sound;

    juce::TextButton soundBtn   { "Sound" };
    juce::TextButton motionBtn  { "Motion" };
    juce::TextButton toneBtn    { "Tone" };
    juce::TextButton performBtn { "Perform" };

    juce::Component soundPage;
    juce::Component motionPage;
    juce::Component tonePage;
    juce::Component performPage;

    ScopeComponent scope;

    juce::Slider macro1, macro2, macro3, macro4;
    juce::Slider cutoff, res, drive;
    juce::Slider glide;
    juce::ComboBox playMode;
    juce::ComboBox notePriority;

    std::unique_ptr<SliderAttachment> macro1Att, macro2Att, macro3Att, macro4Att;
    std::unique_ptr<SliderAttachment> cutoffAtt, resAtt, driveAtt, glideAtt;
    std::unique_ptr<ComboBoxAttachment> playModeAtt, notePriorityAtt;

    void setupKnob(juce::Slider& s);
    void styleTab(juce::TextButton& b);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BadlineDnBAudioProcessorEditor)
};
