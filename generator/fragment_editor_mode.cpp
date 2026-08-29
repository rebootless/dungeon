#include "fragment_editor_mode.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "../core/display.h"
#include "../core/layout.h"
#include "../core/level.h"
#include "../core/renderer.h"
#include "../core/tiles.h"
#include "fragment.h"
#include "fragment_editor_panel.h"
#include "fragment_editor_state.h"

/*
Fragment-editor map state
Defined here (the ONE translation unit that allocates them); declared
`extern` in fragment_editor_state.h for fragment_editor_controls.cpp to
reach.
*/
TileID frGroundMap    [MAX_HEIGHT][MAX_WIDTH];
TileID frObjectMap    [MAX_HEIGHT][MAX_WIDTH];
TileID frEntityMap    [MAX_HEIGHT][MAX_WIDTH];
TileID frCollisionMap [MAX_HEIGHT][MAX_WIDTH];
TileID frOcclusionMap [MAX_HEIGHT][MAX_WIDTH];
TileID frConnectorMap [MAX_HEIGHT][MAX_WIDTH];
FragmentMultiTileCell frMtMap[MAX_HEIGHT][MAX_WIDTH];

// Tile palette
std::vector<TileID> frAvailableTiles;
TileID               frSelectedTile = makeTile(1, 2);

// Layer management
FragmentEditLayer frActiveLayer = FragmentEditLayer::GROUND;

TileID (*frGetLayerMap(FragmentEditLayer layer))[MAX_WIDTH] {
    switch (layer) {
        case FragmentEditLayer::GROUND:    return frGroundMap;
        case FragmentEditLayer::OBJECTS:   return frObjectMap;
        case FragmentEditLayer::ENTITIES:  return frEntityMap;
        case FragmentEditLayer::COLLISION: return frCollisionMap;
        case FragmentEditLayer::OCCLUSION: return frOcclusionMap;
        case FragmentEditLayer::CONNECTOR: return frConnectorMap;
    }
    return frGroundMap;
}

FragmentEditLayer frLayerForTile(TileID id) {
    switch (getTileMeta(id).layer) {
        case LayerType::Ground:    return FragmentEditLayer::GROUND;
        case LayerType::Objects:   return FragmentEditLayer::OBJECTS;
        case LayerType::Entities:  return FragmentEditLayer::ENTITIES;
        case LayerType::Collision: return FragmentEditLayer::COLLISION;
    }
    return FragmentEditLayer::GROUND;
}

// Editor session state
std::string frEditorStatus    = "";
int         frEditorStatusTTL = 0;
int         frPaletteScroll   = 0;
bool        frMouseDown       = false;
bool        frRightMouseDown  = false;

// Fragment selection / size
int fragmentWidth  = 1;
int fragmentHeight = 1;
int selectedFragmentId  = 0;
int fragmentListScroll  = 0;
FragmentCollisionTool frActiveCollisionTool = FragmentCollisionTool::BLOCK;

/*
buildFragmentPalette
Same source as EditorMode's buildPalette — tiles.cpp's hand-maintained
registry (core/tiles.h's getPaletteTiles()).
*/
static void buildFragmentPalette() {
    frAvailableTiles = getPaletteTiles();
    if (!frAvailableTiles.empty()) frSelectedTile = frAvailableTiles[0];
}

/*
Palette layout (shelf packing)
Identical algorithm to editor/editor_mode.cpp's computePaletteLayout —
kept as its own instance since it packs frAvailableTiles into
FragmentPaletteEntry rects rather than EditorMode's PaletteEntry.
*/
std::vector<FragmentPaletteEntry> computeFragmentPaletteLayout(int panelW) {
    std::vector<FragmentPaletteEntry> entries;
    entries.reserve(frAvailableTiles.size());

    const int availW = panelW - PALETTE_MARGIN;
    int cursorX = 0, cursorY = 0, shelfH = 0;

    for (TileID id : frAvailableTiles) {
        TileMetadata meta = getTileMeta(id);
        int w = meta.w * PALETTE_CELL;
        int h = meta.h * PALETTE_CELL;

        if (cursorX > 0 && cursorX + w > availW) {
            cursorX = 0;
            cursorY += shelfH;
            shelfH = 0;
        }

        entries.push_back({ id, SDL_Rect{ PALETTE_MARGIN + cursorX, PALETTE_MARGIN + cursorY, w, h } });

        cursorX += w;
        shelfH = std::max(shelfH, h);
    }

    return entries;
}

