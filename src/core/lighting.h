#pragma once

#include <cstdint>

#include "level.h"

/*
Lighting
Builds a per-pixel light mask covering a `width` x `height` grid of cells:
one light centered on (playerX, playerY) plus one fixed-radius/fixed-color
light per LIGHT_MARKER cell (core/tiles.h) found in `lightMarkerMap`,
combined and stamped as a retro, ordered-dither falloff rather than a
smooth gradient — each ring further from a source's center uses a coarser
pixel grid, collapsing to solid black once past the source's radius.
core/renderer.h's drawLightMask() then multiplies this mask over the
already-rendered map (SDL_BLENDMODE_MOD), so a fully-lit pixel leaves the
tile underneath unchanged and a dark one gets crushed toward black.

Takes a raw marker layer plus its own width/height rather than a whole
Level, since GameMode (core/level.h's Level), EditorMode/FragmentEditorMode
(their own ed-prefixed/fr-prefixed arrays), and GeneratorMode
(dungeon_generator.h's GeneratedDungeon) each keep this layer in a
differently-shaped container — none of that structure matters here beyond
which cells hold LIGHT_MARKER.
GameMode only calls this while the current level's `lightMap` is true; the
other three modes gate it behind their own local preview toggle instead
(console's /lightMap — core/app.cpp), since none of them has a single
"current level" to persist an authored flag on.
*/

// buildLightMask
// Fills `outPixels` (RGBA32, MAP_PIXEL_W * MAP_PIXEL_H pixels, row-major,
// 4 bytes per pixel — see core/layout.h) with the combined light mask for
// every LIGHT_MARKER cell in the first `width` columns / `height` rows of
// `lightMarkerMap`, plus a light centered on (playerX, playerY) in cell
// coordinates. Caller owns the buffer.
void buildLightMask(const TileID lightMarkerMap[MAX_HEIGHT][MAX_WIDTH], int width, int height,
                     int playerX, int playerY, uint8_t* outPixels);
