# Stage 3 (GUI) Integration Checklist - SimpleSampler v2

**Plugin:** SimpleSampler
**Mockup Version:** v2
**Generated:** 2025-12-18
**Window Size:** 500x350 (non-resizable)
**Parameters:** 2 (volume, tuning)

## Overview

This checklist guides the gui-agent through integrating the finalized v2 WebView mockup into SimpleSampler during Stage 3 (GUI) implementation.

**Critical patterns enforced:**
- Member order: relays → webView → attachments
- ES6 module loading with `type="module"`
- Explicit resource provider URL mapping
- Three-parameter WebSliderParameterAttachment (JUCE 8)
- No viewport units in CSS

---

## 1. Copy UI Files

- [ ] Copy `v2-ui.html` to `Source/ui/public/index.html`
- [ ] Copy JUCE frontend library to `Source/ui/public/js/juce/index.js`
- [ ] Copy JUCE interop check to `Source/ui/public/js/juce/check_native_interop.js`
- [ ] Verify all file paths match CMake binary data configuration

**File locations:**
```
Source/
  ui/
    public/
      index.html                        (from v2-ui.html)
      js/
        juce/
          index.js                      (JUCE frontend library)
          check_native_interop.js       (JUCE initialization check)
```

---

## 2. Update PluginEditor.h

- [ ] Replace existing PluginEditor.h with `v2-PluginEditor-TEMPLATE.h` content
- [ ] Update class name to `SimpleSamplerAudioProcessorEditor`
- [ ] Update processor reference to `SimpleSamplerAudioProcessor&`
- [ ] Verify member order: relays (volumeRelay, tuningRelay) → webView → attachments
- [ ] Verify relay count = 2 (volume, tuning)
- [ ] Verify attachment count = 2 (matches relay count)
- [ ] Confirm `#include "PluginProcessor.h"` path is correct

**Critical member order:**
```cpp
private:
    SimpleSamplerAudioProcessor& audioProcessor;

    // 1. RELAYS FIRST
    std::unique_ptr<juce::WebSliderRelay> volumeRelay;
    std::unique_ptr<juce::WebSliderRelay> tuningRelay;

    // 2. WEBVIEW SECOND
    std::unique_ptr<juce::WebBrowserComponent> webView;

    // 3. ATTACHMENTS LAST
    std::unique_ptr<juce::WebSliderParameterAttachment> volumeAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> tuningAttachment;
```

---

## 3. Update PluginEditor.cpp

- [ ] Replace existing PluginEditor.cpp with `v2-PluginEditor-TEMPLATE.cpp` content
- [ ] Update class name to `SimpleSamplerAudioProcessorEditor`
- [ ] Update processor reference to `SimpleSamplerAudioProcessor&`
- [ ] Verify initialization order matches declaration order
- [ ] Verify window size: `setSize(500, 350)` (from YAML)
- [ ] Verify parameter IDs match APVTS: "volume", "tuning"
- [ ] Verify THREE parameters in WebSliderParameterAttachment (parameter, relay, nullptr)
- [ ] Update resource provider with correct BinaryData symbols

**Initialization order verification:**
```cpp
// 1. Create relays
volumeRelay = std::make_unique<juce::WebSliderRelay>("volume");
tuningRelay = std::make_unique<juce::WebSliderRelay>("tuning");

// 2. Create WebView with relay options
webView = std::make_unique<juce::WebBrowserComponent>(
    juce::WebBrowserComponent::Options{}
        .withNativeIntegrationEnabled()
        .withResourceProvider([this](const auto& url) { return getResource(url); })
        .withOptionsFrom(*volumeRelay)
        .withOptionsFrom(*tuningRelay)
);

// 3. Create attachments (THREE parameters - JUCE 8)
volumeAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
    *audioProcessor.parameters.getParameter("volume"), *volumeRelay, nullptr);
tuningAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
    *audioProcessor.parameters.getParameter("tuning"), *tuningRelay, nullptr);
```

---

## 4. Update CMakeLists.txt

