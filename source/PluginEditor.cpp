#include "PluginEditor.h"

void ScopeComponent::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();

    g.setColour(juce::Colour(0xff0f1318));
    g.fillRoundedRectangle(area, 16.0f);

    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawRoundedRectangle(area, 16.0f, 1.0f);

    g.setColour(juce::Colours::white.withAlpha(0.15f));
    for (int i = 1; i < 4; ++i)
    {
        const float y = area.getY() + area.getHeight() * (float) i / 4.0f;
        g.drawLine(area.getX() + 12.0f, y, area.getRight() - 12.0f, y, 1.0f);
    }

    juce::Path p;
    const auto w = area.getWidth() - 24.0f;
    const auto h = area.getHeight() - 24.0f;
    const auto x0 = area.getX() + 12.0f;
    const auto y0 = area.getCentreY();

    p.startNewSubPath(x0, y0);

    for (size_t i = 0; i < scopeData.size(); ++i)
    {
        const float x = x0 + (float) i / (float) (scopeData.size() - 1) * w;
        const float y = y0 - scopeData[i] * (h * 0.42f);
        p.lineTo(x, y);
    }

    g.setColour(juce::Colour(0xff9ee7ff).withAlpha(0.14f));
    g.strokePath(p, juce::PathStrokeType(6.0f));

    g.setColour(juce::Colour(0xffd8f8ff));
    g.strokePath(p, juce::PathStrokeType(2.0f));
}

void ScopeComponent::timerCallback()
{
    processor.copyScopeData(scopeData);
    repaint();
}

BadlineDnBAudioProcessorEditor::BadlineDnBAudioProcessorEditor (BadlineDnBAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), scope(p)
{
    setSize(1280, 760);

    for (auto* b : { &soundBtn, &motionBtn, &toneBtn, &performBtn })
    {
        styleTab(*b);
        addAndMakeVisible(*b);
    }

    soundBtn.onClick   = [this] { switchPage(Page::Sound); };
    motionBtn.onClick  = [this] { switchPage(Page::Motion); };
    toneBtn.onClick    = [this] { switchPage(Page::Tone); };
    performBtn.onClick = [this] { switchPage(Page::Perform); };

    addAndMakeVisible(soundPage);
    addAndMakeVisible(motionPage);
    addAndMakeVisible(tonePage);
    addAndMakeVisible(performPage);

    soundPage.addAndMakeVisible(scope);

    for (auto* s : { &macro1, &macro2, &macro3, &macro4, &cutoff, &res, &drive, &glide })
    {
        setupKnob(*s);
        soundPage.addAndMakeVisible(*s);
    }

    playMode.addItem("Mono", 1);
    playMode.addItem("Legato", 2);
    playMode.addItem("Poly", 3);

    notePriority.addItem("Last", 1);
    notePriority.addItem("Low", 2);
    notePriority.addItem("High", 3);

    soundPage.addAndMakeVisible(playMode);
    soundPage.addAndMakeVisible(notePriority);

    macro1Att = std::make_unique<SliderAttachment>(audioProcessor.apvts, "macro1", macro1);
    macro2Att = std::make_unique<SliderAttachment>(audioProcessor.apvts, "macro2", macro2);
    macro3Att = std::make_unique<SliderAttachment>(audioProcessor.apvts, "macro3", macro3);
    macro4Att = std::make_unique<SliderAttachment>(audioProcessor.apvts, "macro4", macro4);

    cutoffAtt = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filter_cutoff", cutoff);
    resAtt    = std::make_unique<SliderAttachment>(audioProcessor.apvts, "filter_res", res);
    driveAtt  = std::make_unique<SliderAttachment>(audioProcessor.apvts, "dist_drive", drive);
    glideAtt  = std::make_unique<SliderAttachment>(audioProcessor.apvts, "glide_ms", glide);

    playModeAtt     = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "play_mode", playMode);
    notePriorityAtt = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "note_priority", notePriority);

    switchPage(Page::Sound);
    startTimerHz(30);
}