/*
clearFragmentEditorMaps
A genuinely empty canvas — no border, no default content. Used both for
a fresh editor session and as F9's fallback when the selected id has no
saved file yet.
*/
static void clearFragmentEditorMaps() {
    for (int y = 0; y < MAX_HEIGHT; ++y) {
        for (int x = 0; x < MAX_WIDTH; ++x) {
            frGroundMap[y][x]    = EMPTY_ID;
            frObjectMap[y][x]    = EMPTY_ID;
            frEntityMap[y][x]    = EMPTY_ID;
            frCollisionMap[y][x] = EMPTY_ID;
            frOcclusionMap[y][x] = EMPTY_ID;
            frConnectorMap[y][x] = EMPTY_ID;
            frMtMap[y][x]        = { false, 0, 0 };
        }
    }
}

/*
rebuildFragmentMultiTileOccupancy
Same reasoning as EditorMode's rebuildMultiTileOccupancy — frMtMap is
editor-only state that never gets saved to JSON, so after F9 loads a
file into fr*Map, this reconstructs it by scanning for anchor tiles.
*/
static void rebuildFragmentMultiTileOccupancy() {
    for (int y = 0; y < MAX_HEIGHT; ++y)
        for (int x = 0; x < MAX_WIDTH; ++x)
            frMtMap[y][x] = { false, 0, 0 };

    auto scanLayer = [](TileID map[][MAX_WIDTH]) {
        for (int y = 0; y < MAX_HEIGHT; ++y) {
            for (int x = 0; x < MAX_WIDTH; ++x) {
                TileID id = map[y][x];
                if (id == EMPTY_ID) continue;

                TileMetadata meta = getTileMeta(id);
                if (meta.w <= 1 && meta.h <= 1) continue;

                for (int dy = 0; dy < meta.h; ++dy) {
                    for (int dx = 0; dx < meta.w; ++dx) {
                        if (dx == 0 && dy == 0) continue;
                        int cx = x + dx, cy = y + dy;
                        if (cx < MAX_WIDTH && cy < MAX_HEIGHT)
                            frMtMap[cy][cx] = { true, x, y };
                    }
                }
            }
        }
    };
    scanLayer(frGroundMap);
    scanLayer(frObjectMap);
    scanLayer(frEntityMap);
}

/*
loadFragmentIntoEditor
F9: loads fragment `id` into the fr*Map layers. If nothing is saved
there yet, clears to a genuinely empty canvas.
*/
void loadFragmentIntoEditor(int id) {
    Fragment* fragment = findFragment(id);
    if (!fragment) {
        clearFragmentEditorMaps();
        fragmentWidth  = 1;
        fragmentHeight = 1;
        frEditorStatus    = "No saved fragment at this id \u2014 empty canvas";
        frEditorStatusTTL = 150;
        return;
    }

    for (int y = 0; y < MAX_HEIGHT; ++y) {
        for (int x = 0; x < MAX_WIDTH; ++x) {
            frGroundMap[y][x]    = fragment->tileMap[y][x];
            frObjectMap[y][x]    = fragment->objectMap[y][x];
            frEntityMap[y][x]    = fragment->entityMap[y][x];
            frCollisionMap[y][x] = fragment->collisionMap[y][x];
            frOcclusionMap[y][x] = fragment->occlusionMap[y][x];
            frConnectorMap[y][x] = fragment->connectorMap[y][x];
        }
    }
    fragmentWidth  = fragment->width;
    fragmentHeight = std::min(fragment->height, MAX_FRAGMENT_HEIGHT);
    rebuildFragmentMultiTileOccupancy();

    frEditorStatus    = std::string("Loaded \"") + fragment->name + "\"";
    frEditorStatusTTL = 150;
}

