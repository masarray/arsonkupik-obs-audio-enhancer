# ArSonKuPik v0.4.3 — Extension Smart Makeup Benefit

Fixes after listening feedback:

- Removed the old **Manual macro tuning** checkbox. Sliders are always live.
- Preset selection still loads the preset recipe into the sliders.
- Added extension-style smart gain staging:
  - hot media gets temporary pre-chain headroom,
  - the reserved headroom is restored after color/width,
  - additional clean perceived loudness is added before the limiter,
  - makeup backs off when pre-limiter peaks or limiter GR become risky.
- Reduced harmonic/saturation as a source of loudness.
- Enhancement now comes mainly from smart compressor makeup, dry-energy restoration, clean side-air polish, mid anchor, and output makeup.
- Default OBS UI values are more obviously audible while staying CPU-safe.

Design rule: user must hear a clear ON benefit versus raw/bypass without crackle or heavy saturation.
