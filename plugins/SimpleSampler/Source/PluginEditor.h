#pragma once
#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <set>

class SimpleSamplerAudioProcessorEditor : public juce::AudioProcessorEditor,
                                           public juce::FileDragAndDropTarget,
                                           public juce::Timer
{
public:
    explicit SimpleSamplerAudioProcessorEditor(SimpleSamplerAudioProcessor&);
    ~SimpleSamplerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // FileDragAndDropTarget interface (Phase 4.2)
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    // Timer callback for UI updates
    void timerCallback() override;

    // Keyboard input (Phase 4.3)
    bool keyPressed(const juce::KeyPress& key) override;
    bool keyStateChanged(bool isKeyDown) override;

private:
    SimpleSamplerAudioProcessor& processorRef;

    // UI Components (Phase 4.2: Minimal UI for file loading testing)
    juce::TextButton browseButton;
    juce::Label sampleNameLabel;

    // File browser
    void openFileBrowser();

    // Keyboard-to-MIDI mapping (Phase 4.3)
    int mapKeyToMidiNote(int keyCode) const;
    std::set<int> activeKeys;  // Track active keys to prevent key repeat

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleSamplerAudioProcessorEditor)
};