/*
saveFragmentEditorTo
F5: writes the fr*Map layers out to fragment `id`'s JSON file in
fragments/, and registers/updates the in-memory copy in `fragments`
immediately — so the right-panel list sees it without needing a restart.
*/
void saveFragmentEditorTo(int id) {
    Fragment& fragment = getOrCreateFragment(id);

    for (int y = 0; y < MAX_HEIGHT; ++y) {
        for (int x = 0; x < MAX_WIDTH; ++x) {
            fragment.tileMap[y][x]      = frGroundMap[y][x];
            fragment.objectMap[y][x]    = frObjectMap[y][x];
            fragment.entityMap[y][x]    = frEntityMap[y][x];
            fragment.collisionMap[y][x] = frCollisionMap[y][x];
            fragment.occlusionMap[y][x] = frOcclusionMap[y][x];
            fragment.connectorMap[y][x] = frConnectorMap[y][x];
        }
    }
    fragment.width  = fragmentWidth;
    fragment.height = fragmentHeight;
    if (strcmp(fragment.name, "New Fragment") == 0)
        snprintf(fragment.name, sizeof(fragment.name), "Fragment %d", id);

    bool ok = saveFragmentToFile(fragment, fragmentFileName(id).c_str());

    frEditorStatus    = ok ? (std::string(" Saved \"") + fragment.name + "\" to " + fragmentFileName(id))
                            : "Save FAILED \u2014 check fragments/ is writable";
    frEditorStatusTTL = 150;
}

/*
drawInfoStr
Info-box text helper (relative to map area, cell-grid coords), identical
to EditorMode's private helper of the same name.
*/
static void drawInfoStr(const std::string& text, int gx, int gy, int originX) {
    drawStringPx(text, originX + gx * CELL_SIZE, gy * CELL_SIZE);
}

/*
IMode
Tries to load fragment 0 — selectedFragmentId's initial value — and if
nothing is saved there, leaves the canvas genuinely empty (same
convention as EditorMode::onEnter()).
*/
void FragmentEditorMode::onEnter() {
    buildFragmentPalette();
    initFragments();
    loadFragmentIntoEditor(selectedFragmentId);
}

