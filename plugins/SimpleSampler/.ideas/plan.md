# SimpleSampler - Implementation Plan

**Date:** 2025-12-18
**Complexity Score:** 3.4 (Complex)
**Strategy:** Phase-based implementation

---

## Complexity Factors

**Calculation breakdown:**

- **Parameters:** 2 parameters (2/5 = 0.4 points)
- **Algorithms:** 5 DSP components = 5.0 points
  - Sample Playback Engine (juce::Synthesiser)
  - Voice Management System (16 voices, polyphony)
  - Pitch-Shifting Engine (playback rate calculation)
  - Sample Interpolation (linear interpolation)
  - Velocity-Sensitive Volume (MIDI velocity × volume)
- **Features:** 2 points
  - File I/O with thread-safe sample loading (+1)
  - External MIDI control (computer keyboard → MIDI) (+1)
- **Total:** 0.4 + 5.0 + 2.0 = 7.4, capped at 5.0

**Adjusted score:** 3.4 (accounts for JUCE Synthesiser infrastructure reducing implementation burden)

**Rationale for adjustment:**
- Raw calculation: 7.4 (would suggest extreme complexity)
- JUCE Synthesiser provides: Voice allocation, MIDI parsing, polyphony management
- Reduces implementation from scratch to configuration + custom voice rendering
- Adjusted to 3.4: Complex but manageable with JUCE infrastructure

---

## Stages

- Stage 0: Research ✓
- Stage 1: Planning ✓
- Stage 1: Foundation ← Next
- Stage 2: Shell
- Stage 3: DSP (3 phases)
- Stage 3: GUI (2 phases)
- Stage 3: Validation

---

## Complex Implementation (Score = 3.4)

### Stage 3: DSP Phases

#### Phase 4.1: Core Sample Playback (Hardcoded Sample)

**Goal:** Validate JUCE Synthesiser architecture with polyphonic playback of hardcoded sample

**Components:**
- Custom SynthesiserSound class (holds hardcoded sample buffer)
- Custom SynthesiserVoice class (sample playback with linear interpolation)
- Synthesiser setup (16 voices, single sound)
- Basic pitch-shifting (playback rate calculation)
- MIDI input processing (note-on/off handling)
- Volume parameter (no tuning yet)

**Test Criteria:**
- [ ] Plugin loads in DAW without crashes
- [ ] Hardcoded sample plays when MIDI notes triggered
- [ ] 16-voice polyphony works (can play 16 simultaneous notes)
- [ ] Voice stealing works (17th note steals oldest voice)
- [ ] Pitch-shifting works (MIDI 60 = original pitch, MIDI 72 = 1 octave up)
- [ ] Volume parameter affects playback loudness
- [ ] MIDI velocity affects note volume
- [ ] No clicks, pops, or audio glitches

**Deliverables:**
- `SimpleSamplerSound.h/cpp` - SynthesiserSound subclass
- `SimpleSamplerVoice.h/cpp` - SynthesiserVoice subclass with interpolation
- `PluginProcessor.cpp` - Synthesiser setup in prepareToPlay()

---

#### Phase 4.2: Thread-Safe Sample Loading

**Goal:** Replace hardcoded sample with drag-drop and file browser loading

**Components:**
- AudioFormatManager setup (WAV, AIFF, MP3 support)
- Background thread file loading (std::async or juce::Thread)
- Atomic pointer swap for sample buffer
- Double-buffering (load to temp → swap → delete old)
- Sample rate conversion (if loaded sample rate ≠ plugin rate)
- File drag-drop UI handling (message thread)
- File browser button (FileChooser integration)

**Test Criteria:**
- [ ] Drag WAV file onto UI → sample loads and plays
- [ ] Drag AIFF file onto UI → sample loads and plays
- [ ] Drag MP3 file onto UI → sample loads and plays
- [ ] Browse button opens file picker → load works
- [ ] Loading sample doesn't cause audio glitches or dropouts
- [ ] Switching samples mid-playback is smooth (no crashes)
- [ ] Sample at 44.1kHz loads correctly into 48kHz plugin (resampling works)
- [ ] Rapid file loading doesn't cause memory leaks (Valgrind clean)
- [ ] Invalid file (e.g., text file) shows error, doesn't crash

