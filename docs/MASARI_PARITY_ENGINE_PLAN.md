# MasAri parity engine plan

Target: make the native OBS filter sound clearly better than raw bypass and closer to the Chrome extension reference.

Planned DSP fixes:

1. Smart headroom before enhancement.
2. Adaptive makeup gain after color/width and before limiter.
3. Loudness-matched A/B so OBS filter ON is not perceived as smaller than bypass.
4. Stronger mid/body anchor.
5. More natural side-air and high-tickle layers.
6. Safer limiter feedback so loudness returns without clipping.

This plan is the basis for v0.4.0.
