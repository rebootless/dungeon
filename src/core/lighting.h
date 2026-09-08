#pragma once

#include <cstdint>

#include "level.h"

/*
Lighting
Builds a per-pixel light mask for a `lightMap`-flagged Level: the player's
own light plus one fixed-radius/fixed-color light per LIGHT_MARKER cell
(core/tiles.h), combined and stamped as a retro, ordered-dither falloff
rather than a smooth gradient — each ring further from a source's center
uses a coarser pixel grid, collapsing to solid black once past the
source's radius. core/renderer.h's drawLightMask() then multiplies this
mask over the already-rendered map (SDL_BLENDMODE_MOD), so a fully-lit
pixel leaves the tile underneath unchanged and a dark one gets crushed
toward black.

GameMode is the only caller (game/game_mode.cpp's render()) — it skips
this module entirely whenever the current level's `lightMap` is false, so
every other location's rendering cost is completely unaffected.
*/

// buildLightMask
// Fills `outPixels` (RGBA32, MAP_PIXEL_W * MAP_PIXEL_H pixels, row-major,
// 4 bytes per pixel — see core/layout.h) with the combined light mask for
// `level`'s LIGHT_MARKER cells plus a light centered on (playerX, playerY)
// in cell coordinates. Caller owns the buffer.
void buildLightMask(const Level& level, int playerX, int playerY, uint8_t* outPixels);
