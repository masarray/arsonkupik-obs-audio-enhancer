# Audio quality standards

ArSonKuPik OBS Audio Enhancer is designed as a real-time listening product, not as a volume trick. Changes to the DSP engine, presets, macro mapping, gain staging, or OBS wrapper should preserve the standards in this document.

## Product targets

The public quality targets are:

1. **Crackle-free control movement** — moving a continuous control must not reset filter history, create zipper noise, or add an audible discontinuity beyond the derivative already present in the source.
2. **Stable preset changes** — switching presets must not create a hard pop or click, even when the preset changes DSP topology.
3. **Practical CPU use** — steady-state audio processing must remain suitable for live OBS use and must not keep rebuilding or allocating after parameters have settled.
4. **Stable dynamics** — enhancement must not create obvious full-band pumping, breathing, or recovery wobble on bass-heavy or transient material.
5. **Calibrated loudness benefit** — the main listening target is a tasteful Filter ON benefit of roughly **3–4 dB** in the project smoke test, not an extreme hidden boost.
6. **No clipping regression** — test output must remain finite and below full-scale sample clipping under the maintained smoke-test scenarios.
7. **Stereo integrity** — widening must preserve a stable center and useful mono compatibility; narrowing must move predictably toward mono without phase instability.
8. **Audible macro range** — Smart Bass, Smart Treble, Vocal Body, and Stereo Magic should produce a clearly audible min-to-max change while keeping the neutral region useful.

## DSP implementation rules

Real-time DSP changes should follow these rules unless a pull request provides strong measurements and a documented reason to deviate:

- Do not clear or replace stateful filters merely because a coefficient changed.
- Reuse existing filter stages when the topology is unchanged.
- Smooth continuous user parameters before applying them to the DSP engine.
- Apply coefficient changes at a controlled rate, normally once per audio block rather than once per mouse event.
- Avoid heap allocation, file access, logging, locks, or unbounded work in the steady-state audio path.
- Stop retuning after smoothed parameters reach their targets.
- Mask unavoidable topology changes with a short click-safe transition.
- Clamp frequencies, Q values, gains, widths, ratios, times, and mixes to valid ranges.
- Keep gain staging explicit. Do not hide a large output boost inside tonal macros.
- Prefer linked and conservative dynamics when full-band processing can be driven by bass transients.
- Preserve the original center image before adding side enhancement.

## Gain-staging policy

The current engine uses internal creative-chain headroom before EQ, color, compressor, width, and output protection. Only part of that headroom is restored automatically.

The intended result is:

- Filter ON sounds more polished and engaging.
- The audible improvement remains present at a controlled listening level.
- The main preset does not jump tens of decibels above bypass.
- Output Trim remains a user-controlled final adjustment rather than an implicit part of tonal macro mapping.
- Smart protection and limiter behavior prevent clipping without becoming the primary loudness engine.

A louder result is not automatically a better result. Every listening review should include both:

1. **Native ON/OFF comparison** — confirms the intended product benefit.
2. **Loudness-matched comparison** — checks whether tonal, spatial, and dynamic improvements remain after loudness bias is reduced.

## Crackle and automation validation

A control-automation test should:

1. Generate or load a continuous signal with stable level.
2. Process a static reference with unchanged controls.
3. Process the same signal while aggressively sweeping each continuous control.
4. Measure maximum sample-to-sample jump for the static and automated renders.
5. Confirm that automation does not introduce non-finite samples or a materially larger discontinuity.
6. Repeat while changing presets during active audio.

Recommended release gate:

- automation jump ratio should remain close to the static reference and normally not exceed **1.10×** without a documented source-dependent explanation;
- no NaN or infinity values;
- no audible crackle, zipper noise, hard pop, or click in headphones and speakers.

This metric is a regression guard, not a complete perceptual model. A passing number does not override an audible failure.

## Dynamics and pumping validation

Use material that exposes gain-envelope problems:

- bass-heavy music with repeated kick transients;
- sustained vocal over changing low-frequency energy;
- sparse percussion with long decays;
- dense full-range program material;
- spoken voice with pauses and plosives.

Listen for:

- the vocal or treble level rocking after every kick;
- the whole mix audibly inhaling and exhaling;
- long recovery swells after a loud transient;
- limiter chatter or repeated brightness changes;
- stereo image movement caused by linked or asymmetric gain behavior.

The preferred correction order is:

1. reduce unnecessary makeup gain;
2. reduce full-band parallel mix or ratio;
3. adjust attack and release for calmer recovery;
4. improve gain staging before adding more limiting;
5. use source-aware or frequency-selective processing only when the simpler solution is insufficient.

