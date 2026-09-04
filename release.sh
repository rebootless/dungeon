#!/bin/bash
set -euo pipefail

rm -rf release
mkdir -p release
cd release

cmake .. \
    -DCMAKE_BUILD_TYPE=Release

cmake --build . -j"$(nproc)"

if [[ ! -f "dungeon" ]]; then
    echo "FAILED"
    exit 1
fi

# world/ and fragments/ (core/level.h's WORLD_DIR, generator/fragment.h's
# FRAGMENTS_DIR) live under assets/, so copying assets/ already brings
# both along — no separate copy needed for either.
rm -f assets

cp -r ../assets .

rm -rf CMakeFiles CMakeCache.txt cmake_install.cmake Makefile

echo "SUCCESS"
