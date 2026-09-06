#include "palette.h"

#include <SDL2/SDL_image.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "json_min.h"

namespace {

constexpr const char* kPaletteDir     = "assets/palettes/";
constexpr const char* kRegistryPath   = "assets/palettes/palettes.json";

// The palette whose own PNG colors define the domain every palette (this
// one included) maps FROM — see palette.h's file comment. Hardcoded
// rather than JSON-flagged since exactly one palette must play this role
// for the lookup table to mean anything.
constexpr const char* kReferenceFile = "grayscale.png";

std::vector<PaletteInfo> g_palettes;
std::string              g_activeFile;

// Reference palette's colors and the active palette's colors, both in
// file pixel order — g_domainColors[i] is the gray shade tile art uses
// for ramp index i, g_activeColors[i] is what it should become.
std::vector<Uint32> g_domainColors;
std::vector<Uint32> g_activeColors;

[[noreturn]] void fail(const std::string& message) {
    fprintf(stderr, "palettes: %s\n", message.c_str());
    exit(1);
}

Uint32 packRGB(Uint8 r, Uint8 g, Uint8 b) {
    return (static_cast<Uint32>(r) << 16) | (static_cast<Uint32>(g) << 8) | b;
}

// Reads a palette PNG's pixels left to right into a flat RGB list, one
// entry per ramp index — alpha is ignored, since palette swatches are
// fully opaque by convention.
std::vector<Uint32> loadPaletteColors(const std::string& file) {
    std::string path = std::string(kPaletteDir) + file;
    SDL_Surface* raw = IMG_Load(path.c_str());
    if (!raw) fail("could not open \"" + path + "\" (" + IMG_GetError() + ")");

    SDL_Surface* surf = SDL_ConvertSurfaceFormat(raw, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(raw);
    if (!surf) fail("could not convert \"" + path + "\" to RGBA32");

    std::vector<Uint32> colors;
    colors.reserve((size_t)surf->w);
    const Uint8* pixels = static_cast<const Uint8*>(surf->pixels);
    for (int x = 0; x < surf->w; ++x) {
        const Uint8* p = pixels + x * 4;
        colors.push_back(packRGB(p[0], p[1], p[2]));
    }
    SDL_FreeSurface(surf);
    return colors;
}

// Parses one {"file": "...", "name": "..."} object — same manual
// key-dispatch style as core/tiles.cpp's parseEntry.
const char* parseEntry(const char* p, PaletteInfo& out) {
    p = jChar(p, '{');
    while (p && *p && *p != '}') {
        char key[32] = {};
        p = jString(p, key, sizeof(key));
        p = jChar(p, ':');

        char strBuf[256] = {};
        if      (strcmp(key, "file") == 0) { p = jString(p, strBuf, sizeof(strBuf)); out.file = strBuf; }
        else if (strcmp(key, "name") == 0) { p = jString(p, strBuf, sizeof(strBuf)); out.name = strBuf; }
        else                                 { p = jSkipValue(p); } // forward-compatible: ignore unknown keys

        p = jSkip(p);
        if (p && *p == ',') ++p;
        p = jSkip(p);
    }
    p = jChar(p, '}');
    return p;
}

void rebuildActiveColors() {
    for (const PaletteInfo& pi : g_palettes) {
        if (pi.file != g_activeFile) continue;
        g_activeColors = loadPaletteColors(pi.file);
        return;
    }
}

} // namespace

void loadPaletteRegistry() {
    g_palettes.clear();

    FILE* f = fopen(kRegistryPath, "rb");
    if (!f) fail(std::string("could not open \"") + kRegistryPath + "\"");
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);
    std::vector<char> buf((size_t)len + 1);
    fread(buf.data(), 1, (size_t)len, f);
    fclose(f);
    buf[(size_t)len] = '\0';

    const char* p = jChar(buf.data(), '[');
    p = jSkip(p);
    while (p && *p && *p != ']') {
        PaletteInfo info;
        p = parseEntry(p, info);

        if (info.file.empty()) fail("a palette entry has no \"file\"");
        if (info.name.empty()) fail("palette \"" + info.file + "\" has no \"name\"");
        g_palettes.push_back(info);

        p = jSkip(p);
        if (p && *p == ',') ++p;
        p = jSkip(p);
    }

    if (g_palettes.empty()) fail(std::string(kRegistryPath) + " defines no palettes");

    g_domainColors = loadPaletteColors(kReferenceFile);

    g_activeFile = g_palettes.front().file;
    rebuildActiveColors();
}

const std::vector<PaletteInfo>& getPalettes() { return g_palettes; }
const std::string& getActivePaletteFile() { return g_activeFile; }

bool setActivePalette(const std::string& file) {
    for (const PaletteInfo& pi : g_palettes) {
        if (pi.file != file) continue;
        g_activeFile = file;
        rebuildActiveColors();
        return true;
    }
    return false;
}

void applyActivePalette(SDL_Surface* surf) {
    if (!surf || surf->format->format != SDL_PIXELFORMAT_RGBA32) return;
    if (g_activeColors.size() != g_domainColors.size() || g_domainColors.empty()) return;

    Uint8* pixels = static_cast<Uint8*>(surf->pixels);
    for (int y = 0; y < surf->h; ++y) {
        Uint8* row = pixels + y * surf->pitch;
        for (int x = 0; x < surf->w; ++x) {
            Uint8* px = row + x * 4;
            if (px[3] == 0) continue; // fully transparent — nothing to recolor

            Uint32 color = packRGB(px[0], px[1], px[2]);
            for (size_t i = 0; i < g_domainColors.size(); ++i) {
                if (g_domainColors[i] != color) continue;
                Uint32 target = g_activeColors[i];
                px[0] = (Uint8)(target >> 16);
                px[1] = (Uint8)(target >> 8);
                px[2] = (Uint8)target;
                break;
            }
        }
    }
}
