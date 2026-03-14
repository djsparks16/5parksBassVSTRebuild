#include "PluginEditor.h"

namespace
{
    void drawPanel(juce::Graphics& g, juce::Rectangle<float> area, const juce::String& title, juce::Colour edge = juce::Colour(0x44a8eaff))
    {
        g.setColour(juce::Colour(0xff0f1b26));
        g.fillRoundedRectangle(area, 14.0f);
        g.setColour(edge);
        g.drawRoundedRectangle(area, 14.0f, 1.0f);
        g.setColour(juce::Colours::white.withAlpha(0.82f));
        g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
        g.drawText(title, area.toNearestInt().reduced(12, 8).removeFromTop(18), juce::Justification::centredLeft, false);
    }

    void drawMeter(juce::Graphics& g, juce::Rectangle<float> area, float value, const juce::String& label, juce::Colour colour)
    {
        g.setColour(juce::Colour(0xff10161d));
        g.fillRoundedRectangle(area, 10.0f);
        g.setColour(juce::Colours::white.withAlpha(0.12f));
        g.drawRoundedRectangle(area, 10.0f, 1.0f);
        const auto inner = area.reduced(4.0f);
        const float height = inner.getHeight() * juce::jlimit(0.0f, 1.0f, value);
        const juce::Rectangle<float> fill(inner.getX(), inner.getBottom() - height, inner.getWidth(), height);
        g.setGradientFill(juce::ColourGradient(colour.brighter(), fill.getCentreX(), fill.getBottom(), colour.darker(0.8f), fill.getCentreX(), fill.getY(), false));
        g.fillRoundedRectangle(fill, 7.0f);
        g.setColour(juce::Colours::white.withAlpha(0.72f));
        g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
        auto labelBounds = area.toNearestInt();
        labelBounds.setY(labelBounds.getBottom() + 4);
        labelBounds.setHeight(14);
        g.drawText(label, labelBounds, juce::Justification::centred, false);
    }

    void layoutKnobGrid(juce::Rectangle<int> area, std::initializer_list<LabeledKnob*> knobs, int columns)
    {
        if (knobs.size() == 0 || columns <= 0)
            return;
        constexpr int gap = 10;
        const int count = (int) knobs.size();
        const int rows = (count + columns - 1) / columns;
        const int cellW = (area.getWidth() - (columns - 1) * gap) / columns;
        const int cellH = (area.getHeight() - (rows - 1) * gap) / rows;
        int index = 0;
        for (auto* knob : knobs)
        {
            const int row = index / columns;
            const int col = index % columns;
            knob->setBounds(area.getX() + col * (cellW + gap), area.getY() + row * (cellH + gap), cellW, cellH);
            ++index;
        }
    }
}

LabeledKnob::LabeledKnob()
{
    addAndMakeVisible(slider);
    addAndMakeVisible(title);
    addAndMakeVisible(footer);

    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 54, 16);
    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff8fe7ff));
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0x33587d94));
    slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffb4f0ff));
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff15202b));

    for (auto* label : { &title, &footer })
    {
        label->setJustificationType(juce::Justification::centred);
        label->setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.88f));
    }
    title.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    footer.setFont(juce::FontOptions(9.0f, juce::Font::plain));
}

void LabeledKnob::resized()
{
    auto bounds = getLocalBounds();
    title.setBounds(bounds.removeFromTop(15));
    footer.setBounds(bounds.removeFromBottom(12));
    slider.setBounds(bounds.reduced(2));
}

