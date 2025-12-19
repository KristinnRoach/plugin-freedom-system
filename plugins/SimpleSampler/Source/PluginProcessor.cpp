#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <thread>

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
    // Register audio format readers (WAV, AIFF, MP3)
    formatManager.registerBasicFormats();
}

SimpleSamplerAudioProcessor::~SimpleSamplerAudioProcessor()
{
    // Clean up atomic sample buffer
    auto* buffer = currentSampleBuffer.exchange(nullptr);
    delete buffer;
}

void SimpleSamplerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);

    // Set synthesiser sample rate
    synth.setCurrentPlaybackSampleRate(sampleRate);

    // Add 16 voices for polyphony
    synth.clearVoices();
    for (int i = 0; i < 16; ++i)
        synth.addVoice(new SimpleSamplerVoice());

    // Add single sound (Phase 4.2: uses external sample buffer)
    synth.clearSounds();
    synth.addSound(new SimpleSamplerSound());
}

void SimpleSamplerAudioProcessor::releaseResources()
{
    // Cleanup will be added in Stage 2 (DSP)
}

void SimpleSamplerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Clear unused channels
    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Phase 4.3: Drain keyboard MIDI queue (lock-free, inject before synthesiser processing)
    while (midiQueue.getNumReady() > 0)
    {
        int start1, size1, start2, size2;
        midiQueue.prepareToRead(1, start1, size1, start2, size2);
        if (size1 > 0)
        {
            midiMessages.addEvent(midiBuffer[static_cast<size_t>(start1)], 0);
            midiQueue.finishedRead(1);
        }
    }

    // Read volume parameter (atomic, real-time safe)
    auto* volumeParam = parameters.getRawParameterValue("volume");
    float volumeValue = volumeParam->load();

    // Phase 4.3: Read tuning parameter (atomic, real-time safe)
    auto* tuningParam = parameters.getRawParameterValue("tuning");
    float tuningValue = tuningParam->load();

    // Update volume and tuning for all voices
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<SimpleSamplerVoice*>(synth.getVoice(i)))
        {
            voice->setVolumeParameter(volumeValue);
            voice->setTuningParameter(tuningValue);
        }
    }

    // Phase 4.2: Pass loaded sample buffer to sound (atomic load with acquire ordering)
    auto* sampleBuffer = currentSampleBuffer.load(std::memory_order_acquire);
    if (sampleBuffer != nullptr && synth.getNumSounds() > 0)
    {
        if (auto* sound = dynamic_cast<SimpleSamplerSound*>(synth.getSound(0).get()))
            sound->setSampleBuffer(sampleBuffer);
    }

    // Render synthesiser (handles MIDI, voice allocation, sample playback)
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
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

// Phase 4.2: File loading implementation
void SimpleSamplerAudioProcessor::loadSampleFromFile(const juce::File& file)
{
    if (!file.existsAsFile())
    {
        DBG("File does not exist: " + file.getFullPathName());
        return;
    }

    // Launch background thread to load file
    loadSampleInBackground(file);
}

void SimpleSamplerAudioProcessor::loadSampleInBackground(const juce::File& file)
{
    // Background thread: Load file using AudioFormatReader
    std::thread([this, file]()
    {
        // Create reader for the file
        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

        if (reader == nullptr)
        {
            DBG("Failed to create reader for file: " + file.getFullPathName());
            juce::MessageManager::callAsync([this]()
            {
                currentSampleName = "Invalid File";
            });
            return;
        }

        // Allocate new buffer for loaded sample
        auto* newBuffer = new juce::AudioBuffer<float>(
            static_cast<int>(reader->numChannels),
            static_cast<int>(reader->lengthInSamples)
        );

        // Read entire file into buffer
        reader->read(newBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);

        // Get sample name
        juce::String sampleName = file.getFileNameWithoutExtension();
        juce::String samplePath = file.getFullPathName();

        // Atomic swap on message thread (NOT background thread)
        juce::MessageManager::callAsync([this, newBuffer, sampleName, samplePath]()
        {
            atomicSwapBuffer(newBuffer, sampleName, samplePath);
        });
    }).detach();
}

void SimpleSamplerAudioProcessor::atomicSwapBuffer(juce::AudioBuffer<float>* newBuffer, const juce::String& sampleName, const juce::String& samplePath)
{
    // Atomic swap with release ordering (message thread)
    auto* oldBuffer = currentSampleBuffer.exchange(newBuffer, std::memory_order_release);

    // Update sample name and path
    currentSampleName = sampleName;
    currentSamplePath = samplePath;

    // Delete old buffer (safe: audio thread done with it)
    delete oldBuffer;

    DBG("Sample loaded: " + sampleName);
}

// Phase 4.3: Add keyboard MIDI to queue (called from PluginEditor on message thread)
void SimpleSamplerAudioProcessor::addKeyboardMidi(const juce::MidiMessage& message)
{
    // Lock-free queue write (message thread → audio thread)
    if (midiQueue.getFreeSpace() > 0)
    {
        int start1, size1, start2, size2;
        midiQueue.prepareToWrite(1, start1, size1, start2, size2);
        if (size1 > 0)
        {
            midiBuffer[static_cast<size_t>(start1)] = message;
            midiQueue.finishedWrite(1);
        }
    }
    else
    {
        // Queue full - drop message (extremely unlikely with 128 buffer)
        DBG("Keyboard MIDI queue full, dropping message");
    }
}

// Factory function
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleSamplerAudioProcessor();
}
