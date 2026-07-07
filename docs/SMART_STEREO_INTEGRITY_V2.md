# Smart Stereo Integrity Engine v2

Goal: keep vocal/lead center solid while making the original stereo instruments feel more polished, alive and 3D without phasey artifacts.

## Why this exists

The old stereo engine was mostly a generated-side enhancer: it generated a side signal from Mid, then added it as +Side/-Side. That creates width, but it does not know whether the Mid content is vocal, snare, bass, piano or a lead instrument.

The new direction is different:

- Preserve the dry L/R image.
- Use original Side content as the main source of width.
- Keep generated side very small and mostly high/air only.
- Detect likely vocal/center dominance and back off generated-side widening.
- Detect pan/call-response material and preserve it instead of forcing it toward center.
- Reduce only risky sustained anti-phase low side; do not mono all bass.
- Use correlation guard inside the DSP decision, not only as a meter.

## Engine policy

### Center/vocal/lead

Center is not stem-separated. It is protected with a probability model based on Mid/Side dominance, transient score and stereo ratio. If a vocal/lead-like center is likely:

- Side widening in 700 Hz - 3.6 kHz is reduced.
- Generated side from Mid is strongly reduced.
- A tiny Mid anchor is applied.
- No time shift is applied to the dry center.

### Instruments and call-response stereo

If the source already has strong but safe side content, the engine treats it as an intentional musical stereo event:

- Original pan is preserved.
- Original side bands are polished.
- Generated side is reduced so the engine does not smear the arrangement.
- High side-air can still be enhanced for a live/3D feeling.

### Low-frequency stereo

The engine no longer treats all bass as something that must become mono.

- Transient low stereo can be preserved, useful for stereo drums, toms and room hits.
- Sustained low side with low/negative correlation is reduced.
- Sub side reduction is dynamic, not hard mono.

### Color/harmonic policy

Harmonic wet is intentionally conservative. Detail and polish should come from:

- dry energy
- center anchor
- real-side multiband polish
- side-air
- transient/pan-aware enhancement

not from excessive harmonic saturation, which can make audio thick but dull.

## Test result

Standalone DSP smoke test passed after the v2 change.

Example smoke output:

```text
preset=MasAri
input_peak_db=-24.2728
output_peak_db=-19.5652
gain_reduction_db=3.59948
correlation=0.738099
```

This means the default path builds, runs, keeps output finite and gives a clear loudness benefit over the test input while maintaining healthy correlation.