**Deliverables:**
- `PluginProcessor.h/cpp` - AudioFormatManager, atomic buffer pointer
- File loading methods: `loadSampleFromFile()`, `atomicSwapBuffer()`
- Background thread launching and completion handling

---

#### Phase 4.3: Computer Keyboard to MIDI Mapping + Tuning Parameter

**Goal:** Enable computer keyboard input and add tuning parameter

**Components:**
- Lock-free MIDI queue (juce::AbstractFifo)
- KeyPress event handling in PluginEditor
- Major scale key-to-note mapping (based on reference implementation)
- MIDI message injection into processBlock
- Tuning parameter integration (adjust semitone offset)
- Key repeat detection (prevent multiple note-ons from held key)

**Test Criteria:**
- [ ] Pressing Z key triggers MIDI note 48 (C3)
- [ ] Pressing Q key triggers MIDI note 60 (C4)
- [ ] Major scale mapping works (white keys only, correct intervals)
- [ ] Holding key doesn't trigger multiple note-ons (key repeat handled)
- [ ] Tuning parameter shifts pitch correctly (tuning +12 = 1 octave up)
- [ ] Tuning + MIDI note combined correctly (MIDI 65 + tuning -5 = original pitch)
- [ ] Keyboard input doesn't block audio thread (no dropouts)
- [ ] Rapid typing doesn't drop notes (queue buffers correctly)

**Deliverables:**
- `PluginEditor.h/cpp` - KeyPress handling, MIDI queue, key mapping
- `PluginProcessor.h/cpp` - Queue draining in processBlock, tuning parameter integration
- Major scale interval array: `{0, 2, 4, 5, 7, 9, 11}`

---

### Stage 3: GUI Phases

#### Phase 5.1: Layout and Basic Controls

**Goal:** Integrate v2 mockup HTML with WebView, bind volume/tuning knobs

**Components:**
- Copy `v2-ui.html` to `Source/ui/public/index.html`
- Update `PluginEditor.h/cpp` with WebView setup
- Configure `CMakeLists.txt` for WebView resources (index.html, JUCE index.js)
- Implement resource provider (getResource method with explicit URL mapping)
- Create WebSliderRelay for volume and tuning parameters
- Create WebSliderParameterAttachment (3-parameter signature with nullptr)

