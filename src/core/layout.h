#pragma once

#include "level.h"

/*
Fixed virtual canvas
The game ALWAYS builds its frame at exactly CANVAS_W x CANVAS_H logical
pixels — the game world, the camera, and every layout in game/editor/
settings mode work purely in these coordinates and never see the real
window size. core/display.h takes that one finished CANVAS_W x CANVAS_H
frame and scales/letterboxes it into whatever window is actually open.
See core/renderer.cpp's beginFrame()/endFrame() and core/app.cpp's run().

Because the canvas size is a compile-time constant rather than a
window-dependent value, every panel/divider/map position below is ALSO a
compile-time constant — there is no per-frame layout computation.
Game/editor/settings modes just reference these constants directly.
*/

constexpr int CELL_SIZE = 16;

constexpr int CANVAS_W = 1280; // fixed virtual resolution — never changes with window size
constexpr int CANVAS_H = 800;

constexpr int UI_BOX_ROWS = 13; // bottom info box, in cells

/*
One dedicated column of UI chrome, used TWICE in EditorMode/GameMode —
once between the left panel and the map, once between the map and the
right panel — so both panels are visually (and spatially) separated from
the map. Never part of either panel's own width and never part of the
map's own MAX_WIDTH cells; see FrameBuilder in core/renderer.h.

DIVIDER_H is the same idea rotated 90 degrees: one dedicated row above
the map (the window's own top wall) and one below it (the map/info-box
divider), each kept out of the map's own MAX_HEIGHT rows the same way
DIVIDER_W is kept out of MAX_WIDTH.
*/
constexpr int DIVIDER_W = CELL_SIZE;
constexpr int DIVIDER_H = CELL_SIZE;

constexpr int MAP_PIXEL_W = MAX_WIDTH  * CELL_SIZE;
constexpr int MAP_PIXEL_H = MAX_HEIGHT * CELL_SIZE;

/*
Vertical position of the map itself
MAP_ORIGIN_Y is the row right below the window's own top wall — the map
never draws into row 0, so that row is pure chrome no matter what a level
happens to store there. MAP_BOTTOM_Y is the map/info-box divider's own
row, directly below the map's last row rather than overlapping it.
INFO_BOX_ORIGIN_Y is the info box's first row, one past that divider.
*/
constexpr int MAP_ORIGIN_Y      = DIVIDER_H;
constexpr int MAP_BOTTOM_Y      = MAP_ORIGIN_Y + MAP_PIXEL_H;
constexpr int INFO_BOX_ORIGIN_Y = MAP_BOTTOM_Y + DIVIDER_H;

/*
The top divider, the map, the bottom divider, and the info box below it
must exactly fill the canvas height — this is a hard invariant of the
shared game/editor layout, checked at compile time rather than hoped for.
*/
static_assert(INFO_BOX_ORIGIN_Y + UI_BOX_ROWS * CELL_SIZE == CANVAS_H,
              "top divider + map + bottom divider + info box rows must exactly fill CANVAS_H");

/*
Both GameMode and EditorMode share the identical three-column layout:
a fixed-width panel, a divider, the fixed-width map, another divider,
then a second fixed-width panel — palette + world list in EditorMode,
reserved/empty in GameMode (see game layout.txt / editor layout.txt).
PANEL_W is whatever's left over after the map and both dividers, split
evenly between the two panels — a pure compile-time constant, since the
canvas itself never grows with the window.
*/
constexpr int PANEL_W = (CANVAS_W - MAP_PIXEL_W - DIVIDER_W * 2) / 2;

// Every side panel, divider, and the map itself must tile the canvas width
// with nothing left over and nothing overlapping.
static_assert(PANEL_W * 2 + DIVIDER_W * 2 + MAP_PIXEL_W == CANVAS_W,
              "left panel + divider + map + divider + right panel must exactly fill CANVAS_W");

/*
Shared column positions (pixels, canvas-space)
The one source of truth for where each region starts — used identically
by game_mode.cpp, editor_mode.cpp, and editor_controls.cpp (rendering AND
hit-testing), so they can never drift apart from one another.
*/
constexpr int LEFT_DIVIDER_X   = PANEL_W;                     // divider between left panel and map
constexpr int MAP_ORIGIN_X     = PANEL_W + DIVIDER_W;         // map's first column
constexpr int MAP_RIGHT_EDGE_X = MAP_ORIGIN_X + MAP_PIXEL_W;  // one past the map's last column
constexpr int RIGHT_PANEL_X    = MAP_RIGHT_EDGE_X + DIVIDER_W; // right panel's first column