void ScopeComponent::paint(juce::Graphics& g)
{
    const auto area = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff09131c));
    g.fillRoundedRectangle(area, 10.0f);
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawRoundedRectangle(area, 10.0f, 1.0f);
    juce::Path path;
    const float pad = 8.0f;
    const float width = area.getWidth() - pad * 2.0f;
    const float height = area.getHeight() - pad * 2.0f;
    const float centreY = area.getCentreY();
    path.startNewSubPath(area.getX() + pad, centreY);
    for (size_t i = 0; i < scopeData.size(); ++i)
    {
        const float x = area.getX() + pad + ((float) i / (float) (scopeData.size() - 1)) * width;
        const float y = centreY - scopeData[i] * height * 0.42f;
        path.lineTo(x, y);
    }
    g.setColour(juce::Colour(0xff66ffb0).withAlpha(0.14f));
    g.strokePath(path, juce::PathStrokeType(4.5f));
    g.setColour(juce::Colour(0xff7cfaa8));
    g.strokePath(path, juce::PathStrokeType(1.4f));
}

void ScopeComponent::timerCallback() { processor.copyScopeData(scopeData); repaint(); }

void SpectrumComponent::paint(juce::Graphics& g)
{
    const auto area = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff09131c));
    g.fillRoundedRectangle(area, 10.0f);
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawRoundedRectangle(area, 10.0f, 1.0f);
    const auto inner = area.reduced(10.0f);
    const float barW = inner.getWidth() / (float) bins.size();
    for (size_t i = 0; i < bins.size(); ++i)
    {
        const float h = bins[i] * inner.getHeight();
        juce::Rectangle<float> bar(inner.getX() + barW * (float) i, inner.getBottom() - h, juce::jmax(1.0f, barW - 1.0f), h);
        const auto colour = juce::Colour::fromHSV(0.35f - 0.25f * bins[i], 0.65f, 0.92f, 0.85f);
        g.setColour(colour);
        g.fillRect(bar);
    }
}

void SpectrumComponent::timerCallback() { processor.copyAnalyzerData(bins); repaint(); }

