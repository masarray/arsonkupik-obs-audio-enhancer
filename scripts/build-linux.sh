#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${1:-build}"
cmake -S "$ROOT" -B "$ROOT/$BUILD_DIR" -DBUILD_OBS_PLUGIN=ON -DBUILD_STANDALONE_TESTS=ON
cmake --build "$ROOT/$BUILD_DIR" --config Release
cmake --install "$ROOT/$BUILD_DIR" --config Release --prefix "$ROOT/package"
echo "Built package at: $ROOT/package"
