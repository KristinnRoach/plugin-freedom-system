# Parameter Specification (Draft)

**Status:** Draft - Awaiting UI mockup for full specification
**Created:** 2025-12-18
**Source:** Quick capture during ideation

This is a lightweight specification to enable parallel DSP research.
Full specification will be generated from finalized UI mockup.

## Parameters

### volume
- **Type:** Float
- **Range:** 0.0 to 1.0 (normalized)
- **Default:** 0.75
- **DSP Purpose:** Controls output volume/gain for the sampled audio. Works in conjunction with MIDI velocity to determine final playback volume.

### tuning
- **Type:** Float
- **Range:** -12 to +12 semitones
- **Default:** 0
- **DSP Purpose:** Adjusts pitch offset from the root note (C3/MIDI 60). Allows transposition of the loaded sample up to ±1 octave. Applied to pitch-shifting algorithm for all voices.

## Implementation Notes

- **Root Note:** C3 (MIDI 60) triggers sample at original pitch
- **Polyphony:** 16 voices with voice stealing
- **Velocity:** MIDI velocity controls playback volume (multiplied with volume parameter)
- **Sample Loading:** Drag-drop + file browser (WAV, AIFF, MP3)

## Next Steps

- [ ] Complete UI mockup workflow (/dream → option 3)
- [ ] Finalize design and generate full parameter-spec.md
- [ ] Validate consistency between draft and final spec
