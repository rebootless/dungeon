#pragma once

#include <string>
#include <vector>

#include <SDL2/SDL.h>

/*
Color palettes
assets/tiles/*.png are authored in a small fixed set of gray shades, each
shade doubling as an index into a 24-entry ramp table. assets/palettes/
holds one PNG per palette, every one of them a 1-pixel-tall strip whose
Nth pixel is that palette's color for ramp index N — including
grayscale.png, whose own pixel colors ARE the exact gray shades the tile
art is drawn in, which is what makes it double as this system's domain:
recoloring a tile texture for any other palette is just "look up this
pixel's gray shade in grayscale.png, take the color at the same index
from the active palette instead", no shaders or per-tile art required.

assets/palettes/palettes.json is the single source of truth for which
palettes exist — same shape as assets/tiles/tiles.json, one flat array:
  [ { "file": "rusty.png", "name": "Rusty" }, ... ]
loadPaletteRegistry() parses it at startup. The settings screen's
Palettes category (settings/settings_mode.cpp) lists whatever
getPalettes() returns and calls setActivePalette() on selection;
core/renderer.cpp's getTileTexture() runs every freshly loaded tile
surface through applyActivePalette() before it ever becomes a texture.
*/

struct PaletteInfo {
    std::string file; // e.g. "rusty.png" — stable id, used as the selection key
    std::string name; // display name shown in the settings menu, e.g. "Rusty"
};

// Parses assets/palettes/palettes.json into the registry and activates
// its first entry. Exits with an error, the same way loadTileRegistry()
// does, if the registry or the reference palette (whose own colors
// define the domain every palette maps FROM — see this file's own
// comment above) can't be read.
void loadPaletteRegistry();

// Every known palette, in palettes.json's own order — the order the
// settings screen lists them in.
const std::vector<PaletteInfo>& getPalettes();

// file of whichever palette is currently active.
const std::string& getActivePaletteFile();

// Switches the active palette by its file id and rebuilds the lookup
// table applyActivePalette() uses. Returns false (leaving the previous
// palette active) if file doesn't match any loaded palette.
bool setActivePalette(const std::string& file);

// Remaps every opaque pixel of `surf` in place from the reference
// palette's gray shades to the currently active palette's colors. `surf`
// must already be SDL_PIXELFORMAT_RGBA32 — a no-op otherwise. Alpha is
// left untouched; a pixel whose color isn't one of the reference shades
// (which shouldn't happen for a properly authored tile asset) is left as-is.
void applyActivePalette(SDL_Surface* surf);
