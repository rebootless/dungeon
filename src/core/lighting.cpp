#include "lighting.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include <SDL2/SDL.h>

#include "layout.h"
#include "tiles.h"

namespace {

struct LightSource {
    int cx, cy; // pixel-space center, map-local
    int radiusPx;
    SDL_Color color;
};

// Player's own light — fixed for now; a future item/spell system could
// vary these per player state instead of this one constant pair.
constexpr int        PLAYER_LIGHT_RADIUS_CELLS = 5;
constexpr SDL_Color  PLAYER_LIGHT_COLOR        = {255, 255, 255, 255};

// Every LIGHT_MARKER cell emits an identical light for now — see
// editor_state.h's comment on why only one marker flavor exists yet.
constexpr int        MARKER_LIGHT_RADIUS_CELLS = 4;
constexpr SDL_Color  MARKER_LIGHT_COLOR        = {255, 255, 255, 255};

/*
Ordered (Bayer) dithering, at native 1-pixel granularity everywhere —
no growing blocks. Most of a source's radius (INNER_SOLID_FRAC) is
plain solid color; only the outer band from there to the radius fades
out, and it does so as a per-pixel dot pattern whose density drops from
~100% at the band's inner edge to 0% at the radius, rather than a
smooth gradient or a coarsening checkerboard.
*/
constexpr float INNER_SOLID_FRAC = 0.72f;

constexpr int BAYER_N = 8;
constexpr uint8_t BAYER8[BAYER_N][BAYER_N] = {
    { 0, 32,  8, 40,  2, 34, 10, 42},
    {48, 16, 56, 24, 50, 18, 58, 26},
    {12, 44,  4, 36, 14, 46,  6, 38},
    {60, 28, 52, 20, 62, 30, 54, 22},
    { 3, 35, 11, 43,  1, 33,  9, 41},
    {51, 19, 59, 27, 49, 17, 57, 25},
    {15, 47,  7, 39, 13, 45,  5, 37},
    {63, 31, 55, 23, 61, 29, 53, 21},
};

// Per-pixel threshold in (0, 1) from the 8x8 Bayer matrix, tiled across
// the whole map — this is what turns a density value into a hard
// lit/unlit decision without ever grouping neighboring pixels together.
float bayerThreshold(int px, int py) {
    return (BAYER8[py % BAYER_N][px % BAYER_N] + 0.5f) / 64.0f;
}

/*
Stamps one light source into outPixels/bestT, walking only its own
bounding box — cheap even with several sources on screen at once, since
none of them ever touch a pixel outside their own radius. `bestT` tracks
which source currently owns each pixel (by distance fraction) so two
overlapping lights resolve to whichever one is actually closer there,
rather than whichever was stamped last.
*/
void stampLight(const LightSource& src, int mapW, int mapH,
                 uint8_t* outPixels, std::vector<float>& bestT) {
    int minX = std::max(0, src.cx - src.radiusPx);
    int maxX = std::min(mapW - 1, src.cx + src.radiusPx);
    int minY = std::max(0, src.cy - src.radiusPx);
    int maxY = std::min(mapH - 1, src.cy + src.radiusPx);
    if (minX > maxX || minY > maxY) return;

    float radiusF = static_cast<float>(src.radiusPx);

    for (int py = minY; py <= maxY; ++py) {
        for (int px = minX; px <= maxX; ++px) {
            float dx = static_cast<float>(px - src.cx);
            float dy = static_cast<float>(py - src.cy);
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist >= radiusF) continue;

            float t = dist / radiusF; // [0, 1)

            if (t > INNER_SOLID_FRAC) {
                // Density falls linearly from 1 (band's inner edge) to 0
                // (the radius itself); comparing it against the tiled
                // Bayer threshold is what turns that into a dot pattern
                // instead of a translucent fade.
                float bandT   = (t - INNER_SOLID_FRAC) / (1.0f - INNER_SOLID_FRAC);
                float density = 1.0f - bandT;
                if (bayerThreshold(px, py) >= density) continue;
            }

            size_t idx = static_cast<size_t>(py) * mapW + px;
            if (t >= bestT[idx]) continue; // a closer source already owns this pixel
            bestT[idx] = t;

            uint8_t* p = outPixels + idx * 4;
            p[0] = src.color.r;
            p[1] = src.color.g;
            p[2] = src.color.b;
            p[3] = 255;
        }
    }
}

} // namespace

void buildLightMask(const Level& level, int playerX, int playerY, uint8_t* outPixels) {
    const int mapW = MAP_PIXEL_W;
    const int mapH = MAP_PIXEL_H;

    // Fully dark by default — every pixel no light source reaches stays
    // this way, which is the whole point of the `lightMap` flag.
    for (int i = 0; i < mapW * mapH; ++i) {
        uint8_t* p = outPixels + i * 4;
        p[0] = p[1] = p[2] = 0;
        p[3] = 255;
    }

    std::vector<float> bestT(static_cast<size_t>(mapW) * mapH, 2.0f); // above any valid t

    LightSource player{
        playerX * CELL_SIZE + CELL_SIZE / 2,
        playerY * CELL_SIZE + CELL_SIZE / 2,
        PLAYER_LIGHT_RADIUS_CELLS * CELL_SIZE,
        PLAYER_LIGHT_COLOR
    };
    stampLight(player, mapW, mapH, outPixels, bestT);

    for (int y = 0; y < level.height; ++y) {
        for (int x = 0; x < level.width; ++x) {
            if (level.lightMarkerMap[y][x] != LIGHT_MARKER) continue;

            LightSource marker{
                x * CELL_SIZE + CELL_SIZE / 2,
                y * CELL_SIZE + CELL_SIZE / 2,
                MARKER_LIGHT_RADIUS_CELLS * CELL_SIZE,
                MARKER_LIGHT_COLOR
            };
            stampLight(marker, mapW, mapH, outPixels, bestT);
        }
    }
}
