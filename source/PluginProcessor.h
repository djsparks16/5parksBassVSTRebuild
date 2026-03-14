#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>
#include <vector>

#include "SynthEngine.h"

class BadlineDnBAudioProcessor : public juce::AudioProcessor
{
public:
    BadlineDnBAudioProcessor();
    ~BadlineDnBAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    float getMeterLevel() const noexcept { return meterLevel.load(); }
    float getSubMeterLevel() const noexcept { return subMeterLevel.load(); }
    void copyScopeData(std::array<float, 512>& dest) const noexcept;
    void copyAnalyzerData(std::array<float, 128>& dest) const noexcept;

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

private:
    enum class PlayMode { mono = 0, legato = 1, poly = 2 };
    enum class NotePriority { last = 0, low = 1, high = 2 };

    static constexpr int maxVoices = 12;
    std::array<BlacksideVoice, maxVoices> voices {};

    std::array<bool, 128> keyDown {};
    std::array<bool, 128> sustained {};
    std::array<float, 128> noteVelocity {};
    std::vector<int> monoStack;

    bool sustainPedalDown = false;
    uint64_t voiceCounter = 1;

    std::atomic<float> meterLevel { 0.0f };
    std::atomic<float> subMeterLevel { 0.0f };

    std::array<float, 2048> scopeRing {};
    std::atomic<int> scopeWritePos { 0 };

    std::array<float, 1024> fftFifo {};
    int fftFifoIndex = 0;
    juce::dsp::FFT fft { 10 };
    juce::dsp::WindowingFunction<float> window { 1024, juce::dsp::WindowingFunction<float>::hann };
    std::array<float, 2048> fftData {};
    std::array<float, 128> analyzerBins {};

    void handleNoteOn(int note, float velocity, PlayMode mode);
    void handleNoteOff(int note, PlayMode mode);
    void handleSustainPedal(bool down, PlayMode mode);

    void startMonoVoice(int note, float velocity, bool retrigger);
    void retriggerMonoFromStack();
    void releaseMonoIfIdle();
    void removeNoteFromMonoStack(int note);
    void pruneReleasedMonoNotes();

    int findFreeVoice() const;
    int findVoiceForNote(int note) const;
    int stealVoice(NotePriority priority) const;
    void startPolyVoice(int note, float velocity, NotePriority priority);
    void releasePolyNote(int note);

    void pushScopeSample(float s) noexcept;
    void pushAnalyzerSample(float s) noexcept;
    void computeAnalyzerFrame() noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BadlineDnBAudioProcessor)
};
