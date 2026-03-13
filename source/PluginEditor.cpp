#include "PluginEditor.h"

namespace
{
    void drawPanel(juce::Graphics& g, juce::Rectangle<float> r, const juce::String& title)
    {
        g.setColour(juce::Colour(0xff11161d));
        g.fillRoundedRectangle(r, 12.0f);
        g.setColour(juce::Colour(0x33d8f8ff));
        g.drawRoundedRectangle(r, 12.0f, 1.0f);
        g.setColour(juce::Colours::white.withAlpha(0.82f));
        g.setFont(juce::FontOptions(14.0f, juce::Font::bold));

        const auto titleArea = juce::Rectangle<int>(
            juce::roundToInt(r.getX() + 10.0f),
            juce::roundToInt(r.getY() + 6.0f),
            juce::roundToInt(r.getWidth() - 20.0f),
            18);

        g.drawText(title, titleArea, juce::Justification::left, false);
    }

    void addLabelStyle(juce::Label& label, float size)
    {
        label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.86f));
        label.setFont(juce::FontOptions(size, juce::Font::bold));
    }
}

LabeledKnob::LabeledKnob()
{
    addAndMakeVisible(slider);
    addAndMakeVisible(title);
    addAndMakeVisible(footer);

    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 58, 18);
    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffd8f8ff));
    slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff7ce6ff));
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff151a21));

    for (auto* l : { &title, &footer })
    {
        l->setJustificationType(juce::Justification::centred);
        l->setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.86f));
    }

    title.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    footer.setFont(juce::FontOptions(10.0f, juce::Font::plain));
}

void LabeledKnob::resized()
{
    auto r = getLocalBounds();
    title.setBounds(r.removeFromTop(16));
    footer.setBounds(r.removeFromBottom(14));
    slider.setBounds(r.reduced(2));
}

void ScopeComponent::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff0f1318));
    g.fillRoundedRectangle(area, 12.0f);
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawRoundedRectangle(area, 12.0f, 1.0f);

    juce::Path p;
    const auto w = area.getWidth() - 20.0f;
    const auto h = area.getHeight() - 20.0f;
    const auto x0 = area.getX() + 10.0f;
    const auto y0 = area.getCentreY();
    p.startNewSubPath(x0, y0);

    for (size_t i = 0; i < scopeData.size(); ++i)
    {
        const float x = x0 + static_cast<float>(i) / static_cast<float>(scopeData.size() - 1) * w;
        const float y = y0 - scopeData[i] * (h * 0.42f);
        p.lineTo(x, y);
    }

    g.setColour(juce::Colour(0xff9ee7ff).withAlpha(0.14f));
    g.strokePath(p, juce::PathStrokeType(5.0f));
    g.setColour(juce::Colour(0xffd8f8ff));
    g.strokePath(p, juce::PathStrokeType(1.6f));
}

void ScopeComponent::timerCallback()
{
    processor.copyScopeData(scopeData);
    repaint();
}

void SpectrumComponent::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff0f1318));
    g.fillRoundedRectangle(area, 12.0f);
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawRoundedRectangle(area, 12.0f, 1.0f);

    auto inner = area.reduced(10.0f);
    const float barW = inner.getWidth() / static_cast<float>(bins.size());

    for (size_t i = 0; i < bins.size(); ++i)
    {
        const float h = bins[i] * inner.getHeight();
        juce::Rectangle<float> bar(inner.getX() + barW * static_cast<float>(i),
                                   inner.getBottom() - h,
                                   juce::jmax(1.0f, barW - 1.0f),
                                   h);
        auto c = juce::Colour::fromHSV(0.54f - 0.15f * bins[i], 0.55f, 0.95f, 0.8f);
        g.setColour(c);
        g.fillRect(bar);
    }
}

void SpectrumComponent::timerCallback()
{
    processor.copyAnalyzerData(bins);
    repaint();
}