void FragmentEditorMode::onRender() {
    const int panelH = CANVAS_H;

    const int mapOriginX    = MAP_ORIGIN_X;
    const int mapRightEdgeX = MAP_RIGHT_EDGE_X;

    const int panelContentW = PANEL_W;
    const int leftDividerX  = LEFT_DIVIDER_X;
    const int rightPanelX   = RIGHT_PANEL_X;

    const int totalW = CANVAS_W;
    const int totalH = CANVAS_H;

    // Palette — identical layout to EditorMode's
    {
        setClipRect(0, 0, panelContentW, panelH);

        for (const FragmentPaletteEntry& entry : computeFragmentPaletteLayout(panelContentW)) {
            const SDL_Rect& r = entry.rect;
            int px = r.x;
            int py = r.y - frPaletteScroll;

            if (py + r.h > 0 && py < panelH)
                drawTileRect(entry.id, SDL_Rect{px, py, r.w, r.h});

            if (entry.id == frSelectedTile)
                drawRectOutline(px, py, r.w, r.h, SDL_Color{255, 255, 0, 255});
        }

        clearClipRect();
    }

    /*
    Right panel: fragment list
    The F5/F9 target (selectedFragmentId) plus every fragment already
    saved to fragments/ — doubles as a picker (click a row, or
    PageUp/PageDown to reach an id that isn't saved yet) and a status
    readout of what's on disk right now.
    */
    {
        setClipRect(rightPanelX, 0, panelContentW, panelH);

        std::vector<int> ids = listFragmentIds();
        bool selectedExists = std::find(ids.begin(), ids.end(), selectedFragmentId) != ids.end();

        std::string header = "Fragment " + std::to_string(selectedFragmentId)
            + " (" + std::to_string(fragmentWidth) + "x" + std::to_string(fragmentHeight) + ")"
            + (selectedExists ? "" : " [new]");
        drawInfoStr(header, 0, FragmentEditorPanel::ROW_FRAGMENT_HEADER, rightPanelX);
        drawInfoStr(FragmentEditorPanel::FRAGMENT_LEGEND, 0, FragmentEditorPanel::ROW_FRAGMENT_LEGEND, rightPanelX);

        for (size_t i = 0; i < ids.size(); ++i) {
            int id = ids[i];
            int py = PALETTE_MARGIN + CELL_SIZE * (FragmentEditorPanel::FRAGMENT_LIST_START_ROW + (int)i) - fragmentListScroll;
            if (py + CELL_SIZE <= 0 || py >= panelH) continue;

            bool isSelected = (id == selectedFragmentId);
            const Fragment* fragment = findFragment(id);

            std::string row = " " + std::to_string(id) + " " + (fragment ? fragment->name : "?");

            SDL_Color color = isSelected ? SDL_Color{255, 255, 0, 255} : SDL_Color{200, 200, 200, 255};
            if (isSelected)
                drawRectOutline(rightPanelX, py, panelContentW, CELL_SIZE, SDL_Color{255, 255, 0, 255});
            drawStringPx(row, rightPanelX, py, color);
        }

        clearClipRect();
    }

    /*
    Everything drawn on the map itself — checkerboard through the
    connector overlay — is clipped to the map viewport. Without this,
    content painted past MAX_FRAGMENT_HEIGHT (still reachable — see
    fragment_editor_controls.cpp's frPlaceTile, which never restricted
    painting to width x height) would bleed FRAGMENT_ROW_OFFSET pixels
    into the info box below once shifted.
    */
    setClipRect(mapOriginX, 0, MAP_PIXEL_W, MAP_PIXEL_H);

    /*
    Checkerboard background
    Drawn only under the fragment's current width x height footprint (not
    the full MAX_WIDTH x MAX_HEIGHT map area) — the boundary between
    checkerboard and the plain canvas background IS the size indicator,
    so no separate outline is drawn around it.
    */
    for (int y = 0; y < fragmentHeight && y < MAX_HEIGHT; ++y) {
        for (int x = 0; x < fragmentWidth && x < MAX_WIDTH; ++x) {
            int px = mapOriginX + x * CELL_SIZE;
            int py = FRAGMENT_ROW_OFFSET + y * CELL_SIZE;
            drawCanvasTile(SDL_Rect{px, py, CELL_SIZE, CELL_SIZE});
        }
    }

    // Map layers (Ground → Objects → Entities, matches game render order)
    for (int y = 0; y < MAX_HEIGHT; ++y) {
        for (int x = 0; x < MAX_WIDTH; ++x) {
            int px = mapOriginX + x * CELL_SIZE;
            int py = FRAGMENT_ROW_OFFSET + y * CELL_SIZE;

            drawTileRect(frGroundMap[y][x], SDL_Rect{px, py, CELL_SIZE, CELL_SIZE});
            drawTileRect(frObjectMap[y][x], SDL_Rect{px, py, CELL_SIZE, CELL_SIZE});
            drawTileRect(frEntityMap[y][x], SDL_Rect{px, py, CELL_SIZE, CELL_SIZE});
        }
    }

    /*
    Collision overlay — same three marker colors as EditorMode.
    */
    for (int y = 0; y < MAX_HEIGHT; ++y) {
        for (int x = 0; x < MAX_WIDTH; ++x) {
            TileID marker = frCollisionMap[y][x];
            SDL_Color color;
            if      (marker == COLLISION_MARKER)   color = SDL_Color{255, 220, 0, 200};
            else if (marker == STAIRS_UP_MARKER)   color = SDL_Color{80, 220, 255, 200};
            else if (marker == STAIRS_DOWN_MARKER) color = SDL_Color{230, 90, 255, 200};
            else continue;

            drawRectOutline(mapOriginX + x * CELL_SIZE, FRAGMENT_ROW_OFFSET + y * CELL_SIZE,
                             CELL_SIZE, CELL_SIZE, color);
        }
    }

    // Occlusion overlay — same translucent violet outline as EditorMode.
    for (int y = 0; y < MAX_HEIGHT; ++y) {
        for (int x = 0; x < MAX_WIDTH; ++x) {
            if (frOcclusionMap[y][x] != OCCLUSION_MARKER) continue;
            drawRectOutline(mapOriginX + x * CELL_SIZE, FRAGMENT_ROW_OFFSET + y * CELL_SIZE,
                             CELL_SIZE, CELL_SIZE, SDL_Color{170, 100, 255, 200});
        }
    }

    /*
    Border overlay
    Automatically derived from width/height every frame — not a stored
    marker, so resizing the fragment (arrow keys) never leaves stale
    border cells behind. This rectangle is exactly what
    generator/dungeon_generator.cpp checks for overlap when placing
    fragments on the shared canvas: two fragments may only ever touch
    along this edge, never overlap inside it.
    */
    for (int x = 0; x < fragmentWidth && x < MAX_WIDTH; ++x) {
        drawRectOutline(mapOriginX + x * CELL_SIZE, FRAGMENT_ROW_OFFSET,
                         CELL_SIZE, CELL_SIZE, SDL_Color{255, 60, 60, 200});
        drawRectOutline(mapOriginX + x * CELL_SIZE, FRAGMENT_ROW_OFFSET + (fragmentHeight - 1) * CELL_SIZE,
                         CELL_SIZE, CELL_SIZE, SDL_Color{255, 60, 60, 200});
    }
    for (int y = 0; y < fragmentHeight && y < MAX_HEIGHT; ++y) {
        drawRectOutline(mapOriginX, FRAGMENT_ROW_OFFSET + y * CELL_SIZE,
                         CELL_SIZE, CELL_SIZE, SDL_Color{255, 60, 60, 200});
        drawRectOutline(mapOriginX + (fragmentWidth - 1) * CELL_SIZE, FRAGMENT_ROW_OFFSET + y * CELL_SIZE,
                         CELL_SIZE, CELL_SIZE, SDL_Color{255, 60, 60, 200});
    }

    /*
    Connector overlay
    Candidate stitching points for the procedural generator — a distinct
    green outline, drawn after (so it wins over) the red border above,
    since a connector cell normally sits exactly on that border.
    */
    for (int y = 0; y < MAX_HEIGHT; ++y) {
        for (int x = 0; x < MAX_WIDTH; ++x) {
            if (frConnectorMap[y][x] != CONNECTOR_MARKER) continue;
            drawRectOutline(mapOriginX + x * CELL_SIZE, FRAGMENT_ROW_OFFSET + y * CELL_SIZE,
                             CELL_SIZE, CELL_SIZE, SDL_Color{60, 255, 120, 200});
        }
    }

    clearClipRect();

    // Frame — identical geometry to EditorMode's.
    {
        FrameBuilder fb;
        fb.markRow(0, 0, totalW);
        fb.markRow(totalH - CELL_SIZE, 0, totalW);
        fb.markCol(0, 0, totalH);
        fb.markCol(totalW - CELL_SIZE, 0, totalH);
        fb.markCol(leftDividerX, 0, totalH);
        fb.markCol(mapRightEdgeX, 0, totalH);
        fb.markRow((MAX_HEIGHT - 1) * CELL_SIZE, mapOriginX, mapRightEdgeX);
        fb.draw();
    }

    /*
    Info box text
    Just the transient save/load status now — the control legend that used
    to fill the rest of this box lives in HelpMode (see help/help_mode.cpp,
    reachable with [H]), reusing these same FragmentEditorPanel::HELP_*
    strings rather than duplicating them.
    */
    const int boxStartY = MAX_HEIGHT + 2;
    {
        if (frEditorStatusTTL > 0) {
            drawInfoStr(frEditorStatus, 1, boxStartY + FragmentEditorPanel::ROW_STATUS, mapOriginX);
            --frEditorStatusTTL;
        }
    }
}
