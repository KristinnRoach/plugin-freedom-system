#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "SimpleSamplerSound.h"
#include "SimpleSamplerVoice.h"

class SimpleSamplerAudioProcessor : public juce::AudioProcessor
{
public:
    SimpleSamplerAudioProcessor();
    ~SimpleSamplerAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "SimpleSampler"; }
    bool acceptsMidi() const override { return true; }   // Instrument accepts MIDI
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

    // File loading (called from UI thread)
    void loadSampleFromFile(const juce::File& file);
    juce::String getCurrentSampleName() const { return currentSampleName; }

    juce::AudioProcessorValueTreeState parameters;

private:
    // DSP Components (BEFORE APVTS for initialization order)
    juce::Synthesiser synth;

    // File I/O System (Phase 4.2)
    juce::AudioFormatManager formatManager;
    std::atomic<juce::AudioBuffer<float>*> currentSampleBuffer { nullptr };
    juce::String currentSampleName;
    juce::String currentSamplePath;

    // Helper methods
    void loadSampleInBackground(const juce::File& file);
    void atomicSwapBuffer(juce::AudioBuffer<float>* newBuffer, const juce::String& sampleName, const juce::String& samplePath);

    // Parameter layout creation
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleSamplerAudioProcessor)
};