- [ ] Append `v2-CMakeLists-SNIPPET.txt` to `CMakeLists.txt`
- [ ] Verify `juce_add_binary_data` includes all UI files:
  - `Source/ui/public/index.html`
  - `Source/ui/public/js/juce/index.js`
  - `Source/ui/public/js/juce/check_native_interop.js`
- [ ] Verify `JUCE_WEB_BROWSER=1` definition present
- [ ] Verify `juce::juce_gui_extra` linked
- [ ] Verify `NEEDS_WEB_BROWSER TRUE` in `juce_add_plugin()`

**CMake verification:**
```cmake
# Binary data
juce_add_binary_data(SimpleSampler_UIResources
    SOURCES
        Source/ui/public/index.html
        Source/ui/public/js/juce/index.js
        Source/ui/public/js/juce/check_native_interop.js
)

# Link resources
target_link_libraries(SimpleSampler
    PRIVATE
        SimpleSampler_UIResources
        juce::juce_gui_extra
)

# Enable WebView
target_compile_definitions(SimpleSampler
    PUBLIC
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0
)

# Plugin configuration
juce_add_plugin(SimpleSampler
    # ... other settings ...
    NEEDS_WEB_BROWSER TRUE  # REQUIRED
)
```

---

## 5. Build and Test (Debug)

- [ ] Clean build directory: `rm -rf build/`
- [ ] Run CMake configuration: `cmake -B build -DCMAKE_BUILD_TYPE=Debug`
- [ ] Build succeeds without warnings
- [ ] Install to system: `./scripts/build-and-install.sh SimpleSampler`
- [ ] Standalone loads WebView (not blank screen)
- [ ] Right-click → Inspect works (WebView developer tools)
- [ ] Console shows no JavaScript errors
- [ ] Console shows: "JUCE backend connected"
- [ ] Console shows: "SimpleSampler UI v2 initialized"
- [ ] `window.__JUCE__` object exists in console

**Debug build verification:**
```bash
# Build and install
./scripts/build-and-install.sh SimpleSampler

# Launch standalone
open build/SimpleSampler_artefacts/Debug/Standalone/SimpleSampler.app

# Check WebView initialization:
# - Right-click → Inspect Element
# - Console tab should show:
#   ✓ JUCE backend connected
#   ✓ SimpleSampler UI v2 initialized
#   ✓ Parameter bindings active
```

---

## 6. Build and Test (Release)

- [ ] Build release: `cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build`
- [ ] Release build succeeds without warnings
- [ ] Install to system: `./scripts/build-and-install.sh SimpleSampler`
- [ ] Plugin loads in DAW (no crashes)
- [ ] Reload plugin 10 times (tests member order correctness)
- [ ] No crashes on plugin reload
- [ ] WebView displays correctly in release mode

**Release build stress test:**
```bash
# Build release
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Install
./scripts/build-and-install.sh SimpleSampler

# Test in DAW:
# 1. Open project with SimpleSampler
# 2. Close and reopen plugin GUI 10 times
# 3. Close and reopen project 5 times
# 4. No crashes = member order correct
```

---

## 7. Test Parameter Binding

- [ ] Volume knob responds to mouse drag (rotates smoothly)
- [ ] Tuning knob responds to mouse drag (rotates smoothly)
- [ ] Double-click volume → resets to 0.75
- [ ] Double-click tuning → resets to 0.0 st
- [ ] Mouse wheel on knobs works (fine control)
- [ ] DAW automation updates UI knobs
- [ ] Preset recall updates UI knobs
- [ ] Parameter values persist after plugin reload

**Parameter binding test:**
```
1. Drag volume knob → value changes, rotation updates
2. Drag tuning knob → value changes, rotation updates
3. Double-click volume → jumps to 0.75
4. Double-click tuning → jumps to 0.0 st
5. Scroll mouse wheel on knob → fine adjustment
6. Automate parameter in DAW → knob follows automation
7. Save preset, load preset → knobs restore to saved values
8. Close GUI, reopen → knobs show correct positions
```

---

## 8. WebView-Specific Validation

- [ ] No viewport units in CSS (`100vh`, `100vw` violations)
- [ ] Native feel CSS present (`user-select: none`)
- [ ] Context menu disabled (right-click does nothing)
- [ ] Resource provider returns all files (no 404s in console)
- [ ] Correct MIME types: `text/html`, `application/javascript`
- [ ] ES6 module loading works (`type="module"` in script tags)
- [ ] `import { getSliderState }` syntax correct

