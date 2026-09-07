#include "generator_mode.h"

#include "../core/display.h"
#include "../core/layout.h"
#include "../core/level.h"
#include "../core/renderer.h"
#include "../core/tiles.h"
#include "../help/help_mode.h"
#include "../settings/settings_mode.h"
#include "dungeon_generator.h"
#include "fragment.h"

namespace {

// Filled fresh by generate() every [R] press; onRender() just draws
// whatever's in here, exactly like GameMode draws gCurrentLevel's layers.
GeneratedDungeon dungeon_;
bool             hasDungeon_ = false;

void generate() {
    hasDungeon_ = generateDungeon(dungeon_);
}

} // namespace

/*
onEnter() re-scans fragments/ every time this mode is entered, so a
fragment saved (or edited) since the last visit is picked up without
needing a restart — same reasoning as EditorMode's onEnter() reloading
from disk.
*/
void GeneratorMode::onEnter() {
    initFragments();
    generate();
}

void GeneratorMode::onEvent(const SDL_Event& e) {
    if (e.type != SDL_KEYDOWN) return;

    switch (e.key.keysym.scancode) {
        case SDL_SCANCODE_R:
            generate();
            return;

        case SDL_SCANCODE_H:
            HelpMode::setReturnMode(HelpMode::ReturnMode::Generator);
            context_.switchMode(std::make_unique<HelpMode>());
            return;

        case SDL_SCANCODE_Q:
            context_.requestQuit();
            return;

        default:
            return;
    }
}

/*
onRender
Same frame/panel geometry as GameMode/EditorMode (see layout.h) so the
canvas doesn't visibly jump around switching to/from this mode, but the
panels and info box are left completely blank — no HUD, no control
legend, nothing but the assembled map itself. Collision/occlusion are
logical-only here too, same as GameMode: neither is drawn.
*/
void GeneratorMode::onRender() {
    const int mapOriginX    = MAP_ORIGIN_X;
    const int mapRightEdgeX = MAP_RIGHT_EDGE_X;
    const int leftDividerX  = LEFT_DIVIDER_X;
    const int totalW        = CANVAS_W;
    const int totalH        = CANVAS_H;

    /*
    Frame
    Marked once, up front, so every layer's draw() call classifies corners
    and T-junctions against the whole frame regardless of which piece it's
    actually drawing this pass — identical geometry to GameMode's/
    EditorMode's. See game_mode.cpp's render() for the full rationale.
    */
    FrameBuilder fb;
    fb.markRow(0, 0, totalW, FRAME_LAYER_WINDOW);
    fb.markRow(totalH - CELL_SIZE, 0, totalW, FRAME_LAYER_WINDOW);
    fb.markCol(0, 0, totalH, FRAME_LAYER_WINDOW);
    fb.markCol(totalW - CELL_SIZE, 0, totalH, FRAME_LAYER_WINDOW);
    fb.markCol(leftDividerX, 0, totalH, FRAME_LAYER_PANELS);
    fb.markCol(mapRightEdgeX, 0, totalH, FRAME_LAYER_PANELS);
    fb.markRow(MAP_BOTTOM_Y, mapOriginX, mapRightEdgeX, FRAME_LAYER_MAP);

    // Layer 1: window edges.
    fb.draw(FRAME_LAYER_WINDOW);

    // Layer 2: side panels — left blank, same as the info box; only the
    // dividers themselves need drawing.
    fb.draw(FRAME_LAYER_PANELS);

    // Layer 3: game panel
    setMapOrigin(mapOriginX);
    setMapClip(true);

    if (hasDungeon_) {
        // Render order: Ground → Objects → Entities, matches GameMode.
        for (int y = 0; y < MAX_HEIGHT; ++y)
            for (int x = 0; x < MAX_WIDTH; ++x)
                if (dungeon_.groundMap[y][x] != EMPTY_ID)
                    drawMapChar(dungeon_.groundMap[y][x], x, y);

        for (int y = 0; y < MAX_HEIGHT; ++y)
            for (int x = 0; x < MAX_WIDTH; ++x)
                if (dungeon_.objectMap[y][x] != EMPTY_ID)
                    drawMapChar(dungeon_.objectMap[y][x], x, y);

        for (int y = 0; y < MAX_HEIGHT; ++y)
            for (int x = 0; x < MAX_WIDTH; ++x)
                if (dungeon_.entityMap[y][x] != EMPTY_ID)
                    drawMapChar(dungeon_.entityMap[y][x], x, y);
    }

    setMapClip(false);

    // Map/info-box divider — sits in its own dedicated row below the map
    // (layout.h's MAP_BOTTOM_Y), so drawing it here rather than up front
    // with the outer edges is purely for consistency with the other
    // modes, not because it needs to overlap anything.
    fb.draw(FRAME_LAYER_MAP);

    // Layer 4: text panel — left blank, nothing to draw.
}
