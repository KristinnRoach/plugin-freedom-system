#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

/**
 * SimpleSamplerSound - Holds sample buffer and defines MIDI playback range
 *
 * Phase 4.1: Hardcoded sine wave sample (440Hz, 1 second)
 * Phase 4.2: Will be replaced with file-loaded sample
 */
class SimpleSamplerSound : public juce::SynthesiserSound
{
public:
    SimpleSamplerSound();
    ~SimpleSamplerSound() override = default;

    // SynthesiserSound interface
    bool appliesToNote(int midiNoteNumber) override;
    bool appliesToChannel(int midiChannel) override;

    // Sample access
    const juce::AudioBuffer<float>& getSampleBuffer() const { return sampleBuffer; }
    double getSampleRate() const { return sampleRate; }

private:
    void loadHardcodedSample();

    juce::AudioBuffer<float> sampleBuffer;
    double sampleRate;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleSamplerSound)
};
