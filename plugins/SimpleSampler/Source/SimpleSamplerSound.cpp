#include "SimpleSamplerSound.h"

SimpleSamplerSound::SimpleSamplerSound()
    : sampleRate(44100.0)
{
    loadHardcodedSample();
}

bool SimpleSamplerSound::appliesToNote(int midiNoteNumber)
{
    // Respond to all MIDI notes (0-127)
    juce::ignoreUnused(midiNoteNumber);
    return true;
}

bool SimpleSamplerSound::appliesToChannel(int midiChannel)
{
    // Omni mode: respond to all MIDI channels
    juce::ignoreUnused(midiChannel);
    return true;
}

const juce::AudioBuffer<float>* SimpleSamplerSound::getSampleBuffer() const
{
    // Phase 4.2: Return external buffer if available, otherwise fallback
    if (externalSampleBuffer != nullptr)
        return externalSampleBuffer;

    return &fallbackSampleBuffer;
}

void SimpleSamplerSound::loadHardcodedSample()
{
    // Generate 440Hz sine wave (1 second duration)
    const int duration = 1;  // 1 second
    const int numSamples = static_cast<int>(sampleRate * duration);
    const float frequency = 440.0f;  // A4

    // Allocate mono buffer
    fallbackSampleBuffer.setSize(1, numSamples);
    auto* data = fallbackSampleBuffer.getWritePointer(0);

    // Generate sine wave
    for (int i = 0; i < numSamples; ++i)
    {
        const float phase = 2.0f * juce::MathConstants<float>::pi * frequency * i / static_cast<float>(sampleRate);
        data[i] = std::sin(phase) * 0.5f;  // 0.5 amplitude to prevent clipping
    }
}
