# Audio tuning v0.4 - Addictive loudness direction

Target: every preset should give an immediate audible benefit over raw OBS audio. Filter ON should not feel smaller than OBS bypass.

## Main changes

- Preset output trim is no longer heavily negative.
- Night Listening is no longer extremely quiet.
- Podcast is tuned for thick, close, airy and silky voice.
- MasAri default is louder, fuller, wider and more engaging.
- Headroom compensation is less conservative.
- Harmonic wet gain is intentionally controlled to avoid dull, smeared or less detailed audio.
- Detail is pushed more through dry energy, side-air, mid anchor, air and tickle layers rather than excessive harmonic saturation.

## Important tuning principle

Do not simply increase harmonic color wet amount. Too much harmonic wet can make audio feel thick but dull, smeared and less detailed.

Preferred direction:

```text
more dry energy
+ stronger mid/body anchor
+ controlled bass body
+ side-air sparkle
+ silky high air
+ limiter-safe output loudness
```

Avoid:

```text
excessive harmonic wet
muddy saturation
large negative output trim
night/podcast presets that collapse in loudness
```

## Expected behavior

- MasAri: louder than raw, bass/body fuller, treble silkier, stereo more enjoyable.
- Podcast: voice closer, thicker, airy, silky and easier to listen to.
- Night Listening: relaxed but not buried or very quiet.
- All presets: output should feel at least slightly louder and more polished than raw audio.

## v0.4.1 Smart Stereo Integrity update

This build adds a smarter stereo strategy. The stereo engine no longer relies mainly on generated side from Mid. It now analyzes and polishes the original Side signal per band.

Key behavior:

- Vocal/lead-like center content is protected.
- Instrument call-response left/right content is preserved.
- Low stereo is not blindly forced to mono.
- Sustained anti-phase low side is reduced dynamically.
- Generated side is reduced when the source already has intentional stereo movement.
- Harmonic wet is kept conservative to avoid dullness and loss of detail.
