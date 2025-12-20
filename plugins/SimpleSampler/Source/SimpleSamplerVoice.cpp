#include "SimpleSamplerVoice.h"

bool SimpleSamplerVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<SimpleSamplerSound*>(sound) != nullptr;
}

void SimpleSamplerVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition)
{
    juce::ignoreUnused(currentPitchWheelPosition);

    // Get sample buffer from sound (Phase 4.2: returns pointer)
    auto* samplerSound = dynamic_cast<SimpleSamplerSound*>(sound);
    if (samplerSound == nullptr)
        return;

    currentSampleBuffer = samplerSound->getSampleBuffer();
    currentSampleRate = samplerSound->getSampleRate();

    // Reset playback position
    playbackPosition = 0.0;

    // Store MIDI note for pitch recalculation (when tuning parameter changes)
    currentMidiNote = midiNoteNumber;

    // Calculate pitch-shifting playback rate
    // Root note: C3 (MIDI 60) = 1.0x playback rate
    // Formula: playbackRate = pow(2.0, semitoneOffset / 12.0)
    // Phase 4.3: Integrate tuning parameter
    const float semitoneOffset = static_cast<float>(midiNoteNumber - 60) + tuningParameter;
    playbackRate = std::pow(2.0, semitoneOffset / 12.0);

    // Store velocity for volume calculation
    velocityGain = velocity;  // Already normalized 0.0-1.0 by JUCE

    // Note is now active - voice will render until sample ends or stopNote() called
}

void SimpleSamplerVoice::stopNote(float velocity, bool allowTailOff)
{
    juce::ignoreUnused(velocity, allowTailOff);

    // Immediate stop (no envelope in Phase 4.1)
    clearCurrentNote();
    currentSampleBuffer = nullptr;
}

void SimpleSamplerVoice::pitchWheelMoved(int newPitchWheelValue)
{
    juce::ignoreUnused(newPitchWheelValue);
    // Not implemented in Phase 4.1
}

void SimpleSamplerVoice::controllerMoved(int controllerNumber, int newControllerValue)
{
    juce::ignoreUnused(controllerNumber, newControllerValue);
    // Not implemented in Phase 4.1
}

void SimpleSamplerVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (currentSampleBuffer == nullptr || currentSampleBuffer->getNumSamples() == 0)
    {
        // No sample loaded, render silence
        return;
    }

    // Calculate final gain: volume parameter × MIDI velocity
    const float finalGain = volumeParameter * velocityGain;

    // Get sample data (handle both mono and stereo samples)
    const int numChannelsInSample = currentSampleBuffer->getNumChannels();
    const int sampleLength = currentSampleBuffer->getNumSamples();

    if (numChannelsInSample == 0)
        return;

    // Save starting position (will be updated once after all channels processed)
    double pos = playbackPosition;

    // Render to all output channels
    for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
    {
        auto* outputData = outputBuffer.getWritePointer(channel);

        // Use first channel if mono sample, otherwise use corresponding channel
        const int sampleChannel = juce::jmin(channel, numChannelsInSample - 1);
        const auto* sampleData = currentSampleBuffer->getReadPointer(sampleChannel);

        // Each channel reads from the same playback position
        double channelPos = pos;

        for (int i = 0; i < numSamples; ++i)
        {
            // Linear interpolation for fractional sample reading
            const int index = static_cast<int>(channelPos);

            if (index >= sampleLength - 1)
            {
                // Reached end of sample (one-shot, no loop)
                clearCurrentNote();
                return;
            }

            const float frac = static_cast<float>(channelPos - index);
            const float sample0 = sampleData[index];
            const float sample1 = sampleData[index + 1];

            // Linear interpolation: output = sample0 * (1 - frac) + sample1 * frac
            const float interpolatedSample = sample0 + frac * (sample1 - sample0);

            // Apply gain and add to output buffer (mix with other voices)
            outputData[startSample + i] += interpolatedSample * finalGain;

            // Advance channel's local playback position by playback rate
            channelPos += playbackRate;
        }
    }

    // Update playback position once after all channels processed (was being updated per channel - bug fix)
    playbackPosition = pos + (numSamples * playbackRate);
}
