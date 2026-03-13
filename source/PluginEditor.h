#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include "PluginProcessor.h"

class BlacksidePage : public juce::Component
{
public:
    explicit BlacksidePage(const juce::String& titleToShow) : title(titleToShow) {}

    void paint(juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced(16.0f);
        g.setColour(juce::Colour(0xff121318));
        g.fillRoundedRectangle(r, 22.0f);
        g.setColour(juce::Colour(0x33d8f8ff));
        g.drawRoundedRectangle(r, 22.0f, 1.0f);
        g.setColour(juce::Colours::white.withAlpha(0.95f));
        g.setFont(juce::FontOptions(22.0f, juce::Font::bold));
        g.drawText(title, r.reduced(24.0f).toNearestInt(), juce::Justification::topLeft, false);
    }

private:
    juce::String title;
};

class MacroStrip : public juce::Component
{
public:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    explicit MacroStrip(juce::AudioProcessorValueTreeState& state);
    void resized() override;

private:
    std::array<juce::Slider, 8> sliders;
    std::array<std::unique_ptr<SliderAttachment>, 8> attachments;
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
    enum class Page { Sound, Motion, Tone, Perform };

    void switchPage(Page newPage);
    void timerCallback() override;

    BadlineDnBAudioProcessor& processor;
    Page currentPage = Page::Sound;

    juce::TextButton soundBtn {"Sound"};
    juce::TextButton motionBtn {"Motion"};
    juce::TextButton toneBtn {"Tone"};
    juce::TextButton performBtn {"Perform"};

    BlacksidePage soundPage {"Sound Lab"};
    BlacksidePage motionPage {"Motion Matrix"};
    BlacksidePage tonePage {"Tone Forge"};
    BlacksidePage performPage {"Performance"};

    MacroStrip macroStrip;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BadlineDnBAudioProcessorEditor)
};
