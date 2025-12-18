#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

/**
 * SimpleSampler WebView-based Plugin Editor - v2
 *
 * CRITICAL: Member declaration order prevents release build crashes.
 * Order: Relays → WebView → Attachments
 *
 * Destruction order (reverse of declaration):
 * 1. Attachments destroyed FIRST (stop using relays and WebView)
 * 2. WebView destroyed SECOND (safe, attachments are gone)
 * 3. Relays destroyed LAST (safe, nothing using them)
 *
 * Generated from v2-ui.yaml mockup specification
 * Parameters: volume (slider), tuning (slider)
 */

class SimpleSamplerAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    /**
     * Constructor
     * @param p Reference to the audio processor
     */
    SimpleSamplerAudioProcessorEditor(SimpleSamplerAudioProcessor& p);

    /**
     * Destructor
     * Members destroyed in reverse order of declaration.
     */
    ~SimpleSamplerAudioProcessorEditor() override;

    // AudioProcessorEditor overrides
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    /**
     * Resource provider (JUCE 8 required pattern)
     * Maps URLs to embedded binary data.
     *
     * @param url Requested resource URL (e.g., "/", "/js/juce/index.js")
     * @return Resource data and MIME type, or std::nullopt for 404
     */
    std::optional<juce::WebBrowserComponent::Resource> getResource(
        const juce::String& url
    );

    // Reference to audio processor
    SimpleSamplerAudioProcessor& audioProcessor;

    // ========================================================================
    // ⚠️ CRITICAL MEMBER DECLARATION ORDER ⚠️
    //
    // Order: Relays → WebView → Attachments
    //
    // Why: Members are destroyed in REVERSE order of declaration.
    // - Attachments must be destroyed BEFORE WebView (they call evaluateJavascript)
    // - WebView must be destroyed BEFORE Relays (it holds references via Options)
    //
    // DO NOT REORDER without understanding destructor sequence!
    // ========================================================================

    // ------------------------------------------------------------------------
    // 1️⃣ RELAYS FIRST (created first, destroyed last)
    // ------------------------------------------------------------------------
    //
    // Relays bridge C++ parameters to JavaScript state objects.
    // They have no dependencies, so they're declared first.
    //
    // SimpleSampler v2 has 2 parameters (both sliders):
    // - volume: 0.0 to 1.0 (normalized gain)
    // - tuning: -12.0 to +12.0 semitones
    //
    std::unique_ptr<juce::WebSliderRelay> volumeRelay;
    std::unique_ptr<juce::WebSliderRelay> tuningRelay;

    // ------------------------------------------------------------------------
    // 2️⃣ WEBVIEW SECOND (created after relays, destroyed before relays)
    // ------------------------------------------------------------------------
    //
    // WebBrowserComponent is the HTML rendering engine.
    // It depends on relays (registered via withOptionsFrom).
    //
    // Must be destroyed AFTER attachments (they call evaluateJavascript).
    // Must be destroyed BEFORE relays (holds references to them).
    //
    std::unique_ptr<juce::WebBrowserComponent> webView;

    // ------------------------------------------------------------------------
    // 3️⃣ PARAMETER ATTACHMENTS LAST (created last, destroyed first)
    // ------------------------------------------------------------------------
    //
    // Attachments synchronize APVTS parameters with relay state.
    // They depend on BOTH the relay AND the WebView.
    //
    // MUST be declared AFTER WebView to ensure correct destruction order.
    //
    std::unique_ptr<juce::WebSliderParameterAttachment> volumeAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> tuningAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleSamplerAudioProcessorEditor)
};