void BadlineDnBAudioProcessorEditor::setupKnob(juce::Slider& s)
{
    s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 66, 18);
    s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffd8f8ff));
    s.setColour(juce::Slider::thumbColourId, juce::Colour(0xff7ce6ff));
    s.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    s.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff151a21));
}

void BadlineDnBAudioProcessorEditor::styleTab(juce::TextButton& b)
{
    b.setClickingTogglesState(false);
    b.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff11161d));
    b.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff1e2a36));
    b.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.80f));
    b.setColour(juce::TextButton::textColourOnId, juce::Colour(0xffd8f8ff));
}

void BadlineDnBAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.fillAll(juce::Colour(0xff090b10));

    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff0d1015), 0.0f, 0.0f,
                                           juce::Colour(0xff151820), 0.0f, bounds.getBottom(), false));
    g.fillRoundedRectangle(bounds.reduced(10.0f), 28.0f);

    g.setColour(juce::Colour(0x33d8f8ff));
    g.drawRoundedRectangle(bounds.reduced(10.0f), 28.0f, 1.2f);

    g.setColour(juce::Colours::white.withAlpha(0.98f));
    g.setFont(juce::FontOptions(32.0f, juce::Font::bold));
    g.drawText("Blackside Bass", 28, 18, 340, 34, juce::Justification::left);

    g.setColour(juce::Colours::white.withAlpha(0.45f));
    g.setFont(juce::FontOptions(14.0f, juce::Font::plain));
    g.drawText("Live scope • sustain • mono / legato / poly", 31, 50, 380, 18, juce::Justification::left);

    auto drawMeter = [&](juce::Rectangle<float> area, float value, const juce::String& label, juce::Colour c)
    {
        g.setColour(juce::Colour(0xff121418));
        g.fillRoundedRectangle(area, 10.0f);
        g.setColour(juce::Colours::white.withAlpha(0.12f));
        g.drawRoundedRectangle(area, 10.0f, 1.0f);

        auto inner = area.reduced(5.0f);
        auto h = inner.getHeight() * juce::jlimit(0.0f, 1.0f, value);
        juce::Rectangle<float> fill(inner.getX(), inner.getBottom() - h, inner.getWidth(), h);
        g.setGradientFill(juce::ColourGradient(c.brighter(), fill.getCentreX(), fill.getBottom(),
                                               c.darker(0.6f), fill.getCentreX(), fill.getY(), false));
        g.fillRoundedRectangle(fill, 6.0f);

        g.setColour(juce::Colours::white.withAlpha(0.75f));
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.drawText(label,
                   juce::Rectangle<int>((int) area.getX() - 4, (int) area.getBottom() + 6,
                                        (int) area.getWidth() + 8, 14),
                   juce::Justification::centred, false);
    };

    drawMeter({ 1180.0f, 96.0f, 22.0f, 180.0f }, audioProcessor.getMeterLevel(), "OUT", juce::Colour(0xffd8f8ff));
    drawMeter({ 1210.0f, 96.0f, 22.0f, 180.0f }, audioProcessor.getSubMeterLevel(), "SUB", juce::Colour(0xff7ce6ff));
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

    soundPage.setBounds(content);
    motionPage.setBounds(content);
    tonePage.setBounds(content);
    performPage.setBounds(content);

    auto area = soundPage.getLocalBounds().reduced(18);

    auto top = area.removeFromTop(300);
    scope.setBounds(top.removeFromLeft(760).reduced(8));

    auto perf = top.reduced(8);
    playMode.setBounds(perf.removeFromTop(28));
    perf.removeFromTop(10);
    notePriority.setBounds(perf.removeFromTop(28));

    auto knobs = area.removeFromTop(220);
    auto place = [&](juce::Slider& s, int x)
    {
        s.setBounds(20 + x * 120, 0, 100, 120);
    };

    macro1.setBounds(20, 20, 100, 120);
    macro2.setBounds(140, 20, 100, 120);
    macro3.setBounds(260, 20, 100, 120);
    macro4.setBounds(380, 20, 100, 120);

    cutoff.setBounds(560, 20, 100, 120);
    res.setBounds(680, 20, 100, 120);
    drive.setBounds(800, 20, 100, 120);
    glide.setBounds(920, 20, 100, 120);
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
