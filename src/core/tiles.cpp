#include "tiles.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <vector>

#include "json_min.h"

namespace {

/*
tiles.json entry, exactly as authored — see assets/tiles/tiles.json's own
field-by-field usage for what each mode actually needs. Defaults here
match the generator's: cellW/cellH/tier default to 1, col/row/mask
default to 0/-1, paletteVisible defaults true.
*/
struct RawEntry {
    TileID      id = EMPTY_ID;
    std::string file;
    LayerType   layer = LayerType::Ground;
    TileMode    mode  = TileMode::Manual;
    int         cellW = 1, cellH = 1;
    int         col = 0, row = 0;
    std::string group;
    std::string object;
    int         tier = 1;
    std::string role;
    std::string blobKind;
    int         mask = -1;
    bool        paletteVisible = true;
};

std::vector<RawEntry> g_entries;

// TileID -> resolved render/footprint metadata, for every real entry AND
// every synthesized sub-cell (see synthesizeSubTiles below).
std::unordered_map<TileID, TileMetadata> g_meta;

// TileID -> which group/mode it belongs to (real entries only — a
// synthesized sub-cell is never itself group-addressable, only its
// anchor is, which is exactly what placeTile/eraseTile already hold).
std::unordered_map<TileID, std::string> g_groupOf;
std::unordered_map<TileID, TileMode>    g_modeOf;

// group -> members, in file order — random-fill resolution.
std::unordered_map<std::string, std::vector<TileID>> g_randomGroups;

// group -> role -> id — autotile_blend resolution.
std::unordered_map<std::string, std::unordered_map<std::string, TileID>> g_blendGroups;

// group -> mask -> id — autotile_blob resolution.
std::unordered_map<std::string, std::unordered_map<int, TileID>> g_blobGroups;

// "object|tier" -> ordered frames — interactive resolution.
std::unordered_map<std::string, std::vector<TileID>> g_interactiveFrames;

// TileID (real OR synthesized sub-id) -> (object, tier, frame index) —
// covers sub-ids too, since isDoorBlocking() may be asked about a door's
// bottom-half sub-cell just as often as its anchor.
struct InteractiveInfo { std::string object; int tier; int frameIndex; };
std::unordered_map<TileID, InteractiveInfo> g_interactiveInfo;

// Sub-cell synthesis (see tiles.h's subTileId doc comment). Starts well
// above the highest possible makeTileId() value (0x5A5A) and stays well
// below the special-sprite constants (0xFFF5+), leaving ~24k ids of
// headroom — far more than any realistic sheet of multi-cell sprites
// will ever need.
constexpr TileID kSubIdRangeStart = 0x6000;
constexpr TileID kSubIdRangeEnd   = 0xFFF4;
TileID g_nextSubId = kSubIdRangeStart;

inline uint32_t subKey(TileID anchor, int dx, int dy) {
    return ((uint32_t)anchor << 16) | ((uint32_t)(uint8_t)dy << 8) | (uint32_t)(uint8_t)dx;
}
std::unordered_map<uint32_t, TileID> g_subIds;

// Reverse of g_subIds — synthesized sub-id -> (anchor, dx, dy). See
// tiles.h's anchorOf.
struct AnchorInfo { TileID anchor; int dx, dy; };
std::unordered_map<TileID, AnchorInfo> g_anchorOf;

std::vector<TileID> g_paletteTiles;
std::vector<TileID> g_autotilePaletteTiles;

[[noreturn]] void fail(const std::string& message) {
    fprintf(stderr, "tiles.json: %s\n", message.c_str());
    exit(1);
}

LayerType parseLayer(const std::string& s) {
    if (s == "ground")   return LayerType::Ground;
    if (s == "objects")  return LayerType::Objects;
    if (s == "entities") return LayerType::Entities;
    fail("unknown layer \"" + s + "\"");
}

TileMode parseMode(const std::string& s) {
    if (s == "manual")          return TileMode::Manual;
    if (s == "random")          return TileMode::Random;
    if (s == "interactive")     return TileMode::Interactive;
    if (s == "autotile_blend")  return TileMode::AutotileBlend;
    if (s == "autotile_blob")   return TileMode::AutotileBlob;
    fail("unknown mode \"" + s + "\"");
}

// Every character of a tiles.json id must be one of these 36 symbols —
// see tiles.h's makeTileId doc comment for why that keeps the packed
// TileID space collision-free by construction.
bool validIdChar(char c) { return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z'); }

TileID parseId(const std::string& s) {
    if (s.size() != 2 || !validIdChar(s[0]) || !validIdChar(s[1]))
        fail("id \"" + s + "\" must be exactly 2 characters from [0-9A-Z]");
    return makeTileId(s[0], s[1]);
}

// Parses one {...} object (the cursor sits just past its opening brace's
// containing array bracket — see loadTileRegistry) into a RawEntry, using
// the same key-dispatch loop as core/level.cpp's loadLevelFromFile.
const char* parseEntry(const char* p, RawEntry& e) {
    std::string idStr, layerStr, modeStr;
    p = jChar(p, '{');

    while (p && *p && *p != '}') {
        char key[32] = {};
        p = jString(p, key, sizeof(key));
        p = jChar(p, ':');

        char strBuf[256] = {};
        long long v = 0;

        if      (strcmp(key, "id")             == 0) { p = jString(p, strBuf, sizeof(strBuf)); idStr = strBuf; }
        else if (strcmp(key, "file")            == 0) { p = jString(p, strBuf, sizeof(strBuf)); e.file = strBuf; }
        else if (strcmp(key, "layer")           == 0) { p = jString(p, strBuf, sizeof(strBuf)); layerStr = strBuf; }
        else if (strcmp(key, "mode")            == 0) { p = jString(p, strBuf, sizeof(strBuf)); modeStr = strBuf; }
        else if (strcmp(key, "cellW")           == 0) { p = jInt(p, &v); e.cellW = (int)v; }
        else if (strcmp(key, "cellH")           == 0) { p = jInt(p, &v); e.cellH = (int)v; }
        else if (strcmp(key, "col")             == 0) { p = jInt(p, &v); e.col = (int)v; }
        else if (strcmp(key, "row")             == 0) { p = jInt(p, &v); e.row = (int)v; }
        else if (strcmp(key, "group")           == 0) { p = jString(p, strBuf, sizeof(strBuf)); e.group = strBuf; }
        else if (strcmp(key, "object")          == 0) { p = jString(p, strBuf, sizeof(strBuf)); e.object = strBuf; }
        else if (strcmp(key, "tier")            == 0) { p = jInt(p, &v); e.tier = (int)v; }
        else if (strcmp(key, "role")            == 0) { p = jString(p, strBuf, sizeof(strBuf)); e.role = strBuf; }
        else if (strcmp(key, "blobKind")        == 0) { p = jString(p, strBuf, sizeof(strBuf)); e.blobKind = strBuf; }
        else if (strcmp(key, "mask")            == 0) { p = jInt(p, &v); e.mask = (int)v; }
        else if (strcmp(key, "paletteVisible")  == 0) { p = jBool(p, &e.paletteVisible); }
        else                                            { p = jSkipValue(p); } // forward-compatible: ignore unknown keys

        p = jSkip(p);
        if (p && *p == ',') ++p;
        p = jSkip(p);
    }
    p = jChar(p, '}');

    if (idStr.empty())    fail("entry with file \"" + e.file + "\" has no \"id\"");
    if (e.file.empty())   fail("entry \"" + idStr + "\" has no \"file\"");
    if (layerStr.empty()) fail("entry \"" + idStr + "\" has no \"layer\"");
    if (modeStr.empty())  fail("entry \"" + idStr + "\" has no \"mode\"");

    e.id    = parseId(idStr);
    e.layer = parseLayer(layerStr);
    e.mode  = parseMode(modeStr);
    return p;
}

TileMetadata metaFor(const RawEntry& e) {
    TileMetadata m;
    m.layer          = e.layer;
    m.w              = (uint8_t)e.cellW;
    m.h              = (uint8_t)e.cellH;
    m.file           = e.file;
    m.srcCellX       = e.col * e.cellW;
    m.srcCellY       = e.row * e.cellH;
    m.paletteVisible = e.paletteVisible;
    return m;
}

/*
Allocates a fresh sub-id for every non-anchor (dx, dy) offset of every
multi-cell entry, so every render loop can stay a dumb per-cell "look up
this exact TileID" pass — see tiles.h's subTileId doc comment for why
this exists at all instead of just having getTileMeta report a footprint
for renderer.cpp to crop into.
*/
void synthesizeSubTiles(const RawEntry& e) {
    if (e.cellW <= 1 && e.cellH <= 1) return;

    for (int dy = 0; dy < e.cellH; ++dy) {
        for (int dx = 0; dx < e.cellW; ++dx) {
            if (dx == 0 && dy == 0) continue; // the anchor cell IS e.id — nothing to synthesize

            if (g_nextSubId >= kSubIdRangeEnd)
                fail("ran out of synthesized sub-tile ids (too many multi-cell entries)");
            TileID subId = g_nextSubId++;

            TileMetadata m;
            m.layer          = e.layer;
            m.w = m.h        = 1;
            m.file           = e.file;
            m.srcCellX       = e.col * e.cellW + dx;
            m.srcCellY       = e.row * e.cellH + dy;
            m.paletteVisible = false;
            g_meta[subId] = m;
            g_subIds[subKey(e.id, dx, dy)] = subId;
            g_anchorOf[subId] = { e.id, dx, dy };

            // Propagate interactive identity so a door's bottom half
            // still answers tileObjectName()/isDoorBlocking() correctly.
            auto it = g_interactiveInfo.find(e.id);
            if (it != g_interactiveInfo.end())
                g_interactiveInfo[subId] = it->second;
        }
    }
}

} // namespace

void loadTileRegistry() {
    g_entries.clear();
    g_meta.clear();
    g_groupOf.clear();
    g_modeOf.clear();
    g_randomGroups.clear();
    g_blendGroups.clear();
    g_blobGroups.clear();
    g_interactiveFrames.clear();
    g_interactiveInfo.clear();
    g_subIds.clear();
    g_anchorOf.clear();
    g_paletteTiles.clear();
    g_autotilePaletteTiles.clear();
    g_nextSubId = kSubIdRangeStart;

    const char* path = "assets/tiles/tiles.json";
    FILE* f = fopen(path, "rb");
    if (!f) fail(std::string("could not open \"") + path + "\"");

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);
    std::vector<char> buf(len + 1);
    fread(buf.data(), 1, (size_t)len, f);
    fclose(f);
    buf[len] = '\0';

