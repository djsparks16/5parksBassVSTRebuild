#include "PluginEditor.h"

namespace
{
    void styleNavButton(juce::TextButton& b)
    {
        b.setClickingTogglesState(false);
        b.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff191b20));
        b.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffd8f8ff));
        b.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.92f));
        b.setColour(juce::TextButton::textColourOnId, juce::Colour(0xff0a0b0e));
    }

    void styleMacroSlider(juce::Slider& s)
    {
        s.setSliderStyle(juce::Slider::LinearVertical);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 48, 16);
        s.setColour(juce::Slider::trackColourId, juce::Colour(0x44d8f8ff));
        s.setColour(juce::Slider::thumbColourId, juce::Colour(0xffd8f8ff));
        s.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff15171c));
        s.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        s.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff111318));
        s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    }
}

MacroStrip::MacroStrip(juce::AudioProcessorValueTreeState& state)
{
    for (int i = 0; i < 8; ++i)
    {
        auto& s = sliders[(size_t) i];
        styleMacroSlider(s);
        addAndMakeVisible(s);
        attachments[(size_t) i] = std::make_unique<SliderAttachment>(state, "macro" + juce::String(i + 1), s);
    }
}

void MacroStrip::resized()
{
    auto area = getLocalBounds().reduced(8);
    const int width = juce::jmax(40, area.getWidth() / 8);
    for (int i = 0; i < 8; ++i)
        sliders[(size_t) i].setBounds(area.removeFromLeft(width).reduced(4));
}

BadlineDnBAudioProcessorEditor::BadlineDnBAudioProcessorEditor (BadlineDnBAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p), macroStrip(p.apvts)
{
    setSize(1280, 780);

    for (auto* b : { &soundBtn, &motionBtn, &toneBtn, &performBtn })
    {
        styleNavButton(*b);
        addAndMakeVisible(*b);
    }

    soundBtn.onClick = [this] { switchPage(Page::Sound); };
    motionBtn.onClick = [this] { switchPage(Page::Motion); };
    toneBtn.onClick = [this] { switchPage(Page::Tone); };
    performBtn.onClick = [this] { switchPage(Page::Perform); };

    addAndMakeVisible(soundPage);
    addAndMakeVisible(motionPage);
    addAndMakeVisible(tonePage);
    addAndMakeVisible(performPage);
    addAndMakeVisible(macroStrip);

    switchPage(Page::Sound);
    startTimerHz(30);
}

void BadlineDnBAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.fillAll(juce::Colour(0xff0a0b0e));

    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff0b0c10), 0.0f, 0.0f,
                                           juce::Colour(0xff151820), 0.0f, bounds.getBottom(), false));
    g.fillRoundedRectangle(bounds.reduced(10.0f), 28.0f);

    g.setColour(juce::Colour(0x33d8f8ff));
    g.drawRoundedRectangle(bounds.reduced(10.0f), 28.0f, 1.2f);

    g.setColour(juce::Colours::white.withAlpha(0.98f));
    g.setFont(juce::FontOptions(32.0f, juce::Font::bold));
    g.drawText("Blackside Bass", 28, 18, 340, 34, juce::Justification::left);

    g.setColour(juce::Colours::white.withAlpha(0.45f));
    g.setFont(juce::FontOptions(14.0f, juce::Font::plain));
    g.drawText("Bass Instrument", 31, 50, 180, 18, juce::Justification::left);

    auto drawMeter = [&](juce::Rectangle<float> area, float value, const juce::String& label, juce::Colour c)
    {
        g.setColour(juce::Colour(0xff121418));
        g.fillRoundedRectangle(area, 10.0f);
        g.setColour(juce::Colours::white.withAlpha(0.12f));
        g.drawRoundedRectangle(area, 10.0f, 1.0f);

        auto inner = area.reduced(5.0f);
        auto h = inner.getHeight() * juce::jlimit(0.0f, 1.0f, value);
        juce::Rectangle<float> fill(inner.getX(), inner.getBottom() - h, inner.getWidth(), h);
        g.setGradientFill(juce::ColourGradient(c.brighter(), fill.getCentreX(), fill.getBottom(), c.darker(0.6f), fill.getCentreX(), fill.getY(), false));
        g.fillRoundedRectangle(fill, 6.0f);

        g.setColour(juce::Colours::white.withAlpha(0.75f));
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.drawText(label, juce::Rectangle<int>((int) area.getX() - 4, (int) area.getBottom() + 6, (int) area.getWidth() + 8, 14), juce::Justification::centred, false);
    };

    drawMeter({ 1180.0f, 96.0f, 22.0f, 180.0f }, processor.getMeterLevel(), "OUT", juce::Colour(0xffd8f8ff));
    drawMeter({ 1210.0f, 96.0f, 22.0f, 180.0f }, processor.getSubMeterLevel(), "SUB", juce::Colour(0xff7ce6ff));
}

void BadlineDnBAudioProcessorEditor::resized()
{
    const int topY = 86;
    const int navH = 34;
    const int navW = 110;

    soundBtn.setBounds(28, topY, navW, navH);
    motionBtn.setBounds(146, topY, navW, navH);
    toneBtn.setBounds(264, topY, navW, navH);
    performBtn.setBounds(382, topY, navW, navH);

    auto content = getLocalBounds().reduced(20);
    content.removeFromTop(126);

    auto macroArea = content.removeFromBottom(170);
    macroStrip.setBounds(macroArea.removeFromLeft(560).reduced(4));

    soundPage.setBounds(content);
    motionPage.setBounds(content);
    tonePage.setBounds(content);
    performPage.setBounds(content);
}

void BadlineDnBAudioProcessorEditor::switchPage(Page newPage)
{
    currentPage = newPage;
    soundPage.setVisible(newPage == Page::Sound);
    motionPage.setVisible(newPage == Page::Motion);
    tonePage.setVisible(newPage == Page::Tone);
    performPage.setVisible(newPage == Page::Perform);

    soundBtn.setToggleState(newPage == Page::Sound, juce::dontSendNotification);
    motionBtn.setToggleState(newPage == Page::Motion, juce::dontSendNotification);
    toneBtn.setToggleState(newPage == Page::Tone, juce::dontSendNotification);
    performBtn.setToggleState(newPage == Page::Perform, juce::dontSendNotification);
}

void BadlineDnBAudioProcessorEditor::timerCallback()
{
    repaint();
}