**Test Criteria:**
- [ ] WebView window opens with 500×350 size
- [ ] Sample dropzone visible (420×80px dashed border)
- [ ] Browse button visible inside dropzone (folder icon)
- [ ] Volume knob visible (120px diameter, left position)
- [ ] Tuning knob visible (120px diameter, right position)
- [ ] Background color matches mockup (#1a1a1a)
- [ ] Text color matches mockup (white #ffffff)
- [ ] Layout matches v2 mockup exactly

**Deliverables:**
- `Source/ui/public/index.html` - Copied from v2-ui.html
- `Source/ui/public/js/juce/index.js` - JUCE WebView bridge
- `Source/ui/public/js/juce/check_native_interop.js` - WebView initialization
- `CMakeLists.txt` - Updated with NEEDS_WEB_BROWSER, juce_add_binary_data
- `PluginEditor.h/cpp` - WebView setup, resource provider, relays

---

#### Phase 5.2: Parameter Binding, File Handling, and Interaction

**Goal:** Two-way parameter communication + drag-drop + file browser

**Components:**
- JavaScript → C++ relay calls (knob drag updates parameters)
- C++ → JavaScript parameter updates (host automation, preset recall)
- Value formatting and display (volume: 0.75, tuning: +5.0 st)
- Double-click reset on knobs (volume → 0.75, tuning → 0.0)
- Drag-drop file handling (JavaScript detects drop → call C++ loadSample)
- Browse button click handler (JavaScript calls C++ openFileBrowser)
- Sample name display (show loaded file name)
- Relative knob drag (frame-delta, not absolute positioning)

**Test Criteria:**
- [ ] Dragging volume knob changes output volume (audio gets louder/quieter)
- [ ] Dragging tuning knob changes pitch (sample transposes up/down)
- [ ] Host automation updates knobs (automation lane → UI reflects changes)
- [ ] Preset change updates all knobs (load preset → knobs jump to saved values)
- [ ] Double-click volume knob → resets to 0.75
- [ ] Double-click tuning knob → resets to 0.0 st
- [ ] Dragging file onto dropzone → loads sample, shows file name
- [ ] Clicking browse button → opens file picker, loads selected file
- [ ] Parameter values display correctly (volume: "0.75", tuning: "+5.0 st")
- [ ] Knob drag is relative (not absolute) - smooth, natural interaction
- [ ] No lag or visual glitches during parameter changes

**Deliverables:**
- JavaScript: Knob drag handlers (mousedown, mousemove, mouseup)
- JavaScript: `valueChangedEvent.addListener()` with `getNormalisedValue()` (no callback params)
- JavaScript: Drag-drop handler for dropzone
- JavaScript: Browse button click handler
- C++ `PluginEditor`: `loadSampleFromDragDrop()`, `openFileBrowser()` methods
- Sample name display update logic

---

### Implementation Flow

- Stage 1: Foundation - project structure, CMakeLists.txt, PluginProcessor/Editor stubs
- Stage 2: Shell - APVTS with volume and tuning parameters
- Stage 3: DSP - 3 phases
  - Phase 4.1: Core sample playback (hardcoded sample, validate Synthesiser)
  - Phase 4.2: Thread-safe sample loading (drag-drop, file browser, atomic swap)
  - Phase 4.3: Computer keyboard to MIDI + tuning parameter
- Stage 3: GUI - 2 phases
  - Phase 5.1: Layout and basic controls (WebView setup, mockup integration)
  - Phase 5.2: Parameter binding and file handling (two-way communication, drag-drop)
- Stage 3: Validation - presets, pluginval, changelog, testing

---

## Implementation Notes

### Thread Safety

**Critical patterns:**
- All parameter reads use atomic `getRawParameterValue()->load()` (APVTS provides this)
- Sample buffer pointer: `std::atomic<AudioBuffer<float>*>` with acquire/release ordering
- Memory ordering: Audio thread uses `std::memory_order_acquire`, message thread uses `std::memory_order_release`
- Keyboard MIDI queue: `juce::AbstractFifo` (lock-free single-producer/single-consumer)
- NO mutex locks in audio thread (real-time violation)
- Buffer deletion ONLY on message thread (never on audio thread)

**Double-buffering pattern:**
1. Background thread: Load file into temp `AudioBuffer<float>*`
2. Background thread: Set completion flag (`std::atomic<bool>`)
3. Message thread: Poll flag, perform atomic swap
4. Message thread: Delete old buffer (safe, not in use by audio thread)
5. Audio thread: Load pointer with acquire ordering (sees new buffer)

### Performance

**Estimated CPU usage:**
- Synthesiser overhead: ~1-2% (JUCE voice allocation is efficient)
- Linear interpolation: ~5 CPU cycles per sample per voice
- 16 voices active: ~5-10% single core at 48kHz
- File loading: Background thread (doesn't affect audio thread)
- Keyboard queue: Negligible (~0.1% CPU)
- Total: ~5-10% single core, leaves headroom for future envelope/filters

**Optimization opportunities (future):**
- SIMD for sample rendering (4x speedup with SSE/NEON)
- Pre-calculate pitch ratios for common MIDI notes (lookup table)
- Skip inactive voices in voice stealing scan (reduce O(16) to O(active))

### Latency

**Processing latency:** 0 samples (one-shot playback, no lookahead)
- Do NOT report latency via `getLatencySamples()` (returns 0)
- Keyboard-to-MIDI latency: ~1-10ms (buffer boundary injection, imperceptible)
- File loading latency: Asynchronous (doesn't block playback)

### Denormal Protection

- Use `juce::ScopedNoDenormals` in `processBlock()` (flushes denormals to zero)
- Sample playback doesn't generate denormals (audio data typically non-zero)
- Tuning parameter cannot produce denormals (exponential function always normal)
- Volume at 0.0 produces silence (zeros, not denormals)

### Known Challenges

**Challenge 1: Voice stealing glitches**
- **Problem:** Abrupt cutoff when stealing oldest voice (no crossfade in POC)
- **Solution:** Acceptable for POC, can add short fade-out (5-10ms) post-testing if needed
- **Reference:** Kontakt uses fade-out on stolen voices

**Challenge 2: Interpolation quality at extreme transposition**
- **Problem:** Linear interpolation degrades at ±2 octaves (aliasing, muddy sound)
- **Solution:** Document recommended range (±1-2 octaves) in manual, upgrade to Lagrange post-POC
- **Reference:** Professional samplers use high-quality resampling (sinc, Lagrange)

**Challenge 3: Keyboard focus for key events**
- **Problem:** PluginEditor must have focus to receive KeyPress events
- **Solution:** Call `setWantsKeyboardFocus(true)` in constructor, test in DAWs
- **Fallback:** If focus unreliable, disable keyboard feature in plugin (Standalone-only)

**Challenge 4: Sample rate mismatch**
- **Problem:** Loaded sample at 44.1kHz but plugin at 48kHz (or vice versa)
- **Solution:** Resample during background load using `juce::ResamplingAudioSource`, convert before atomic swap
- **Reference:** JUCE tutorial "Playing sound files" shows resampling pattern

**Challenge 5: WebView parameter binding signature (JUCE 8)**
- **Problem:** JUCE 8 WebSliderParameterAttachment requires 3 parameters (old: 2)
- **Solution:** Always pass `nullptr` as third parameter (undoManager)
- **Reference:** juce8-critical-patterns.md pattern #12

**Challenge 6: ES6 module loading in WebView**
- **Problem:** JUCE index.js uses ES6 exports, requires `type="module"` in HTML
- **Solution:** Add `type="module"` to script tags, use `import { getSliderState }`
- **Reference:** juce8-critical-patterns.md pattern #21

---

## References

**Contract files:**
- Creative brief: `plugins/SimpleSampler/.ideas/creative-brief.md`
- Parameter spec: `plugins/SimpleSampler/.ideas/parameter-spec.md`
- DSP architecture: `plugins/SimpleSampler/.ideas/architecture.md`
- UI mockup: `plugins/SimpleSampler/.ideas/mockups/v2-ui.yaml`

**Similar plugins for reference:**
- **GainKnob** - WebView UI patterns (knob interaction, resource provider, relay setup)
  - Reference: Parameter binding, double-click reset, relative drag
- **TapeAge** - File I/O patterns (if implemented, check for sample loading reference)
  - Reference: Drag-drop handling, file browser integration
- **LushPad** - Synthesiser patterns (if implemented, check for JUCE Synthesiser usage)
  - Reference: Voice allocation, MIDI handling, instrument setup

**JUCE critical patterns:**
- `troubleshooting/patterns/juce8-critical-patterns.md` - Read BEFORE implementing
  - Pattern #1: CMakeLists.txt header generation (juce_generate_juce_header)
  - Pattern #9: CMakeLists.txt NEEDS_WEB_BROWSER for VST3
  - Pattern #11: WebView member initialization (std::unique_ptr ordering)
  - Pattern #12: WebSliderParameterAttachment 3-parameter signature
  - Pattern #15: valueChangedEvent callback (no parameters passed, call getNormalisedValue)
  - Pattern #16: Relative knob drag (frame-delta, not absolute)
  - Pattern #21: ES6 module loading (type="module" required)
  - Pattern #22: IS_SYNTH flag for instruments (REQUIRED for SimpleSampler)

**External references:**
- Keyboard mapping implementation: `/Users/kristinnroachgunnarsson/Desktop/Dev/sampler-monorepo/packages/audio-components/src/shared/keyboard/keyboard-keymaps.ts`
- JUCE Synthesiser tutorial: https://juce.com/tutorials/tutorial_synth_using_midi_input/
- JUCE file loading tutorial: https://juce.com/tutorials/tutorial_playing_sound_files/