## CPU and real-time validation

CPU quality is not defined by one machine or one percentage. A release should instead demonstrate that processing remains bounded and stable.

Validate at least:

- 44.1 kHz and 48 kHz;
- mono and stereo sources;
- continuous playback while moving controls;
- repeated preset changes;
- multiple plugin instances when practical;
- idle playback after all controls have settled.

Reject a change when it causes:

- audio dropouts or missed real-time deadlines;
- continuous coefficient rebuilds after controls stop moving;
- recurring allocation in steady state;
- avoidable per-sample work that can be calculated per block;
- a large CPU increase without a documented and audible benefit.

Global oversampling is not the default solution. Any oversampling proposal must include an explicit quality benefit, CPU measurements, and a lower-cost comparison.

## Stereo validation

Test with:

- true mono content;
- centered vocal with stereo accompaniment;
- already-wide commercial music;
- strongly correlated left/right material;
- headphones and speakers;
- mono fold-down.

Acceptance criteria:

- the center remains stable when widening;
- a stereo source is not accidentally converted to mono at the neutral position;
- the far-left Stereo Magic position reaches a predictable narrow or mono result;
- the far-right position sounds wider without obvious phase swirl or hollow center;
- mono fold-down does not reveal severe cancellation or unexpected tonal loss.

## Macro-control validation

For each macro, test `0 → 50 → 100 → 50` while audio is continuous.

### Smart Bass

- Left: rumble and mud cleanup should be audible without making the source thin or phasey.
- Center: should remain a useful neutral reference.
- Right: sub weight, body, and punch should increase without uncontrolled low-frequency level or compressor pumping.

### Smart Treble

- Left: de-harsh and de-ess behavior should smooth the source without dulling it completely.
- Center: should remain neutral.
- Right: air, presence, and sparkle should be audible without brittle peaks or exaggerated sibilance.

### Vocal Body

- Left: vocal should tuck and smooth predictably.
- Center: should remain neutral.
- Right: body, clarity, and forward presence should improve without honk, masking, or excessive output gain.

### Stereo Magic

- Left: should narrow predictably toward mono.
- Center: should preserve the source stereo image.
- Right: should widen progressively while protecting center focus and mono compatibility.

## Maintained smoke-test expectations

The standalone DSP smoke test is a fast regression screen. It should verify at minimum:

- all maintained presets produce finite output;
- clipping guards remain effective;
- the main preset stays near its calibrated loudness target;
- extreme hidden gain regressions fail the test;
- representative presets continue to provide audible benefit;
- DSP code builds on supported CI platforms.

Run the standalone test with:

```bash
cmake -S . -B build-test -DBUILD_OBS_PLUGIN=OFF -DBUILD_STANDALONE_TESTS=ON
cmake --build build-test --config Release
```

On Linux:

```bash
./build-test/arsonkupik_dsp_smoke
```

On a multi-configuration Windows generator, the executable is normally under `build-test/Release/`.

## Listening-test protocol

Use the same source, source fader, monitoring path, and sample rate throughout a comparison.

Document:

- ArSonKuPik version;
- OBS Studio version;
- operating system;
- sample rate;
- source type;
- exact preset;
- all macro values;
- Smart Protect state;
- Output Trim;
- bypass and filter states;
- observed input and output peaks;
- whether the comparison was loudness matched;
- headphones, speakers, or both.

Use several source types before changing a global preset or engine coefficient. A correction that helps one song but damages speech, mono content, or already-wide sources is not a global improvement.

## Release quality gate

Before a DSP-affecting release:

- [ ] Windows CI build passes.
- [ ] Linux plugin build passes.
- [ ] Standalone DSP smoke test passes.
- [ ] Continuous control sweeps are audibly crackle-free.
- [ ] Preset switching is free of hard pops and clicks.
- [ ] Filter ON gain remains within the documented target for the main preset.
- [ ] Bass-heavy material does not produce obvious full-band pumping.
- [ ] Stereo widening preserves center focus and acceptable mono fold-down.
- [ ] CPU remains stable during automation and after parameters settle.
- [ ] Release notes describe any intentional audible change.

## Reporting an audio-quality problem

Use the dedicated **Audio quality report** issue form and include the exact settings, source type, sample rate, reproduction steps, and ON/bypass observations:

https://github.com/masarray/arsonkupik-obs-audio-enhancer/issues/new/choose

Do not upload copyrighted material unless you have permission to share it. A short original or licensed test sample is preferred.
