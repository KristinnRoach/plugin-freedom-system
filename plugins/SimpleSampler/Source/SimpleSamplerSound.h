#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

/**
 * SimpleSamplerSound - Holds reference to externally loaded sample buffer
 *
 * Phase 4.1: Hardcoded sine wave sample (440Hz, 1 second)
 * Phase 4.2: Uses external sample buffer loaded from file (atomic pointer)
 */
class SimpleSamplerSound : public juce::SynthesiserSound
{
public:
    SimpleSamplerSound(double sampleRate = 44100.0);
    ~SimpleSamplerSound() override = default;

    // SynthesiserSound interface
    bool appliesToNote(int midiNoteNumber) override;
    bool appliesToChannel(int midiChannel) override;

    // Phase 4.2: Set external sample buffer (called from audio thread)
    void setSampleBuffer(const juce::AudioBuffer<float>* buffer) { externalSampleBuffer = buffer; }

    // Sample access
    const juce::AudioBuffer<float>* getSampleBuffer() const;
    double getSampleRate() const { return sampleRate; }

private:
    void loadHardcodedSample();

    juce::AudioBuffer<float> fallbackSampleBuffer;  // Hardcoded sine wave fallback
    const juce::AudioBuffer<float>* externalSampleBuffer = nullptr;  // Loaded from file
    double sampleRate;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleSamplerSound)
};
