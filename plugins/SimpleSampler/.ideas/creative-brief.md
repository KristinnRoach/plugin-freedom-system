# SimpleSampler - Creative Brief

## Overview

**Type:** Synth (Instrument)
**Core Concept:** Minimal proof-of-concept sampler for JUCE learning - single sample loading with 16-voice polyphonic playback
**Status:** 💡 Ideated
**Created:** 2025-12-18

## Vision

SimpleSampler is an intentionally minimal sampler instrument designed as a learning project and foundation for future expansion. It focuses on core sampler functionality: load a single audio sample and play it polyphonically across the full MIDI keyboard range with pitch-shifting from a C3 root note. The simplicity allows for understanding JUCE's audio and MIDI handling while providing a functional instrument for testing in DAWs like Ableton.

This POC will serve as the base for a more sophisticated sampler with pre-processing pipelines, performance effects, sample trimming, and looping controls - but those features are explicitly scoped for future iterations after successful testing.

## Parameters

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| Volume | 0.0 - 1.0 | 0.75 | Output volume control (normalized linear scale) |
| Tuning | -12 to +12 semitones | 0 | Pitch adjustment ±1 octave from original sample |

## UI Concept

**Layout:** Simple two-knob interface with sample loading area
**Visual Style:** Reference GainKnob plugin for knob implementation
**Key Elements:**
- Sample drop zone / file browser button for loading audio files
- Volume knob (0-1 normalized, like GainKnob)
- Tuning knob (±12 semitones)
- Visual feedback showing loaded sample name

## Technical Notes

**Audio Processing:**
- Single sample playback engine
- 16-voice polyphony
- Root note: C3 (MIDI 60) triggers sample at original pitch
- Full MIDI range playable (C-2 to G8, all 128 notes)
- Pitch-shifting algorithm for transposition
- MIDI velocity controls playback volume

**Sample Loading:**
- Drag-and-drop onto UI
- File browser button (system file picker)
- Supported formats: WAV, AIFF, MP3

**Behavior:**
- One-shot playback (no looping in POC)
- Velocity-sensitive volume
- Polyphonic note handling with voice stealing at 16-voice limit

## Use Cases

- JUCE learning project - first-time user introduction to JUCE framework
- Testing in Ableton Live and other DAWs
- Playing pitched sounds polyphonically (melodic instruments, synths, sustained tones)
- Foundation for advanced sampler features (post-testing)

## Inspirations

- Classic hardware samplers (single-sample mode)
- GainKnob plugin (UI/knob implementation reference)
- Focus on simplicity and learning over feature richness

## Future Expansion (Post-Testing)

After successful Ableton testing, planned features include:
- Sample trimming controls (start/end points)
- Looping controls (loop points, crossfade)
- Pre-processing pipeline (filters, effects before sampling)
- Performance effects (post-sample processing)
- Multi-sample support
- Velocity layers

## Next Steps

- [ ] Create UI mockup (`/dream SimpleSampler` → option 3)
- [ ] Start implementation (`/implement SimpleSampler`)