BadlineDnBAudioProcessorEditor::BadlineDnBAudioProcessorEditor(BadlineDnBAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), scope(p), spectrum(p)
{
    setSize(1500, 940);

    for (auto* button : { &oscBtn, &motionBtn, &toneBtn, &performBtn, &globalBtn })
    {
        styleTab(*button);
        addAndMakeVisible(*button);
    }
    oscBtn.onClick = [this] { switchPage(Page::Osc); };
    motionBtn.onClick = [this] { switchPage(Page::Motion); };
    toneBtn.onClick = [this] { switchPage(Page::Tone); };
    performBtn.onClick = [this] { switchPage(Page::Perform); };
    globalBtn.onClick = [this] { switchPage(Page::Global); };

    for (auto* page : { &oscPage, &motionPage, &tonePage, &performPage, &globalPage })
        addAndMakeVisible(*page);

    styleHeader(oscPageHeader, "OSC A / OSC B / WARHORN / SUB");
    styleHeader(motionPageHeader, "DRAWN MOD / BASSLINE MOTION / VOCALITY");
    styleHeader(tonePageHeader, "FILTER / DISTORTION / ANALYZER");
    styleHeader(performPageHeader, "PLAY MODES / GLIDE / ENVELOPES");
    styleHeader(globalPageHeader, "HARMONIES / WIDTH / FINAL STAGE");
    oscPage.addAndMakeVisible(oscPageHeader);
    motionPage.addAndMakeVisible(motionPageHeader);
    tonePage.addAndMakeVisible(tonePageHeader);
    performPage.addAndMakeVisible(performPageHeader);
    globalPage.addAndMakeVisible(globalPageHeader);
    oscPage.addAndMakeVisible(scope);
    tonePage.addAndMakeVisible(spectrum);

    addKnobToPage(oscPage, oscAWave, "OSC A WT", "main", "oscA_wave");
    addKnobToPage(oscPage, oscAWarp, "OSC A WARP", "teeth", "oscA_warp");
    addKnobToPage(oscPage, oscALevel, "OSC A LVL", "front", "oscA_level");
    addKnobToPage(oscPage, oscBWave, "OSC B WT", "counter", "oscB_wave");
    addKnobToPage(oscPage, oscBWarp, "OSC B WARP", "phase", "oscB_warp");
    addKnobToPage(oscPage, oscBLevel, "OSC B LVL", "pair", "oscB_level");
    addKnobToPage(oscPage, oscCWave, "WARHORN WT", "body", "oscC_wave");
    addKnobToPage(oscPage, oscCWarp, "WARHORN WRP", "brass", "oscC_warp");
    addKnobToPage(oscPage, oscCLevel, "OSC C LVL", "throat", "oscC_level");
    addKnobToPage(oscPage, warhornMix, "WARHORN", "epic", "warhorn_mix");
    addKnobToPage(oscPage, subLevel, "SUB", "mono", "sub_level");
    addKnobToPage(oscPage, subDrive, "SUB DRV", "crush", "sub_drive");
    addKnobToPage(oscPage, subOctave, "SUB OCT", "lower", "sub_octave");
    addKnobToPage(oscPage, subHarmony, "SUB HARM", "5ths", "sub_harmony");
    addKnobToPage(oscPage, noiseLevel, "NOISE", "air", "noise_level");
    addKnobToPage(oscPage, reeseDetune, "REESE DTN", "detune", "reese_detune");
    addKnobToPage(oscPage, reeseWidth, "REESE WID", "stereo", "reese_width");
    addKnobToPage(oscPage, reeseDrift, "REESE DRF", "motion", "reese_drift");
    addKnobToPage(oscPage, comboStack, "COMBINED", "stack", "combo_stack");

    addKnobToPage(motionPage, macro1, "MACRO 1", "motion", "macro1");
    addKnobToPage(motionPage, macro2, "MACRO 2", "growl", "macro2");
    addKnobToPage(motionPage, macro3, "MACRO 3", "voice", "macro3");
    addKnobToPage(motionPage, macro4, "MACRO 4", "drop", "macro4");
    addKnobToPage(motionPage, lfoRate, "LFO RATE", "sync", "lfo1_rate");
    addKnobToPage(motionPage, lfoAmt, "LFO AMT", "depth", "lfo1_amt");
    addKnobToPage(motionPage, envAmt, "ENV AMT", "pluck", "envamt");
    addKnobToPage(motionPage, wobbleAmt, "WOBBLE", "swing", "wobble_amt");
    addKnobToPage(motionPage, wompAmt, "WOMP", "donk", "womp_amt");
    addKnobToPage(motionPage, buzzAmt, "BUZZ", "virji", "buzz_amt");
    addKnobToPage(motionPage, talkAmt, "TALK", "vocal", "talk_amt");
    addKnobToPage(motionPage, biteAmt, "BITE", "attack", "bite_amt");
    addKnobToPage(motionPage, punchAmt, "PUNCH", "knock", "punch_amt");
    addKnobToPage(motionPage, hornBend, "HORN BEND", "rise", "horn_bend");
    addKnobToPage(motionPage, hornFormant, "FORMANT", "mouth", "horn_formant");
    addKnobToPage(motionPage, hornBody, "HORN BODY", "chest", "horn_body");
    addKnobToPage(motionPage, screechDrive, "SCREECH DRV", "edge", "screech_drive");
    addKnobToPage(motionPage, screechFm, "SCREECH FM", "tear", "screech_fm");
    addKnobToPage(motionPage, fmGrit, "FM GRIT", "wreck", "fm_grit");

    addKnobToPage(tonePage, cutoff, "F1 CUT", "main", "filter_cutoff");
    addKnobToPage(tonePage, res, "F1 RES", "peak", "filter_res");
    addKnobToPage(tonePage, filterDrive, "F1 DRIVE", "pre", "filter_drive");
    addKnobToPage(tonePage, filter2Cutoff, "F2 CUT", "talk", "filter2_cutoff");
    addKnobToPage(tonePage, filter2Res, "F2 RES", "formant", "filter2_res");
    addKnobToPage(tonePage, notchAmt, "NOTCH", "hollow", "notch_amt");
    addKnobToPage(tonePage, distDrive, "DIST DRV", "aggro", "dist_drive");
    addKnobToPage(tonePage, distMix, "DIST MIX", "parallel", "dist_mix");
    addKnobToPage(tonePage, outputClip, "CLIP", "master", "output_clip");

    addKnobToPage(globalPage, harmMix, "HARM MIX", "layer", "harm_mix");
    addKnobToPage(globalPage, harmInterval, "HARM INT", "semi", "harm_interval");
    addKnobToPage(globalPage, harmSpread, "HARM WID", "spread", "harm_spread");
    addKnobToPage(globalPage, airAmt, "AIR", "top", "air_amt");
    addKnobToPage(globalPage, stereoSpin, "STEREO", "sides", "stereo_spin");
    addKnobToPage(globalPage, monoBlend, "MONO", "club", "mono_blend");
    addKnobToPage(globalPage, masterGain, "MASTER", "out", "master_gain");

    addKnobToPage(performPage, glide, "GLIDE", "porta", "glide_ms");
    addKnobToPage(performPage, attack, "ATTACK", "rise", "envA");
    addKnobToPage(performPage, decay, "DECAY", "fall", "envD");
    addKnobToPage(performPage, sustain, "SUSTAIN", "hold", "envS");
    addKnobToPage(performPage, release, "RELEASE", "tail", "envR");

    styleHeader(playModeLabel, "PLAY MODE");
    styleHeader(notePriorityLabel, "VOICE STEAL");
    performPage.addAndMakeVisible(playModeLabel);
    performPage.addAndMakeVisible(notePriorityLabel);
    styleCombo(playMode);
    styleCombo(notePriority);
    performPage.addAndMakeVisible(playMode);
    performPage.addAndMakeVisible(notePriority);
    playMode.addItem("Mono", 1); playMode.addItem("Legato", 2); playMode.addItem("Poly", 3);
    notePriority.addItem("Last", 1); notePriority.addItem("Low", 2); notePriority.addItem("High", 3);
    playModeAtt = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "play_mode", playMode);
    notePriorityAtt = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "note_priority", notePriority);

    switchPage(Page::Osc);
    startTimerHz(20);
}

