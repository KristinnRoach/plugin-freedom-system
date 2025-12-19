#pragma once
#include "PluginProcessor.h"

class SimpleSamplerAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit SimpleSamplerAudioProcessorEditor(SimpleSamplerAudioProcessor&);
    ~SimpleSamplerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    SimpleSamplerAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleSamplerAudioProcessorEditor)
};