BadlineDnBAudioProcessorEditor::BadlineDnBAudioProcessorEditor(BadlineDnBAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), scope(p), spectrum(p)
{
    setSize(1440, 900);

    for (auto* button : { &oscBtn, &motionBtn, &toneBtn, &performBtn, &globalBtn })
    {
        styleTab(*button);
        addAndMakeVisible(*button);
    }

    oscBtn.onClick     = [this] { switchPage(Page::Osc); };
    motionBtn.onClick  = [this] { switchPage(Page::Motion); };
    toneBtn.onClick    = [this] { switchPage(Page::Tone); };
    performBtn.onClick = [this] { switchPage(Page::Perform); };
    globalBtn.onClick  = [this] { switchPage(Page::Global); };

    for (auto* c : { &oscPage, &motionPage, &tonePage, &performPage, &globalPage })
        addAndMakeVisible(*c);

    oscPage.addAndMakeVisible(scope);
    tonePage.addAndMakeVisible(spectrum);

    oscPageHeader.setText("OSCILLATORS & BASS ENGINES", juce::dontSendNotification);
    motionPageHeader.setText("MOTION, MACROS & CHARACTER", juce::dontSendNotification);
    tonePageHeader.setText("FILTERS, DISTORTION & ANALYZER", juce::dontSendNotification);
    performPageHeader.setText("PLAYABILITY, GLIDE & PRIORITY", juce::dontSendNotification);
    globalPageHeader.setText("MASTER OUTPUT & GLOBAL SHAPING", juce::dontSendNotification);
    playModeLabel.setText("PLAY MODE", juce::dontSendNotification);
    notePriorityLabel.setText("NOTE PRIORITY", juce::dontSendNotification);

    for (auto* l : { &oscPageHeader, &motionPageHeader, &tonePageHeader, &performPageHeader, &globalPageHeader })
        addLabelStyle(*l, 15.0f);
    addLabelStyle(playModeLabel, 14.0f);
    addLabelStyle(notePriorityLabel, 14.0f);

    oscPage.addAndMakeVisible(oscPageHeader);
    motionPage.addAndMakeVisible(motionPageHeader);
    tonePage.addAndMakeVisible(tonePageHeader);
    performPage.addAndMakeVisible(performPageHeader);
    globalPage.addAndMakeVisible(globalPageHeader);
    performPage.addAndMakeVisible(playModeLabel);
    performPage.addAndMakeVisible(notePriorityLabel);

    for (auto* k : { &oscAWave, &oscAWarp, &oscBWave, &oscBWarp, &oscCWave, &oscCWarp,
                     &subLevel, &subDrive, &subOctave, &noiseLevel,
                     &reeseDetune, &reeseWidth, &hornBend, &hornFormant })
        oscPage.addAndMakeVisible(*k);

    for (auto* k : { &macro1, &macro2, &macro3, &macro4, &lfoRate, &lfoAmt, &envAmt, &harmMix })
        motionPage.addAndMakeVisible(*k);

    for (auto* k : { &cutoff, &res, &filterDrive, &filter2Cutoff, &filter2Res,
                     &distDrive, &distMix, &screechDrive })
        tonePage.addAndMakeVisible(*k);

    performPage.addAndMakeVisible(playMode);
    performPage.addAndMakeVisible(notePriority);
    performPage.addAndMakeVisible(glide);

    for (auto* k : { &masterGain, &outputClip })
        globalPage.addAndMakeVisible(*k);

    setupKnob(oscAWave,     "OSC A WAVE",   "Primary texture");
    setupKnob(oscAWarp,     "OSC A WARP",   "Bend / metal");
    setupKnob(oscBWave,     "OSC B WAVE",   "Secondary texture");
    setupKnob(oscBWarp,     "OSC B WARP",   "Growl offset");
    setupKnob(oscCWave,     "OSC C WAVE",   "Mid support");
    setupKnob(oscCWarp,     "OSC C WARP",   "Vocal smear");
    setupKnob(subLevel,     "SUB LEVEL",    "Low-end power");
    setupKnob(subDrive,     "SUB DRIVE",    "Sub saturation");
    setupKnob(subOctave,    "SUB OCTAVE",   "Extra depth");
    setupKnob(noiseLevel,   "NOISE",        "Sizzle layer");
    setupKnob(reeseDetune,  "REESE DETUNE", "Width of stack");
    setupKnob(reeseWidth,   "REESE WIDTH",  "Stereo body");
    setupKnob(hornBend,     "HORN BEND",    "Warhorn pitch");
    setupKnob(hornFormant,  "FORMANT",      "Throat shape");

    setupKnob(macro1,       "MACRO 1",      "Reese motion");
    setupKnob(macro2,       "MACRO 2",      "Growl contour");
    setupKnob(macro3,       "MACRO 3",      "Harmonic shape");
    setupKnob(macro4,       "MACRO 4",      "Tone sweep");
    setupKnob(lfoRate,      "LFO RATE",     "Motion speed");
    setupKnob(lfoAmt,       "LFO AMOUNT",   "Modulation depth");
    setupKnob(envAmt,       "ENV AMOUNT",   "Envelope punch");
    setupKnob(harmMix,      "HARMONY MIX",  "Intervals blend");

    setupKnob(cutoff,       "FILTER 1 CUT", "Main movement");
    setupKnob(res,          "FILTER 1 RES", "Vocal edge");
    setupKnob(filterDrive,  "FILTER DRIVE", "Pre-drive");
    setupKnob(filter2Cutoff,"FILTER 2 CUT", "Band sculpt");
    setupKnob(filter2Res,   "FILTER 2 RES", "Band focus");
    setupKnob(distDrive,    "DIST DRIVE",   "Output aggression");
    setupKnob(distMix,      "DIST MIX",     "Clean / dirty");
    setupKnob(screechDrive, "SCREECH",      "Metal edge");

    setupKnob(glide,        "GLIDE",        "Portamento time");
    setupKnob(masterGain,   "MASTER",       "Final output");
    setupKnob(outputClip,   "SOFT CLIP",    "Ceiling");

    styleCombo(playMode);
    styleCombo(notePriority);
    playMode.addItem("Mono", 1);
    playMode.addItem("Legato", 2);
    playMode.addItem("Poly", 3);
    notePriority.addItem("Last", 1);
    notePriority.addItem("Low", 2);
    notePriority.addItem("High", 3);

    attach(oscAWave, "oscA_wave");
    attach(oscAWarp, "oscA_warp");
    attach(oscBWave, "oscB_wave");
    attach(oscBWarp, "oscB_warp");
    attach(oscCWave, "oscC_wave");
    attach(oscCWarp, "oscC_warp");
    attach(subLevel, "sub_level");
    attach(subDrive, "sub_drive");
    attach(subOctave, "sub_octave");
    attach(noiseLevel, "noise_level");
    attach(reeseDetune, "reese_detune");
    attach(reeseWidth, "reese_width");
    attach(hornBend, "horn_bend");
    attach(hornFormant, "horn_formant");

    attach(macro1, "macro1");
    attach(macro2, "macro2");
    attach(macro3, "macro3");
    attach(macro4, "macro4");
    attach(lfoRate, "lfo1_rate");
    attach(lfoAmt, "lfo1_amt");
    attach(envAmt, "envamt");
    attach(harmMix, "harm_mix");

    attach(cutoff, "filter_cutoff");
    attach(res, "filter_res");
    attach(filterDrive, "filter_drive");
    attach(filter2Cutoff, "filter2_cutoff");
    attach(filter2Res, "filter2_res");
    attach(distDrive, "dist_drive");
    attach(distMix, "dist_mix");
    attach(screechDrive, "screech_drive");

    attach(glide, "glide_ms");
    attach(masterGain, "master_gain");
    attach(outputClip, "output_clip");

    playModeAtt     = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "play_mode", playMode);
    notePriorityAtt = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "note_priority", notePriority);

    switchPage(Page::Osc);
    startTimerHz(24);
}