void BadlineDnBAudioProcessorEditor::styleTab(juce::TextButton& button)
{
    button.setClickingTogglesState(true);
    button.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff12202d));
    button.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff1b3f57));
    button.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.78f));
    button.setColour(juce::TextButton::textColourOnId, juce::Colour(0xffd8f8ff));
}

void BadlineDnBAudioProcessorEditor::styleCombo(juce::ComboBox& combo)
{
    combo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff11161d));
    combo.setColour(juce::ComboBox::outlineColourId, juce::Colour(0x44d8f8ff));
    combo.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    combo.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xffd8f8ff));
}

void BadlineDnBAudioProcessorEditor::styleHeader(juce::Label& label, const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.88f));
    label.setFont(juce::FontOptions(14.0f, juce::Font::bold));
}

void BadlineDnBAudioProcessorEditor::setupKnob(LabeledKnob& knob, const juce::String& header, const juce::String& footer)
{
    knob.title.setText(header, juce::dontSendNotification);
    knob.footer.setText(footer, juce::dontSendNotification);
}

void BadlineDnBAudioProcessorEditor::addKnobToPage(juce::Component& page, LabeledKnob& knob, const juce::String& title, const juce::String& footer, const juce::String& paramID)
{
    setupKnob(knob, title, footer);
    page.addAndMakeVisible(knob);
    attach(knob, paramID);
}

void BadlineDnBAudioProcessorEditor::attach(LabeledKnob& knob, const juce::String& paramID)
{
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, paramID, knob.slider));
}

juce::Component& BadlineDnBAudioProcessorEditor::pageComponent(Page p)
{
    switch (p)
    {
        case Page::Osc: return oscPage;
        case Page::Motion: return motionPage;
        case Page::Tone: return tonePage;
        case Page::Perform: return performPage;
        default: return globalPage;
    }
}

