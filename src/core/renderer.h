#pragma once

#include <string>
#include <unordered_map>

#include <SDL2/SDL.h>

#include "tiles.h"

// Initialise SDL, create the game window + renderer, load assets. The
// logical render target is created lazily by the first beginFrame() call.
void initSDL();

// Destroy all SDL resources and shut down subsystems.
void cleanupSDL();

/*
Drops every cached tile texture (core/palette.h's applyActivePalette()
already ran on the ones being dropped) so the next drawChar/drawTileRect
call for a given file reloads and recolors it from disk against whichever
palette is active NOW. Called by settings/settings_mode.cpp right after
setActivePalette() picks a new one.
*/
void reloadTileTextures();

// The single SDL_Window owned by the app. Needed by core/app.cpp to call
// SDL_SetWindowSize when the /set resolution command runs.
SDL_Window* getWindow();

/*
Begin drawing into the logical canvas (not the real backbuffer directly).
Always exactly CANVAS_W x CANVAS_H (core/layout.h) — every mode builds
its frame at this one fixed virtual resolution regardless of the actual
window size; only the FINISHED frame gets scaled, in endFrame(). The
render target is allocated once, lazily, on the first call.
*/
void beginFrame();

/*
The logical canvas size — always CANVAS_W x CANVAS_H. Kept as a function
(rather than having callers just use the layout.h constants directly) so
core/console.cpp doesn't need to depend on layout.h itself.
*/
void getCanvasSize(int& w, int& h);

/*
Blit the logical canvas into the real window — scaled to the largest
integer factor that fits (core/display.h), centered, nearest-neighbor.
Any leftover letterbox space is filled with the same color the canvas
itself clears to, so it reads as part of the interface rather than a
black border.
*/
void endFrame();

// Blit the spritesheet tile that corresponds to TileID `c` at grid cell (x, y).
// Coordinates are raw pixel-grid cells — no zoom applied.
void drawChar(TileID c, int x, int y);

/*
Tiles assets/canvas.png (a checkerboard swatch, signalling "transparent"
the way image editors do) into an arbitrary pixel-space rect — used by
FragmentEditorMode to show which cells sit within the fragment's current
width x height footprint before any tiles are painted over them. A no-op
if the asset failed to load.
*/
void drawCanvasTile(SDL_Rect dst);

/*
Like drawChar, but with an arbitrary pixel-space destination rect instead
of a CELL_SIZE grid cell. Used by EditorMode for UI that isn't cell-aligned
(the palette preview scales sprites to fit, category tab labels are offset
a few px within their cell, etc). drawChar is a thin wrapper around this.
*/
void drawTileRect(TileID c, SDL_Rect dst);

/*
Like drawTileRect, but sources its rect from core/tiles.h's
getPalettePreviewMeta instead of getTileMeta — the one difference being
an autotile_blend representative renders its whole 3x3 base sheet here
instead of the single "c" piece it actually resolves to on the map. Used
only for the palette's own tile icons (editor/editor_mode.cpp's
onRender and the fragment editor's equivalent); every other draw call
(the map, the palette's own selection outline, FrameBuilder, ...) keeps
using plain drawTileRect/drawChar/drawMapChar.
*/
void drawTilePreview(TileID c, SDL_Rect dst);

// Render UTF-8 string `str` at grid cell (x, y) using the game font, in
// plain white.
void drawString(const std::string& str, int x, int y);

// Colored variant — used by the console to tell echoed input, success
// results, and error results apart at a glance (Minecraft-chat style).
void drawString(const std::string& str, int x, int y, SDL_Color color);

// Like drawString, but at an arbitrary pixel position instead of a grid
// cell. drawString is a thin wrapper around this.
void drawStringPx(const std::string& str, int px, int py);
void drawStringPx(const std::string& str, int px, int py, SDL_Color color);

// Fill an axis-aligned rect in raw logical-canvas pixel coordinates (not
// grid cells). Used for UI backgrounds such as the console overlay.
void fillRect(int px, int py, int pw, int ph, SDL_Color color);

// Outline (unfilled) variant of fillRect — used for selection highlights and
// the editor's collision-marker overlay.
void drawRectOutline(int px, int py, int pw, int ph, SDL_Color color);

// Clip subsequent drawing to an arbitrary pixel-space rect, or clear the
// clip.
void setClipRect(int px, int py, int pw, int ph);
void clearClipRect();

/*
Zoom & camera
In-game camera zoom. Unrelated to the interface scale / resolution in
core/display.h — this only affects drawMapChar.
*/

void setZoom(int level);
int  getZoom();

// Update the camera target used by drawMapChar.
void setMapCamera(int tileX, int tileY);

/*
Horizontal pixel offset added to every drawMapChar/setMapClip call —
GameMode reserves a left panel exactly like EditorMode, so the map itself
does not start at canvas x=0. Call once per frame with the current
mapOriginX (core/layout.h's MAP_ORIGIN_X) before drawing (see
game/game_mode.cpp's render()); defaults to 0. EditorMode doesn't use
drawMapChar at all (it draws tiles directly via drawTileRect with its own
explicit offsets), so this has no effect there.
*/
void setMapOrigin(int originX);

// Like drawChar, but zoom- and camera-aware.
void drawMapChar(TileID c, int x, int y);

// Enable / disable the SDL clip rect that confines map rendering to
// (mapOriginX, 0, MAP_PIXEL_W, MAP_PIXEL_H).
void setMapClip(bool enable);