    const char* p = jChar(buf.data(), '[');
    p = jSkip(p);
    while (p && *p && *p != ']') {
        RawEntry e;
        p = parseEntry(p, e);

        if (g_meta.count(e.id))
            fail("duplicate id \"" + std::string(1, (char)(e.id >> 8)) + std::string(1, (char)(e.id & 0xFF)) + "\"");

        g_entries.push_back(e);
        g_meta[e.id]     = metaFor(e);
        g_groupOf[e.id]  = e.group;
        g_modeOf[e.id]   = e.mode;

        if (e.mode == TileMode::Random && !e.group.empty())
            g_randomGroups[e.group].push_back(e.id);

        if (e.mode == TileMode::AutotileBlend && !e.group.empty() && !e.role.empty())
            g_blendGroups[e.group][e.role] = e.id;

        if (e.mode == TileMode::AutotileBlob && !e.group.empty() && e.mask >= 0)
            g_blobGroups[e.group][e.mask] = e.id;

        if (e.mode == TileMode::Interactive && !e.object.empty()) {
            std::string key = e.object + "|" + std::to_string(e.tier);
            int frameIndex = (int)g_interactiveFrames[key].size();
            g_interactiveFrames[key].push_back(e.id);
            g_interactiveInfo[e.id] = { e.object, e.tier, frameIndex };
        }

        if (e.paletteVisible) {
            if (e.mode == TileMode::Manual || e.mode == TileMode::Random || e.mode == TileMode::Interactive)
                g_paletteTiles.push_back(e.id);
            else if (e.mode == TileMode::AutotileBlend || e.mode == TileMode::AutotileBlob)
                g_autotilePaletteTiles.push_back(e.id);
        }

        p = jSkip(p);
        if (p && *p == ',') ++p;
        p = jSkip(p);
    }

