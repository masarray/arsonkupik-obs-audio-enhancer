# Audio quality standards

ArSonKuPik OBS Audio Enhancer is designed as a real-time listening product, not as a volume trick. Changes to the DSP engine, presets, macro mapping, gain staging, transition layer, or OBS wrapper must preserve the standards in this document.

## Product targets

1. **Click-safe control movement** — moving a continuous control must not reset filter history, create zipper noise, or add a material discontinuity beyond the source derivative.
2. **Stable preset and bypass transitions** — preset or bypass changes must not create a hard pop, click, or muted block remainder.
3. **Practical CPU use** — steady-state processing must remain suitable for live OBS use, stop rebuilding after controls settle, and skip the creative chain after bypass becomes fully dry.
4. **Stable dynamics** — enhancement must not create obvious full-band pumping, breathing, recovery wobble, or limiter chatter.
5. **Calibrated loudness benefit** — moderate-level sources target an audible Filter ON benefit near 3 dB; hot mastered sources may produce a smaller lift to protect crest factor and avoid sustained limiting.
6. **No clipping regression** — maintained test output must remain finite and below the configured sample ceiling.
7. **Stereo integrity** — widening must preserve a stable center and useful mono compatibility; narrowing must move predictably toward mono.
8. **Audible macro range** — Smart Bass, Smart Treble, Vocal Body, and Stereo Magic must produce clearly audible movement while retaining a useful neutral region.

These are engineering goals and regression gates, not an absolute guarantee for every driver, host configuration, device, or source.

## Channel policy

ArSonKuPik currently processes the stereo/front-left and front-right pair. On a multichannel source, channels after front L/R are passed through unchanged.

This policy avoids applying:

- unlinked gain reduction to center, LFE, or surround channels;
- a front-only output lift while claiming full multichannel processing;
- unvalidated bass management or channel remapping;
- inconsistent limiting across a surround bed.

A future multichannel mode must define channel grouping, linked detection, LFE policy, output ceilings, downmix behavior, and dedicated regression tests before it can replace this pass-through policy.

## DSP implementation rules

Real-time DSP changes should follow these rules unless measurements and listening evidence justify a deviation:

- Do not clear stateful filters merely because a coefficient changes.
- Reuse filter stages when topology is unchanged.
- Smooth continuous controls before applying them to DSP.
- Apply coefficient changes at a controlled block rate.
- Use subsystem dirty flags so unrelated EQ, color, width, compressor, limiter, or gain sections are not rebuilt.
- Avoid heap allocation, file access, logging, locks, or unbounded work in the steady-state audio path.
- Stop retuning after smoothed parameters reach their targets.
- Mask unavoidable preset topology changes with the shared transition processor.
- Keep color, compressor, and width state warm through neutral positions; make their audible contribution reach zero through mix or gain rather than abrupt enable thresholds.
- Clamp frequencies, Q values, gains, widths, ratios, times, and mixes to valid ranges.
- Keep gain staging explicit. Do not hide a large moving output gain inside tonal macros.
- Preserve center information before adding side enhancement.

## Gain-staging policy

The engine uses fixed creative-chain headroom before EQ, color, compression, width, and output protection. Only part of that headroom is restored automatically.

The intended result is:

- Filter ON sounds more polished and engaging;
- moderate-level material receives the calibrated wow effect;
- hot mastered material remains ceiling-safe without forcing an impossible constant +3 dB lift;
- limiter gain reduction does not become the primary loudness engine;
- Output Trim remains a user-controlled final adjustment.

Every review must include both:

1. **Native ON/OFF comparison** — confirms the intended product benefit.
2. **Loudness-matched comparison** — confirms tonal, spatial, and dynamic improvements remain after loudness bias is reduced.

## Multi-level loudness validation

Every factory preset is tested at input peaks of:

```text
-18 dBFS
-12 dBFS
 -6 dBFS
 -3 dBFS
 -1 dBFS
```

The matrix records:

- input and output RMS;
- input and output sample peak;
- RMS and peak benefit;
- crest-factor loss;
- 95th-percentile gain reduction;
- finite output and clipping state.

Moderate rows retain the calibrated loudness target. Hot rows allow the loudness benefit to decrease while enforcing output ceiling, crest-factor, and gain-reduction limits. A preset must not pass by flattening every transient against the limiter.

## Control and transition validation

The shared transition processor used by OBS must also be the implementation exercised by standalone tests.

Test at least:

- 44.1, 48, and 96 kHz;
- 64, 128, 256, 480, and 1024-frame blocks;
- all factory presets;
- preset requests every 2, 10, and 20 ms;
- bypass changes during active audio;
- a new preset request while a prior transition is still active.

Acceptance gates:

- no NaN or infinity values;
- no muted block remainder;
- maximum discontinuity ratio normally no greater than 1.10× the static reference;
- no allocation after initialization;
- no channel mismatch in the processed stereo pair.

A passing metric does not override an audible click or dropout.

## Selective-rebuild and hard-bypass validation

Regression tests must confirm:

- Stereo Magic changes rebuild only the width subsystem;
- Output Trim changes update output gain and limiter configuration without rebuilding EQ, color, width, or compression;
- bypass-only changes do not rebuild creative DSP configuration;
- settled bypass produces sample-exact pass-through;
- settled bypass performs no steady-state heap allocation;
- leaving hard bypass resets and re-enters through the bypass smoother instead of exposing stale DSP state.

## Dynamics and pumping validation

Use material that exposes gain-envelope problems:

- bass-heavy music with repeated kick transients;
- sustained vocal over changing low-frequency energy;
- sparse percussion with long decays;
- dense full-range program material;
- spoken voice with pauses and plosives.

Listen for vocal or treble rocking after kicks, full-mix breathing, long recovery swells, limiter chatter, and stereo-image movement caused by asymmetric gain behavior.

Preferred correction order:

1. reduce unnecessary makeup gain;
2. reduce full-band parallel density or ratio;
3. adjust attack and release for calmer recovery;
4. improve gain staging before adding limiting;
5. add source-aware processing only when simpler corrections are insufficient.

## CPU validation

Validate at least:

- 44.1 and 48 kHz normal operation;
- continuous playback while moving one and several controls;
- repeated preset changes;
- multiple plugin instances when practical;
- idle playback after controls settle;
- fully settled bypass.

Reject a change when it causes dropouts, continuous rebuilds after controls stop, recurring steady-state allocation, avoidable per-sample calculations, or a large CPU increase without documented audible benefit.

Global oversampling is not the default solution. Any oversampling proposal requires explicit quality measurements, CPU data, and a lower-cost comparison.

## Stereo validation

Test true mono, centered vocal with stereo accompaniment, already-wide music, strongly correlated and inverted material, headphones, speakers, and mono fold-down.

Acceptance criteria:

- center remains stable during widening;
- neutral position preserves the source stereo image;
- far-left Stereo Magic narrows predictably;
- far-right widens without obvious phase swirl or hollow center;
- mono fold-down does not reveal severe cancellation.

## Macro-control validation

For each macro, test `0 → 50 → 100 → 50` while audio is continuous.

- **Smart Bass:** cleanup must not sound thin or phasey; boost must not create uncontrolled low-frequency level or pumping.
- **Smart Treble:** cleanup must not dull the source completely; boost must avoid brittle peaks and exaggerated sibilance.
- **Vocal Body:** tuck and forward movement must remain predictable without honk or excessive output gain.
- **Stereo Magic:** narrowing and widening must remain progressive and center-safe.

## Maintained test commands

```bash
cmake -S . -B build-test -DBUILD_OBS_PLUGIN=OFF -DBUILD_STANDALONE_TESTS=ON
cmake --build build-test --config Release
./build-test/arsonkupik_dsp_smoke
./build-test/arsonkupik_dsp_hardening
./build-test/arsonkupik_transition_hardening
```

On a multi-configuration Windows generator, executables are normally under `build-test/Release/`.

## Listening-test record

Document:

- ArSonKuPik and OBS Studio versions;
- operating system and sample rate;
- source type and channel layout;
- preset and macro values;
- Smart Protect, Output Trim, bypass, and filter state;
- observed peaks and whether the comparison was loudness-matched;
- headphones, speakers, or both.

Use several source types before changing a global preset or engine coefficient.

## Release quality gate

Before a DSP-affecting release:

- [ ] multi-level loudness matrix passes;
- [ ] realtime hardening passes;
- [ ] preset and bypass transition hardening passes;
- [ ] selective rebuild and hard-bypass tests pass;
- [ ] Windows native plugin build passes against the pinned OBS dependency;
- [ ] Linux plugin build passes;
- [ ] native OBS listening checks pass;
- [ ] tag, CMake version, release notes, and website version metadata agree;
- [ ] release artifacts include build metadata and SHA-256 checksums.