juce::String BadlineDnBAudioProcessorEditor::pageTitle(Page p) const
{
    switch (p)
    {
        case Page::Osc: return "OSCILLATORS / WARHORN / REESE / SUB";
        case Page::Motion: return "MOD LANES / BASSLINE MOTION / CHARACTER";
        case Page::Tone: return "FILTER / DISTORTION / ANALYZER";
        case Page::Perform: return "VOICING / GLIDE / ENVELOPE";
        default: return "STACK / HARMONY / OUTPUT";
    }
}

void BadlineDnBAudioProcessorEditor::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    g.fillAll(juce::Colour(0xff071018));
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff08121b), 0.0f, 0.0f, juce::Colour(0xff143247), 0.0f, bounds.getBottom(), false));
    g.fillRoundedRectangle(bounds.reduced(8.0f), 24.0f);
    g.setColour(juce::Colour(0x338fe7ff));
    g.drawRoundedRectangle(bounds.reduced(8.0f), 24.0f, 1.2f);

    g.setColour(juce::Colours::white.withAlpha(0.98f));
    g.setFont(juce::FontOptions(32.0f, juce::Font::bold));
    g.drawText("Blackside Bass X", 26, 16, 360, 34, juce::Justification::centredLeft, false);
    g.setColour(juce::Colours::white.withAlpha(0.46f));
    g.setFont(juce::FontOptions(13.0f, juce::Font::plain));
    g.drawText("level 11 engine :: warhorn, reese, womp, sub harmonies and combo stacks", 28, 48, 620, 18, juce::Justification::centredLeft, false);

    auto content = pageComponent(currentPage).getBounds().toFloat();
    drawPanel(g, content, pageTitle(currentPage), juce::Colour(0x559fe7ff));
    drawMeter(g, { 1416.0f, 24.0f, 20.0f, 88.0f }, audioProcessor.getMeterLevel(), "OUT", juce::Colour(0xffb4f0ff));
    drawMeter(g, { 1444.0f, 24.0f, 20.0f, 88.0f }, audioProcessor.getSubMeterLevel(), "SUB", juce::Colour(0xff69f0b0));

    if (currentPage == Page::Osc)
    {
        const auto page = oscPage.getBounds().reduced(32);
        drawPanel(g, juce::Rectangle<float>((float) page.getX(), (float) page.getY() + 42.0f, 300.0f, 240.0f), "OSC A", juce::Colour(0x44ff5faf));
        drawPanel(g, juce::Rectangle<float>((float) page.getX() + 314.0f, (float) page.getY() + 42.0f, 300.0f, 240.0f), "OSC B", juce::Colour(0x44ff5faf));
        drawPanel(g, juce::Rectangle<float>((float) page.getX() + 628.0f, (float) page.getY() + 42.0f, 300.0f, 240.0f), "WARHORN / REESE", juce::Colour(0x44ff5faf));
        drawPanel(g, juce::Rectangle<float>((float) page.getRight() - 210.0f, (float) page.getY() + 42.0f, 180.0f, 240.0f), "SUB", juce::Colour(0x44ff5faf));
        drawPanel(g, juce::Rectangle<float>((float) page.getX(), (float) page.getBottom() - 220.0f, (float) page.getWidth() - 30.0f, 160.0f), "COMBO LAYER / MOD FEEL", juce::Colour(0x44a8eaff));
    }
}

