#pragma once

#include <string>
#include <vector>

/*
Panel themes
assets/panels/ holds one 96x96 PNG per theme — a 6x6 grid of CELL_SIZE
cells: a corner in each of the sheet's own 4 corners (pre-drawn facing
inward, so no runtime flip/rotate is needed), the 2 cells next to each
corner along every side, and the 2 tileable cells between those. This
replaces the old hardcoded border_horizontal.png / border_vertical.png /
border_corner.png FrameBuilder used to draw every mode's frame with — see
core/renderer.h's "Frame system" comment and renderer.cpp's
drawPanelCell()/FrameBuilder::draw() for exactly how a sheet's cells get
picked for a given frame.

assets/panels/panels.json is the registry, same shape as
assets/palettes/palettes.json:
  [ { "file": "ziggurat.png", "name": "Ziggurat" }, ... ]
loadPanelRegistry() parses it at startup. The settings screen's Panels
category (settings/settings_mode.cpp) lists whatever getPanels() returns
and calls setActivePanel() on selection.

This module only tracks the registry and the current selection — loading
and slicing the actual texture is core/renderer.cpp's job (getPanelTexture(),
private to that file), same split as core/palette.h vs. renderer.cpp's
getTileTexture().
*/

struct PanelInfo {
    std::string file; // e.g. "ziggurat.png" — stable id, used as the selection key
    std::string name; // display name shown in the settings menu, e.g. "Ziggurat"
};

// Parses assets/panels/panels.json into the registry and activates its
// first entry. Exits with an error, the same way loadTileRegistry() does,
// if the registry can't be read.
void loadPanelRegistry();

// Every known panel theme, in panels.json's own order — the order the
// settings screen lists them in.
const std::vector<PanelInfo>& getPanels();

// file of whichever panel theme is currently active.
const std::string& getActivePanelFile();

// Switches the active panel theme by its file id. Returns false (leaving
// the previous theme active) if file doesn't match any loaded theme.
bool setActivePanel(const std::string& file);
