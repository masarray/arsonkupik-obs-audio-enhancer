# ArSonKuPik OBS Audio Enhancer v0.4.13

## Highlights
- Hardens the real-time OBS engine for crackle-free parameter changes, lower CPU cost, and stable dynamics.
- Removes UI/audio data races by publishing a lock-free POD parameter snapshot.
- Replaces preset fade-to-silence with a normalized dual-engine crossfade.
- Removes the interacting smart-headroom and smart-makeup AGC loops that could make bass-heavy material breathe.

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
- Parallel compression now uses unity-preserving linear dry/wet mixing.
- Gain staging is fixed at -8 dB creative-chain headroom with calibrated +4.9 dB restore; there is no dynamic full-band makeup AGC.

## Local regression results
- MasAri smoke-test peak benefit: **+3.18 dB**.
- Automation maximum-jump ratio versus static render: **1.00071×** (limit: 1.10×).
- Limiter stress-test output peak: **-1.90 dBFS**.
- Correlation meter: **+1.000** for identical stereo and **-0.999996** for inverted stereo.
- Bass-linked pumping test variation: **0.0057 dB**.
- Realtime parameter/process allocation test: **0 allocations** after initialization.
- AddressSanitizer and UndefinedBehaviorSanitizer standalone tests: passed.

## Validation boundary
The standalone DSP build, smoke test, hardening test, sanitizer build, warning-as-error build, and an OBS-wrapper syntax build with API stubs passed locally. The native Windows and Linux OBS plugin builds still need the repository CI workflows to validate against real OBS headers and libraries.
