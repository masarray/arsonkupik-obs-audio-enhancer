#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cmake -S "$ROOT" -B "$ROOT/build-smoke" -DBUILD_OBS_PLUGIN=OFF -DBUILD_STANDALONE_TESTS=ON
cmake --build "$ROOT/build-smoke" --config Release
"$ROOT/build-smoke/arsonkupik_dsp_smoke"
