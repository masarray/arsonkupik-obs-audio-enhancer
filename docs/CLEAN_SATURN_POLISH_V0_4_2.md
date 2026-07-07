# ArSonKuPik v0.4.2 — Clean Saturn-Style Polish

Target: every preset should give an obvious quality and level benefit over raw audio while avoiding the broken/distorted character caused by excessive harmonic wet/saturation.

## Main DSP changes

- Harmonic/saturation is no longer the main loudness source.
- Bass enhancement is now mostly clean body/punch EQ polish, not heavy tanh saturation.
- Presence and air are cleaner: less harmonic wet, more deharsh + side-air polish.
- The soft clipper in the limiter is now safety-only; normal audio does not constantly pass through a distortion curve.
- Output makeup is applied before the limiter, so presets can sound louder while final peaks remain protected.
- No global oversampling is used by default. CPU stays OBS-friendly.
- Oversampling is avoided by design: nonlinear processing is micro-parallel, band-limited, and very low wet.

## Audio direction

MasAri / music presets:
- louder than raw
- more polished and alive
- bass more solid without fuzzy distortion
- top end cleaner/silkier
- stereo engine still preserves center and original L/R call-response

Podcast:
- centered vocal
- clean compression and air
- no stereo stimulation
- no saturation crackle

Night Listening:
- relaxed and warm
- no longer extremely quiet
- no harsh harmonic texture

## Important design rule

Do not use saturation to chase loudness. Loudness must come from smart compression, clean makeup, EQ polish, and final safety limiting.