void BadlineDnBAudioProcessorEditor::setupKnob(LabeledKnob& k, const juce::String& header, const juce::String& footer)
{
    k.title.setText(header, juce::dontSendNotification);
    k.footer.setText(footer, juce::dontSendNotification);
}

void BadlineDnBAudioProcessorEditor::styleTab(juce::TextButton& b)
{
    b.setClickingTogglesState(false);
    b.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff11161d));
    b.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff233244));
    b.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.78f));
    b.setColour(juce::TextButton::textColourOnId, juce::Colour(0xffd8f8ff));
}

void BadlineDnBAudioProcessorEditor::styleCombo(juce::ComboBox& c)
{
    c.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff11161d));
    c.setColour(juce::ComboBox::outlineColourId, juce::Colour(0x33d8f8ff));
    c.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    c.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xffd8f8ff));
}

void BadlineDnBAudioProcessorEditor::attach(LabeledKnob& knob, const juce::String& paramID)
{
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, paramID, knob.slider));
}

void BadlineDnBAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.fillAll(juce::Colour(0xff090b10));
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff0c1015), 0.0f, 0.0f,
                                           juce::Colour(0xff171b24), 0.0f, bounds.getBottom(), false));
    g.fillRoundedRectangle(bounds.reduced(10.0f), 26.0f);
    g.setColour(juce::Colour(0x33d8f8ff));
    g.drawRoundedRectangle(bounds.reduced(10.0f), 26.0f, 1.2f);

    g.setColour(juce::Colours::white.withAlpha(0.98f));
    g.setFont(juce::FontOptions(34.0f, juce::Font::bold));
    g.drawText("Blackside Bass", 24, 14, 340, 36, juce::Justification::left);
    g.setColour(juce::Colours::white.withAlpha(0.42f));
    g.setFont(juce::FontOptions(14.0f, juce::Font::plain));
    g.drawText("5parks Land flagship bass workstation", 28, 50, 420, 20, juce::Justification::left);

    drawPanel(g, oscPage.getBounds().toFloat(), "OSC / SCOPE / ENGINES");
    drawPanel(g, motionPage.getBounds().toFloat(), "MOTION / MACROS / CHARACTER");
    drawPanel(g, tonePage.getBounds().toFloat(), "TONE / FILTERS / SPECTRUM");
    drawPanel(g, performPage.getBounds().toFloat(), "PERFORM / NOTE MODES / PLAYABILITY");
    drawPanel(g, globalPage.getBounds().toFloat(), "GLOBAL / MASTER / DEPTH");

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
        g.setColour(juce::Colours::white.withAlpha(0.72f));
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.drawText(label,
                   juce::Rectangle<int>(static_cast<int>(area.getX()) - 4,
                                        static_cast<int>(area.getBottom()) + 6,
                                        static_cast<int>(area.getWidth()) + 8,
                                        14),
                   juce::Justification::centred,
                   false);
    };

    drawMeter({ 1360.0f, 90.0f, 20.0f, 160.0f }, audioProcessor.getMeterLevel(), "OUT", juce::Colour(0xffd8f8ff));
    drawMeter({ 1388.0f, 90.0f, 20.0f, 160.0f }, audioProcessor.getSubMeterLevel(), "SUB", juce::Colour(0xff7ce6ff));
}

