#include "PluginEditor.h"

SimpleSamplerAudioProcessorEditor::SimpleSamplerAudioProcessorEditor(SimpleSamplerAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // Phase 4.2: Minimal UI for testing file loading
    setSize(500, 350);  // Matches UI mockup v2 dimensions

    // Phase 4.3: Enable keyboard focus for key events
    setWantsKeyboardFocus(true);

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

// Phase 4.3: Keyboard input handling
bool SimpleSamplerAudioProcessorEditor::keyPressed(const juce::KeyPress& key)
{
    int keyCode = key.getKeyCode();

    // Check if key already active (prevent key repeat)
    if (activeKeys.count(keyCode) > 0)
        return true;  // Key already down, ignore repeat

    // Map key to MIDI note
    int midiNote = mapKeyToMidiNote(keyCode);

    if (midiNote >= 0)
    {
        // Add to active keys
        activeKeys.insert(keyCode);

        // Create MIDI note-on message (velocity 100, channel 1)
        auto noteOn = juce::MidiMessage::noteOn(1, midiNote, static_cast<juce::uint8>(100));
        processorRef.addKeyboardMidi(noteOn);

        return true;  // Key handled
    }

    return false;  // Key not mapped
}

bool SimpleSamplerAudioProcessorEditor::keyStateChanged(bool isKeyDown)
{
    if (!isKeyDown)
    {
        // Key released - send note-off for all active keys
        auto activeKeysCopy = activeKeys;  // Copy to avoid iterator invalidation

        for (int keyCode : activeKeysCopy)
        {
            // Check if key is still down
            if (!juce::KeyPress::isKeyCurrentlyDown(keyCode))
            {
                // Map key to MIDI note
                int midiNote = mapKeyToMidiNote(keyCode);

                if (midiNote >= 0)
                {
                    // Create MIDI note-off message
                    auto noteOff = juce::MidiMessage::noteOff(1, midiNote, static_cast<juce::uint8>(0));
                    processorRef.addKeyboardMidi(noteOff);
                }

                // Remove from active keys
                activeKeys.erase(keyCode);
            }
        }
    }

    return true;
}

// Phase 4.3: Major scale keyboard mapping
int SimpleSamplerAudioProcessorEditor::mapKeyToMidiNote(int keyCode) const
{
    // Major scale intervals: [0, 2, 4, 5, 7, 9, 11] (C, D, E, F, G, A, B)
    static const int majorScaleIntervals[] = { 0, 2, 4, 5, 7, 9, 11 };

    // Base note: C3 (MIDI 48) for bottom row
    const int baseNote = 48;

    // Bottom row (Z to M): C3-B3 (MIDI 48-59)
    // Z X C V B N M
    if (keyCode == 'Z') return baseNote + majorScaleIntervals[0];  // C3 (48)
    if (keyCode == 'X') return baseNote + majorScaleIntervals[1];  // D3 (50)
    if (keyCode == 'C') return baseNote + majorScaleIntervals[2];  // E3 (52)
    if (keyCode == 'V') return baseNote + majorScaleIntervals[3];  // F3 (53)
    if (keyCode == 'B') return baseNote + majorScaleIntervals[4];  // G3 (55)
    if (keyCode == 'N') return baseNote + majorScaleIntervals[5];  // A3 (57)
    if (keyCode == 'M') return baseNote + majorScaleIntervals[6];  // B3 (59)

    // 2nd row (A to L): C4-B4 (MIDI 60-71, one octave up)
    // A S D F G H J K L
    if (keyCode == 'A') return baseNote + 12 + majorScaleIntervals[0];  // C4 (60)
    if (keyCode == 'S') return baseNote + 12 + majorScaleIntervals[1];  // D4 (62)
    if (keyCode == 'D') return baseNote + 12 + majorScaleIntervals[2];  // E4 (64)
    if (keyCode == 'F') return baseNote + 12 + majorScaleIntervals[3];  // F4 (65)
    if (keyCode == 'G') return baseNote + 12 + majorScaleIntervals[4];  // G4 (67)
    if (keyCode == 'H') return baseNote + 12 + majorScaleIntervals[5];  // A4 (69)
    if (keyCode == 'J') return baseNote + 12 + majorScaleIntervals[6];  // B4 (71)

    // 3rd row (Q to P): C5-B5 (MIDI 72-83, two octaves up)
    // Q W E R T Y U I O P
    if (keyCode == 'Q') return baseNote + 24 + majorScaleIntervals[0];  // C5 (72)
    if (keyCode == 'W') return baseNote + 24 + majorScaleIntervals[1];  // D5 (74)
    if (keyCode == 'E') return baseNote + 24 + majorScaleIntervals[2];  // E5 (76)
    if (keyCode == 'R') return baseNote + 24 + majorScaleIntervals[3];  // F5 (77)
    if (keyCode == 'T') return baseNote + 24 + majorScaleIntervals[4];  // G5 (79)
    if (keyCode == 'Y') return baseNote + 24 + majorScaleIntervals[5];  // A5 (81)
    if (keyCode == 'U') return baseNote + 24 + majorScaleIntervals[6];  // B5 (83)

    // Top row (1 to 0): C6-B6 (MIDI 84-95, three octaves up)
    // 1 2 3 4 5 6 7 8 9 0
    if (keyCode == '1') return baseNote + 36 + majorScaleIntervals[0];  // C6 (84)
    if (keyCode == '2') return baseNote + 36 + majorScaleIntervals[1];  // D6 (86)
    if (keyCode == '3') return baseNote + 36 + majorScaleIntervals[2];  // E6 (88)
    if (keyCode == '4') return baseNote + 36 + majorScaleIntervals[3];  // F6 (89)
    if (keyCode == '5') return baseNote + 36 + majorScaleIntervals[4];  // G6 (91)
    if (keyCode == '6') return baseNote + 36 + majorScaleIntervals[5];  // A6 (93)
    if (keyCode == '7') return baseNote + 36 + majorScaleIntervals[6];  // B6 (95)

    return -1;  // Key not mapped
}