void BadlineDnBAudioProcessorEditor::resized()
{
    constexpr int topY = 82, navH = 34, navW = 118;
    oscBtn.setBounds(26, topY, navW, navH);
    motionBtn.setBounds(152, topY, navW, navH);
    toneBtn.setBounds(278, topY, navW, navH);
    performBtn.setBounds(404, topY, navW, navH);
    globalBtn.setBounds(530, topY, navW, navH);

    auto content = getLocalBounds().reduced(22);
    content.removeFromTop(124);
    for (auto* page : { &oscPage, &motionPage, &tonePage, &performPage, &globalPage })
        page->setBounds(content);

    auto oscArea = oscPage.getLocalBounds().reduced(18);
    oscPageHeader.setBounds(oscArea.removeFromTop(22));
    oscArea.removeFromTop(34);
    auto topCards = oscArea.removeFromTop(250);
    auto oscABox = topCards.removeFromLeft(300); topCards.removeFromLeft(14);
    auto oscBBox = topCards.removeFromLeft(300); topCards.removeFromLeft(14);
    auto hornBox = topCards.removeFromLeft(300); topCards.removeFromLeft(14);
    auto subBox = topCards.removeFromLeft(200);
    layoutKnobGrid(oscABox.reduced(14, 34), { &oscAWave, &oscAWarp, &oscALevel }, 3);
    layoutKnobGrid(oscBBox.reduced(14, 34), { &oscBWave, &oscBWarp, &oscBLevel }, 3);
    layoutKnobGrid(hornBox.reduced(14, 34), { &oscCWave, &oscCWarp, &oscCLevel, &warhornMix, &reeseDetune, &reeseWidth }, 3);
    layoutKnobGrid(subBox.reduced(12, 34), { &subLevel, &subDrive, &subOctave, &subHarmony }, 2);
    oscArea.removeFromTop(12);
    auto midArea = oscArea.removeFromTop(160);
    scope.setBounds(midArea.removeFromLeft(650).reduced(6));
    layoutKnobGrid(midArea.reduced(4), { &noiseLevel, &reeseDrift, &comboStack }, 3);
    oscArea.removeFromTop(12);
    layoutKnobGrid(oscArea.removeFromTop(180), { &comboStack, &reeseDetune, &reeseWidth, &reeseDrift, &warhornMix, &subHarmony }, 6);

    auto motionArea = motionPage.getLocalBounds().reduced(18);
    motionPageHeader.setBounds(motionArea.removeFromTop(22)); motionArea.removeFromTop(10);
    layoutKnobGrid(motionArea.removeFromTop(520), { &macro1, &macro2, &macro3, &macro4, &lfoRate, &lfoAmt, &envAmt, &wobbleAmt, &wompAmt, &buzzAmt, &talkAmt, &biteAmt, &punchAmt, &hornBend, &hornFormant, &hornBody, &screechDrive, &screechFm, &fmGrit }, 5);

    auto toneArea = tonePage.getLocalBounds().reduced(18);
    tonePageHeader.setBounds(toneArea.removeFromTop(22)); toneArea.removeFromTop(10);
    auto toneTop = toneArea.removeFromTop(320);
    spectrum.setBounds(toneTop.removeFromLeft(560).reduced(6));
    layoutKnobGrid(toneTop.reduced(4), { &cutoff, &res, &filterDrive, &filter2Cutoff, &filter2Res, &notchAmt, &distDrive, &distMix, &outputClip }, 3);

    auto performArea = performPage.getLocalBounds().reduced(18);
    performPageHeader.setBounds(performArea.removeFromTop(22)); performArea.removeFromTop(10);
    auto comboRow = performArea.removeFromTop(34);
    playModeLabel.setBounds(comboRow.removeFromLeft(120));
    playMode.setBounds(comboRow.removeFromLeft(160));
    comboRow.removeFromLeft(24);
    notePriorityLabel.setBounds(comboRow.removeFromLeft(120));
    notePriority.setBounds(comboRow.removeFromLeft(160));
    performArea.removeFromTop(18);
    layoutKnobGrid(performArea.removeFromTop(180), { &glide, &attack, &decay, &sustain, &release }, 5);

    auto globalArea = globalPage.getLocalBounds().reduced(18);
    globalPageHeader.setBounds(globalArea.removeFromTop(22)); globalArea.removeFromTop(10);
    layoutKnobGrid(globalArea.removeFromTop(280), { &harmMix, &harmInterval, &harmSpread, &subHarmony, &airAmt, &stereoSpin, &monoBlend, &masterGain }, 4);
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
    repaint();
}

void BadlineDnBAudioProcessorEditor::timerCallback() { repaint(); }
