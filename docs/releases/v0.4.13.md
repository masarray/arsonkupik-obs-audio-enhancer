# ArSonKuPik OBS Audio Enhancer v0.4.13

## Highlights
- Hardens the real-time OBS engine for crackle-free parameter changes, lower CPU cost, and stable dynamics.
- Removes UI/audio data races through a lock-free POD parameter snapshot.
- Replaces preset fade-to-silence with a normalized dual-engine crossfade.
- Calibrates every factory preset for an immediate audible ON/bypass benefit instead of allowing restrained presets to sound almost unchanged or quieter than raw audio.

## Preset wow calibration
- Moderate macro values now use slightly stronger response curves, so Mastering, Audiophile, Movie, Podcast, Night Listening, and other restrained presets sound more clearly processed while preserving their own tonal identity.
- Every factory preset receives a fixed source-independent calibration trim. This is not an AGC: the value does not chase the source level and therefore does not recreate the pumping-prone smart-makeup loop removed during hardening.
- The maintained smoke signal now measures both RMS and peak benefit for every preset.
- Required factory-preset window: approximately **+3 dB RMS**, with transient peak benefit guarded below **+4.7 dB** and no clipping.

## Real-time architecture
- UI updates publish only atomic numeric values and flags. Preset names and `std::string` values never cross into shared audio-thread state.
- DSP engines are prepared and mutated only by the OBS audio callback.
- Preset changes are prepared in a second engine and crossfaded for 10 ms; no remainder of the OBS block is muted.
- Continuous macro automation uses a fixed 96-stage-capacity biquad bank with stable stage count inside each preset.
- Repeated macro retuning is allocation-free after initialization.

## CPU and stability
- Compressor attack/release coefficients, limiter release/ceiling/drive, width-detector coefficients, and linear input/output gains are cached.
- Compressor transfer calculation runs at an interpolated 8-sample control rate.
- Stereo analysis runs every 16 samples using true energy correlation.
- Meter conversion runs once per audio block instead of once per sample.
- Parallel compression uses unity-preserving linear dry/wet mixing.
- Gain staging keeps fixed -8 dB creative-chain headroom with calibrated restore; there is no dynamic full-band makeup AGC.

## Preset calibration results
The maintained 48 kHz stereo smoke signal produced the following default ON/bypass differences:

| Preset | RMS benefit | Peak benefit |
| --- | ---: | ---: |
| MasAri | +3.10 dB | +4.41 dB |
| Mastering Global | +3.07 dB | +3.82 dB |
| Max Enhancer | +3.16 dB | +4.03 dB |
| SonKuHoreg | +3.10 dB | +4.28 dB |
| SonKuBattle | +3.00 dB | +4.55 dB |
| SonKuBalap | +3.00 dB | +4.37 dB |
| Audiophile | +3.08 dB | +3.66 dB |
| Punchy Music | +3.27 dB | +3.95 dB |
| Open Air | +3.06 dB | +3.85 dB |
| Movie Sub | +3.00 dB | +4.15 dB |
| Podcast | +3.02 dB | +3.88 dB |
| Night Listening | +3.00 dB | +3.80 dB |

No preset clipped in this calibration run.

## Hardening regression results
- Automation maximum-jump ratio versus static render: **1.00071×** (limit: 1.10×).
- Limiter stress-test output peak: **-1.90 dBFS**.
- Correlation meter: **+1.000** for identical stereo and **-0.999996** for inverted stereo.
- Bass-linked pumping test variation: **0.0057 dB**.
- Realtime parameter/process allocation test: **0 allocations** after initialization.
- AddressSanitizer and UndefinedBehaviorSanitizer standalone tests: passed.

## Listening boundary
The numeric gates prevent quiet-preset regressions, extreme hidden boosts, clipping, allocation regressions, and known hardening failures. Final release approval still requires listening in native OBS with real music, voice, bass-heavy material, rapid knob movement, repeated preset switching, and several plugin instances.
