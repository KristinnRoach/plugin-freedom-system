#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "SimpleSamplerSound.h"

/**
 * SimpleSamplerVoice - Handles sample playback with pitch-shifting
 *
 * Features:
 * - Linear interpolation for fractional sample reading
 * - Pitch-shifting via playback rate calculation
 * - Velocity-sensitive volume
 * - Volume parameter integration
 */
class SimpleSamplerVoice : public juce::SynthesiserVoice
{
public:
    SimpleSamplerVoice() = default;
    ~SimpleSamplerVoice() override = default;

    // SynthesiserVoice interface
    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

    // Volume control (called from PluginProcessor::processBlock)
    void setVolumeParameter(float volume) { volumeParameter = volume; }

private:
    // Playback state
    double playbackPosition = 0.0;
    double playbackRate = 1.0;
    float velocityGain = 1.0f;
    float volumeParameter = 0.75f;

    // Sample reference (set in startNote)
    const juce::AudioBuffer<float>* currentSampleBuffer = nullptr;
    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleSamplerVoice)
};
