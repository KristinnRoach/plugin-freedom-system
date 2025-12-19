#include "PluginEditor.h"

SimpleSamplerAudioProcessorEditor::SimpleSamplerAudioProcessorEditor(SimpleSamplerAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // Phase 4.2: Minimal UI for testing file loading
    setSize(500, 350);  // Matches UI mockup v2 dimensions

    // Browse button
    addAndMakeVisible(browseButton);
    browseButton.setButtonText("Browse...");
    browseButton.onClick = [this]() { openFileBrowser(); };

    // Sample name label
    addAndMakeVisible(sampleNameLabel);
    sampleNameLabel.setText("No sample loaded", juce::dontSendNotification);
    sampleNameLabel.setJustificationType(juce::Justification::centred);
    sampleNameLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    // Start timer to update sample name display
    startTimer(100);  // Update every 100ms
}

SimpleSamplerAudioProcessorEditor::~SimpleSamplerAudioProcessorEditor()
{
    stopTimer();
}

void SimpleSamplerAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(juce::Colour(0xff1a1a1a));  // Matches UI mockup

    // Drag-drop area (dashed border)
    g.setColour(juce::Colours::grey);
    juce::Rectangle<int> dropzone(40, 20, 420, 120);

    // Dashed border
    const float dashLengths[] = { 5.0f, 5.0f };
    g.drawDashedLine(juce::Line<float>(dropzone.getTopLeft().toFloat(), dropzone.getTopRight().toFloat()), dashLengths, 2);
    g.drawDashedLine(juce::Line<float>(dropzone.getTopRight().toFloat(), dropzone.getBottomRight().toFloat()), dashLengths, 2);
    g.drawDashedLine(juce::Line<float>(dropzone.getBottomRight().toFloat(), dropzone.getBottomLeft().toFloat()), dashLengths, 2);
    g.drawDashedLine(juce::Line<float>(dropzone.getBottomLeft().toFloat(), dropzone.getTopLeft().toFloat()), dashLengths, 2);

    // Instructions
    g.setColour(juce::Colours::lightgrey);
    g.setFont(16.0f);
    g.drawFittedText("Drag & drop audio file here", dropzone.reduced(10), juce::Justification::centred, 2);

    g.setFont(12.0f);
    g.drawFittedText("Supported formats: WAV, AIFF, MP3", dropzone.reduced(10).removeFromBottom(30), juce::Justification::centred, 1);
}

void SimpleSamplerAudioProcessorEditor::resized()
{
    // Browse button in top-left corner of dropzone
    browseButton.setBounds(50, 30, 100, 30);

    // Sample name label below dropzone
    sampleNameLabel.setBounds(40, 150, 420, 30);
}

// Phase 4.2: Drag-drop interface
bool SimpleSamplerAudioProcessorEditor::isInterestedInFileDrag(const juce::StringArray& files)
{
    // Accept single audio file
    if (files.size() != 1)
        return false;

    juce::File file(files[0]);
    juce::String extension = file.getFileExtension().toLowerCase();

    // Accept WAV, AIFF, MP3
    return extension == ".wav" || extension == ".aiff" || extension == ".aif" || extension == ".mp3";
}

void SimpleSamplerAudioProcessorEditor::filesDropped(const juce::StringArray& files, int x, int y)
{
    juce::ignoreUnused(x, y);

    if (files.isEmpty())
        return;

    juce::File file(files[0]);
    processorRef.loadSampleFromFile(file);
}

// File browser
void SimpleSamplerAudioProcessorEditor::openFileBrowser()
{
    // Create file chooser for audio files
    auto fileChooser = std::make_shared<juce::FileChooser>(
        "Select an audio file",
        juce::File{},
        "*.wav;*.aiff;*.aif;*.mp3"
    );

    auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(flags, [this, fileChooser](const juce::FileChooser& chooser)
    {
        auto file = chooser.getResult();

        if (file.existsAsFile())
        {
            processorRef.loadSampleFromFile(file);
        }
    });
}

// Timer callback to update sample name display
void SimpleSamplerAudioProcessorEditor::timerCallback()
{
    juce::String sampleName = processorRef.getCurrentSampleName();

    if (sampleName.isEmpty())
        sampleNameLabel.setText("No sample loaded", juce::dontSendNotification);
    else
        sampleNameLabel.setText("Loaded: " + sampleName, juce::dontSendNotification);
}