void BadlineDnBAudioProcessorEditor::resized()
{
    const int topY = 84;
    const int navH = 32;
    const int navW = 102;
    oscBtn.setBounds(24, topY, navW, navH);
    motionBtn.setBounds(134, topY, navW, navH);
    toneBtn.setBounds(244, topY, navW, navH);
    performBtn.setBounds(354, topY, navW, navH);
    globalBtn.setBounds(464, topY, navW, navH);

    auto content = getLocalBounds().reduced(18);
    content.removeFromTop(126);

    auto topRow = content.removeFromTop(310);
    auto bottomRow = content.removeFromTop(250);

    oscPage.setBounds(topRow.removeFromLeft(800).reduced(6));
    motionPage.setBounds(topRow.removeFromLeft(300).reduced(6));
    tonePage.setBounds(topRow.reduced(6));

    performPage.setBounds(bottomRow.removeFromLeft(700).reduced(6));
    globalPage.setBounds(bottomRow.reduced(6));

    auto oscArea = oscPage.getLocalBounds().reduced(16);
    oscPageHeader.setBounds(oscArea.removeFromTop(20));
    auto oscTop = oscArea.removeFromTop(168);
    scope.setBounds(oscTop.removeFromLeft(430).reduced(6));

    const int oscKnobW = 86;
    oscAWave.setBounds(oscTop.removeFromLeft(oscKnobW));
    oscAWarp.setBounds(oscTop.removeFromLeft(oscKnobW));
    oscBWave.setBounds(oscTop.removeFromLeft(oscKnobW));
    oscBWarp.setBounds(oscTop.removeFromLeft(oscKnobW));

    auto oscMid = oscArea.removeFromTop(108);
    oscCWave.setBounds(oscMid.removeFromLeft(86));
    oscCWarp.setBounds(oscMid.removeFromLeft(86));
    subLevel.setBounds(oscMid.removeFromLeft(86));
    subDrive.setBounds(oscMid.removeFromLeft(86));
    noiseLevel.setBounds(oscMid.removeFromLeft(86));

    auto oscBottom = oscArea.removeFromTop(108);
    reeseDetune.setBounds(oscBottom.removeFromLeft(92));
    reeseWidth.setBounds(oscBottom.removeFromLeft(92));
    hornBend.setBounds(oscBottom.removeFromLeft(92));
    hornFormant.setBounds(oscBottom.removeFromLeft(92));
    subOctave.setBounds(oscBottom.removeFromLeft(92));

    auto motArea = motionPage.getLocalBounds().reduced(16);
    motionPageHeader.setBounds(motArea.removeFromTop(20));
    auto motTop = motArea.removeFromTop(120);
    macro1.setBounds(motTop.removeFromLeft(64));
    macro2.setBounds(motTop.removeFromLeft(64));
    macro3.setBounds(motTop.removeFromLeft(64));
    macro4.setBounds(motTop.removeFromLeft(64));

    auto motBottom = motArea.removeFromTop(120);
    lfoRate.setBounds(motBottom.removeFromLeft(90));
    lfoAmt.setBounds(motBottom.removeFromLeft(90));
    envAmt.setBounds(motBottom.removeFromLeft(90));
    harmMix.setBounds(motBottom.removeFromLeft(90));

    auto toneArea = tonePage.getLocalBounds().reduced(16);
    tonePageHeader.setBounds(toneArea.removeFromTop(20));
    auto toneTop = toneArea.removeFromTop(170);
    spectrum.setBounds(toneTop.removeFromLeft(250).reduced(6));
    cutoff.setBounds(toneTop.removeFromLeft(86));
    res.setBounds(toneTop.removeFromLeft(86));
    filterDrive.setBounds(toneTop.removeFromLeft(86));
    filter2Cutoff.setBounds(toneTop.removeFromLeft(86));

    auto toneBottom = toneArea.removeFromTop(108);
    filter2Res.setBounds(toneBottom.removeFromLeft(86));
    distDrive.setBounds(toneBottom.removeFromLeft(86));
    distMix.setBounds(toneBottom.removeFromLeft(86));
    screechDrive.setBounds(toneBottom.removeFromLeft(86));

    auto perfArea = performPage.getLocalBounds().reduced(16);
    performPageHeader.setBounds(perfArea.removeFromTop(20));
    auto perfTop = perfArea.removeFromTop(28);
    playModeLabel.setBounds(perfTop.removeFromLeft(110));
    playMode.setBounds(perfTop.removeFromLeft(170));
    perfTop.removeFromLeft(20);
    notePriorityLabel.setBounds(perfTop.removeFromLeft(120));
    notePriority.setBounds(perfTop.removeFromLeft(170));

    auto perfKnobs = perfArea.removeFromTop(140);
    glide.setBounds(perfKnobs.removeFromLeft(110));

    auto globalArea = globalPage.getLocalBounds().reduced(16);
    globalPageHeader.setBounds(globalArea.removeFromTop(20));
    auto glob = globalArea.removeFromTop(140);
    masterGain.setBounds(glob.removeFromLeft(110));
    outputClip.setBounds(glob.removeFromLeft(110));
}

void BadlineDnBAudioProcessorEditor::switchPage(Page newPage)
{
    currentPage = newPage;
    oscPage.setVisible(newPage == Page::Osc);
    motionPage.setVisible(newPage == Page::Motion);
    tonePage.setVisible(newPage == Page::Tone);
    performPage.setVisible(newPage == Page::Perform);
    globalPage.setVisible(newPage == Page::Global);

    oscBtn.setToggleState(newPage == Page::Osc, juce::dontSendNotification);
    motionBtn.setToggleState(newPage == Page::Motion, juce::dontSendNotification);
    toneBtn.setToggleState(newPage == Page::Tone, juce::dontSendNotification);
    performBtn.setToggleState(newPage == Page::Perform, juce::dontSendNotification);
    globalBtn.setToggleState(newPage == Page::Global, juce::dontSendNotification);
}

void BadlineDnBAudioProcessorEditor::timerCallback()
{
    repaint();
}
