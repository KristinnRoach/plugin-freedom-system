#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout SimpleSamplerAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // volume - Float, 0.0 to 1.0, default 0.75, linear
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "volume", 1 },
        "Volume",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f),  // min, max, step, skew (1.0 = linear)
        0.75f  // default
    ));

    // tuning - Float, -12.0 to +12.0 semitones, default 0.0, linear
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "tuning", 1 },
        "Tuning",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f, 1.0f),  // min, max, step, skew (1.0 = linear)
        0.0f,  // default
        "st"   // semitones unit suffix
    ));

    return layout;
}

SimpleSamplerAudioProcessor::SimpleSamplerAudioProcessor()
    : AudioProcessor(BusesProperties()
                        .withOutput("Output", juce::AudioChannelSet::stereo(), true))  // Output-only for instruments
    , parameters(*this, nullptr, "Parameters", createParameterLayout())
{
}

SimpleSamplerAudioProcessor::~SimpleSamplerAudioProcessor()
{
}

void SimpleSamplerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Initialization will be added in Stage 2 (DSP)
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void SimpleSamplerAudioProcessor::releaseResources()
{
    // Cleanup will be added in Stage 2 (DSP)
}

void SimpleSamplerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    // Parameter access example (for Stage 2 DSP implementation):
    // auto* volumeParam = parameters.getRawParameterValue("volume");
    // float volumeValue = volumeParam->load();  // Atomic read (real-time safe)

    // auto* tuningParam = parameters.getRawParameterValue("tuning");
    // float tuningValue = tuningParam->load();  // Atomic read (real-time safe)

    // Clear output buffer for Stage 1 (no DSP yet - silent output)
    buffer.clear();
}

juce::AudioProcessorEditor* SimpleSamplerAudioProcessor::createEditor()
{
    return new SimpleSamplerAudioProcessorEditor(*this);
}

void SimpleSamplerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SimpleSamplerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

// Factory function
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleSamplerAudioProcessor();
}
