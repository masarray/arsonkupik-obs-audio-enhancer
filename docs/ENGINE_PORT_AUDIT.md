# Engine Port Audit

## Extension findings

Important files in the uploaded extension:

- `src/shared/presets.js`: factory presets, EQ bands, compressor, color, width, output recipes.
- `src/offscreen/offscreen.js`: live Web Audio graph and DSP behavior.
- `src/studio/studio.js/css`: studio UI and live editing surface.

## Extension engine chain

The extension graph is conceptually:

1. Source / capture
2. Input analyser and input gain
3. Smart headroom gain
4. Safety HPF
5. EQ biquad chain
6. Parallel compressor
7. Color psychoacoustic engine
8. Source-aware / phase-safe width engine
9. Smart makeup
10. Limiter drive
11. Soft clipper
12. Limiter
13. Output gain / bypass crossfade / meters

## MasAri Golden Reference

MasAri is the base auditory DNA. Its strength is not maximum bass; it is the perceived combination:

- 25 Hz HPF for sub safety
- 76 Hz low-body lift
- 170 Hz vocal/acoustic body
- 325 Hz mud clean
- 490 Hz body guard
- 2180 Hz mid detail
- 6250 Hz tickle
- 12650 Hz open air shelf
- width 153 and highWidth 200
- godParticles 92
- vocalTickle 67
- air 48.8

## Native OBS mapping

| Extension module | Native implementation |
| --- | --- |
| Web Audio BiquadFilterNode | RBJ biquad filters per channel |
| Cut slope stacking | Butterworth Q stage stacking |
| DynamicsCompressorNode | Native envelope compressor with parallel dry/wet |
| WaveShaperNode | Native tanh saturation layers |
| Color graph | Smart bass, warmth, presence, air, mid anchor, tickle, treble skin, deharsh |
| Width graph | Generated phase-safe side from filtered mid copy, added +Side/-Side |
| Limiter | Soft clipper + peak gain cell |
| Bypass | Smooth crossfade |

## Porting philosophy

The native plugin is not a 1:1 Web Audio node clone. Browser and native DSP will not null-test exactly. The target is **perceptual match**:

- bass remains soft and breathing
- stereo enhancement remains wide without hollow center
- treble remains airy/silky, not harsh
- vocal/body remains anchored
- limiter protects live use