**CSS validation:**
```bash
# Check for viewport unit violations
grep -r "100vh\|100vw\|100dvh\|100svh" Source/ui/public/index.html

# Expected: No matches (should use height: 100% instead)
```

**Resource provider validation:**
```
# Check browser console (Inspect Element):
# - No "Failed to load resource" errors
# - No "net::ERR_FAILED" for JS files
# - MIME type warnings (should be application/javascript, not text/javascript)
```

---

## 9. Sample Loading UI Test (Non-functional in v2)

Note: Sample loading functionality is DSP implementation (Stage 4). v2 mockup provides UI scaffolding only.

- [ ] Dropzone displays correctly (dashed border, folder icon)
- [ ] Browse button visible inside dropzone (top-left corner)
- [ ] Click browse button → file picker opens (OS native dialog)
- [ ] Drag file onto dropzone → visual feedback (border highlight)
- [ ] Console logs sample load attempt (no actual audio loading)

**UI scaffolding only:**
```javascript
// v2 mockup behavior (non-functional):
function loadSample(file) {
    console.log('Loading sample:', file.name);
    // Update UI only - no actual audio loading
    sampleName.textContent = file.name;
}

// Actual implementation in Stage 4 (DSP):
// 1. Send file path to C++ via window.__JUCE__.backend
// 2. C++ loads audio file into juce::AudioBuffer
// 3. C++ sends confirmation back to JavaScript
```

---

## 10. Final Verification

- [ ] All 2 parameters functional: volume, tuning
- [ ] All knobs interactive and visually correct
- [ ] Double-click reset works for both knobs
- [ ] No console errors or warnings
- [ ] Member order correct (verified by release build stability)
- [ ] WebView resources load correctly (no 404s)
- [ ] Plugin appears in DAW instrument browser (not effects)
- [ ] Window size correct: 500x350 pixels
- [ ] Non-resizable window (as per YAML spec)

---

## Parameter List (from parameter-spec.md)

| Parameter ID | Type  | Range           | Default | UI Control | Relay Type         |
|--------------|-------|-----------------|---------|------------|-------------------|
| volume       | Float | 0.0 to 1.0      | 0.75    | Knob       | WebSliderRelay    |
| tuning       | Float | -12.0 to +12.0  | 0.0     | Knob       | WebSliderRelay    |

**Total:** 2 parameters, 2 relays, 2 attachments

---

## Troubleshooting Reference

**If knobs freeze (don't respond to drag):**
- Check: Missing `type="module"` in script tags (Pattern #21)
- Check: Missing third parameter in WebSliderParameterAttachment (Pattern #12)
- Check: Member order violation (attachments before webView) (Pattern #11)

**If release build crashes on reload:**
- Check: Member order in PluginEditor.h (Pattern #11)
- Verify: Relays → WebView → Attachments

**If WebView blank on load:**
- Check: Viewport units in CSS (Pattern #9)
- Check: Resource provider returns `std::nullopt` for root URL
- Check: MIME types correct (`application/javascript`, not `text/javascript`)

**If parameters don't update:**
- Check: Parameter IDs match APVTS exactly ("volume", "tuning")
- Check: Three parameters in attachment constructor (parameter, relay, nullptr)
- Check: `.withOptionsFrom(*relay)` in WebView creation

---

## Next Steps After Integration

1. Test in multiple DAWs (Ableton, Logic, Reaper)
2. Test VST3 and AU formats
3. Verify parameter automation works
4. Test preset save/recall
5. Proceed to Stage 4 (DSP) for sample loading implementation

---

## References

- **JUCE 8 Critical Patterns:** `troubleshooting/patterns/juce8-critical-patterns.md`
- **WebView Patterns:** Patterns #9, #11, #12, #21
- **Member Order:** Pattern #11 (lines 326-384)
- **ES6 Modules:** Pattern #21 (lines 981-1071)
- **Parameter Spec:** `plugins/SimpleSampler/.ideas/parameter-spec.md`