/*
Blits `pixels` (RGBA32, MAP_PIXEL_W x MAP_PIXEL_H, row-major, 4 bytes per
pixel — see core/lighting.h's buildLightMask) over the currently-rendered
map with SDL_BLENDMODE_MOD: a white pixel in the mask leaves the tile
underneath unchanged, a black one crushes it toward black, matching
whatever color the mask stamped in between. Uses the exact same
mapOriginX/zoom/camera transform as drawMapChar, so the overlay lines up
with the map pixel-for-pixel at any zoom level. Call after the map's own
content is drawn and before anything (like the facing cursor) that should
stay readable regardless of darkness.
*/
void drawLightMask(const uint8_t* pixels);


/*
Destroys the currently loaded panel theme texture (if any), so the next
FrameBuilder::draw() call reloads it from disk under whichever theme
core/panel.h's setActivePanel() just switched to. Called by
settings/settings_mode.cpp right after setActivePanel() picks a new one —
same idea as reloadTileTextures() above, just for the one shared panel
sheet instead of a per-file cache.
*/
void reloadPanelTexture();

// Blits the 16x16 cell at (cellX, cellY) of the active panel theme's
// sheet (see core/panel.h) to `dst`. FrameBuilder::draw() is the only
// caller — see this header's "Frame system" comment for how cellX/cellY
// get chosen.
void drawPanelCell(int cellX, int cellY, SDL_Rect dst);

/*
Global border visibility toggle. When off, FrameBuilder::draw() draws
nothing at all — used by the "G" key (see core/app.cpp) to let borders be
hidden across every mode without each mode's onRender() having to know
about the toggle itself.
*/
void toggleBordersVisible();
bool areBordersVisible();

/*
Frame system
FrameBuilder draws panel and divider borders without each mode having to
hand-derive "corner vs edge" itself, and without special-casing where a
panel divider meets the outer border. Mark every cell that should carry
border art (outer edges, plus any interior divider lines), then draw()
decides the correct cell of the active panel theme's 96x96 sheet
(assets/panels/*.png, core/panel.h) for each marked cell purely from how
many of its 4 neighbours are also marked:

  2 opposite neighbours (left+right) -> a horizontal edge cell
  2 opposite neighbours (up+down)    -> a vertical edge cell
  anything else (a real corner, a T
  where a divider meets the outer
  edge, or a cross where two
  dividers meet)                     -> a corner cell

There are no T-junction or cross pieces in the theme art — a T or a cross
just resolves to whichever corner orientation its own two "inner" marked
neighbours match (see renderer.cpp's frameClassify()), the same as a real
corner would. Every theme sheet is a 6x6 grid of CELL_SIZE cells: the 4
true corners at the sheet's own 4 corners (already drawn facing inward,
so no runtime flip/rotate is needed — a cell classified as e.g. top-right
always sources the sheet's own top-right corner cell), the 2 cells next
to each corner along every side ("adjacent" cells, used for the edge
cell immediately touching a corner), and the 2 cells between those
("middle" cells, alternated to fill however much straight run is left).
A horizontal run reads its edge cells off the sheet's top row if its
bounding corners are top corners, its bottom row if they're bottom
corners — vertical runs read the left or right column the same way off
which side their bounding corners are on.

Cells are addressed by raw canvas pixel position, not grid index, so a
frame can be docked anywhere.
*/
/*
Shared layer ids for the three-column mode layout that GameMode,
EditorMode, FragmentEditorMode, and GeneratorMode all use: the outer
window edges, the dividers that wall off the left/right panels from the
map, and the divider that walls off the map from the info box below it.
Each of those modes marks its whole frame up front under these layers,
then interleaves fb.draw(layer) calls with its own panel/map/info-box
content so the interface builds up in the same four visual stages every
time: window edges, side panels, map, info box.
*/
constexpr int FRAME_LAYER_WINDOW = 0; // outer canvas edges
constexpr int FRAME_LAYER_PANELS = 1; // left/right panel dividers
constexpr int FRAME_LAYER_MAP    = 2; // map / info-box divider

class FrameBuilder {
public:
    /*
    Marks every CELL_SIZE-aligned cell in the horizontal run
    [pxFrom, pxTo) at height py. pxFrom/pxTo need not be CELL_SIZE-aligned
    themselves — cells are stepped from pxFrom, so a non-aligned run just
    loses its last partial cell rather than throwing off later runs.

    `layer` tags the cell for the draw(int) overload below — it plays no
    part in corner/edge classification, which always considers every
    marked cell regardless of layer.
    */
    void markRow(int py, int pxFrom, int pxTo, int layer = 0);

    // Same as markRow, but a vertical run at a fixed px across [pyFrom, pyTo).
    void markCol(int px, int pyFrom, int pyTo, int layer = 0);

    /*
    Computes every marked cell's tile per the rule above and draws it.
    Passing a layer draws only cells marked with that layer — letting a
    mode interleave its own content between two draw() calls (e.g. side
    panels before the map, the map before its info box) while still
    classifying corners and T-junctions against the whole frame, not just
    the cells belonging to that one layer. The default (-1) draws every
    marked cell in one pass, layer or no layer.
    */
    void draw(int layer = -1) const;

    /*
    Packs a cell's raw canvas pixel position into cells_'s key space.
    Public so renderer.cpp's free classification/walk functions (which
    need to look cells up by neighbour position, not just iterate them)
    can use the exact same packing draw() and markRow()/markCol() do,
    without duplicating it.
    */
    static long long key(int px, int py);

private:
    std::unordered_map<long long, int> cells_;
};