    // Sub-tile synthesis happens in a second pass, after every real entry
    // (and every interactive entry's g_interactiveInfo) is already known.
    for (const RawEntry& e : g_entries)
        synthesizeSubTiles(e);
}

TileMetadata getTileMeta(TileID id) {
    auto it = g_meta.find(id);
    if (it != g_meta.end()) return it->second;
    return TileMetadata{}; // EMPTY_ID, a sentinel, or an unknown id — safe, invisible default
}

std::vector<TileID> getPaletteTiles() { return g_paletteTiles; }
std::vector<TileID> getAutotilePaletteTiles() { return g_autotilePaletteTiles; }

TileID subTileId(TileID anchorId, int dx, int dy) {
    if (dx == 0 && dy == 0) return anchorId;
    auto it = g_subIds.find(subKey(anchorId, dx, dy));
    return (it != g_subIds.end()) ? it->second : EMPTY_ID;
}

TileID anchorOf(TileID id, int& dx, int& dy) {
    auto it = g_anchorOf.find(id);
    if (it == g_anchorOf.end()) { dx = 0; dy = 0; return id; }
    dx = it->second.dx;
    dy = it->second.dy;
    return it->second.anchor;
}

TileID pickRandomVariant(TileID id) {
    auto groupIt = g_groupOf.find(id);
    if (groupIt == g_groupOf.end() || groupIt->second.empty()) return id;

    auto membersIt = g_randomGroups.find(groupIt->second);
    if (membersIt == g_randomGroups.end() || membersIt->second.empty()) return id;

    const std::vector<TileID>& members = membersIt->second;
    return members[(size_t)rand() % members.size()];
}

bool isAutotileTile(TileID id) {
    auto it = g_modeOf.find(id);
    return it != g_modeOf.end() && (it->second == TileMode::AutotileBlend || it->second == TileMode::AutotileBlob);
}

bool isAutotileBlend(TileID id) {
    auto it = g_modeOf.find(id);
    return it != g_modeOf.end() && it->second == TileMode::AutotileBlend;
}

bool isAutotileBlob(TileID id) {
    auto it = g_modeOf.find(id);
    return it != g_modeOf.end() && it->second == TileMode::AutotileBlob;
}

bool sameTileGroup(TileID a, TileID b) {
    auto ga = g_groupOf.find(a);
    auto gb = g_groupOf.find(b);
    if (ga == g_groupOf.end() || gb == g_groupOf.end()) return false;
    if (ga->second.empty() || gb->second.empty()) return false;
    return ga->second == gb->second;
}

/*
computeBlendRole
Picks which of the 9 base pieces or 4 concave-corner pieces represents
this neighbourhood, for a material laid out as a rounded blob region
(see assets/tiles/tiles.json's "autotile_blend" entries and the visual
catalogue this was designed from). Concave corners take priority over the
base 3x3 grid since they're the more specific case (an L-shaped bend in
an otherwise fully-surrounded cell); only one corner is ever reported per
call even if a cell technically qualifies for more than one, since a
single flat tile can't composite two overlays at once — this is a
best-effort convention to verify visually and adjust in tiles.json
(the (col,row) of any role), not a confirmed pixel-perfect spec.
*/
std::string computeBlendRole(bool n, bool s, bool e, bool w,
                              bool ne, bool nw, bool se, bool sw) {
    if (n && e && !ne) return "corner_ne";
    if (n && w && !nw) return "corner_nw";
    if (s && e && !se) return "corner_se";
    if (s && w && !sw) return "corner_sw";

    if (!n && !w && s && e) return "nw";
    if (!n && !e && s && w) return "ne";
    if (!s && !w && n && e) return "sw";
    if (!s && !e && n && w) return "se";

    if (!n && s && e && w) return "n";
    if (!s && n && e && w) return "s";
    if (!w && n && s && e) return "w";
    if (!e && n && s && w) return "e";

    return "c";
}

TileID resolveAutotileBlend(TileID id, bool n, bool s, bool e, bool w,
                             bool ne, bool nw, bool se, bool sw) {
    auto groupIt = g_groupOf.find(id);
    if (groupIt == g_groupOf.end() || groupIt->second.empty()) return id;

    auto blendIt = g_blendGroups.find(groupIt->second);
    if (blendIt == g_blendGroups.end()) return id;

    std::string role = computeBlendRole(n, s, e, w, ne, nw, se, sw);
    auto roleIt = blendIt->second.find(role);
    return (roleIt != blendIt->second.end()) ? roleIt->second : id;
}

TileID resolveAutotileBlob(TileID id, bool n, bool s, bool e, bool w) {
    auto groupIt = g_groupOf.find(id);
    if (groupIt == g_groupOf.end() || groupIt->second.empty()) return id;

    auto blobIt = g_blobGroups.find(groupIt->second);
    if (blobIt == g_blobGroups.end()) return id;

    // Classic 4-bit blob bitmask: bit0=N, bit1=E, bit2=S, bit3=W — see
    // tiles.json's generator notes on autotile_blob for the (col,row)
    // convention this indexes into.
    int mask = (n ? 1 : 0) | (e ? 2 : 0) | (s ? 4 : 0) | (w ? 8 : 0);

    auto maskIt = blobIt->second.find(mask);
    if (maskIt != blobIt->second.end()) return maskIt->second;

    // Sheet too small to cover every mask (e.g. rails' 10 cells can't
    // hold all 16) — fall back to mask 0 rather than leaving a hole.
    auto zeroIt = blobIt->second.find(0);
    return (zeroIt != blobIt->second.end()) ? zeroIt->second : id;
}

std::vector<TileID> interactiveFrames(const std::string& object, int tier) {
    auto it = g_interactiveFrames.find(object + "|" + std::to_string(tier));
    return (it != g_interactiveFrames.end()) ? it->second : std::vector<TileID>{};
}

std::string tileObjectName(TileID id) {
    auto it = g_interactiveInfo.find(id);
    return (it != g_interactiveInfo.end()) ? it->second.object : std::string();
}

int tileObjectTier(TileID id) {
    auto it = g_interactiveInfo.find(id);
    return (it != g_interactiveInfo.end()) ? it->second.tier : 1;
}

int tileObjectFrameIndex(TileID id) {
    auto it = g_interactiveInfo.find(id);
    return (it != g_interactiveInfo.end()) ? it->second.frameIndex : 0;
}
