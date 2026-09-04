#include "level.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <system_error>
#include <vector>

#include "json_min.h"

/*
Map authoring notes
Locations are stored one-per-file in WORLD_DIR/LEVEL_<floor>_<x>_<y>.json
and loaded at startup by initLevels(). In the Editor: F5 saves the map
currently being edited to the selected coordinate, F6 loads the selected
coordinate's file back into the editor. See editor/editor_mode.cpp.
*/

std::unordered_map<LevelCoord, Level, LevelCoordHash> worldLevels;

// Formats one world coordinate as a sign-aware, always-2-digit-magnitude
// field ("-01", "00", "03") per assets/world/WORLD.md — plain "%02d"
// doesn't zero-pad the magnitude once a '-' is involved.
static void appendCoord(std::string& out, int v) {
    char buf[16];
    if (v < 0) snprintf(buf, sizeof(buf), "-%02d", -v);
    else       snprintf(buf, sizeof(buf), "%02d", v);
    out += buf;
}

std::string levelFileName(int floor, int x, int y) {
    std::string name = WORLD_DIR;
    name += "/LEVEL_";
    appendCoord(name, floor);
    name += "_";
    appendCoord(name, x);
    name += "_";
    appendCoord(name, y);
    name += ".json";
    return name;
}

static void fillEmpty(Level& level) {
    for (int y = 0; y < MAX_HEIGHT; ++y)
        for (int x = 0; x < MAX_WIDTH; ++x) {
            level.tileMap[y][x]      = EMPTY_ID;
            level.collisionMap[y][x] = EMPTY_ID;
            level.entityMap[y][x]    = EMPTY_ID;
            level.objectMap[y][x]    = EMPTY_ID;
            level.occlusionMap[y][x] = EMPTY_ID;
        }
}

// loadLevelFromFile
bool loadLevelFromFile(Level& level, const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return false;

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);

    std::vector<char> buf(len + 1);
    fread(buf.data(), 1, (size_t)len, f);
    fclose(f);
    buf[len] = '\0';

    const char* p = jChar(buf.data(), '{');

    while (p && *p && *p != '}') {
        char key[64] = {};
        p = jString(p, key, sizeof(key));
        p = jChar(p, ':');

        if      (strcmp(key, "name")         == 0) { p = jString(p, level.name, 64); }
        else if (strcmp(key, "floor")        == 0) { long long v; p = jInt(p, &v); level.floor  = (int)v; }
        else if (strcmp(key, "x")            == 0) { long long v; p = jInt(p, &v); level.locX   = (int)v; }
        else if (strcmp(key, "y")            == 0) { long long v; p = jInt(p, &v); level.locY   = (int)v; }
        else if (strcmp(key, "width")        == 0) { long long v; p = jInt(p, &v); level.width  = (int)v; }
        else if (strcmp(key, "height")       == 0) { long long v; p = jInt(p, &v); level.height = (int)v; }
        else if (strcmp(key, "isStatic")     == 0) { p = jBool(p, &level.isStatic); }
        else if (strcmp(key, "tileMap")      == 0) { p = j2DArray(p, level.tileMap); }
        else if (strcmp(key, "collisionMap") == 0) { p = j2DArray(p, level.collisionMap); }
        else if (strcmp(key, "entityMap")    == 0) { p = j2DArray(p, level.entityMap); }
        else if (strcmp(key, "objectMap")    == 0) { p = j2DArray(p, level.objectMap); }
        else if (strcmp(key, "occlusionMap") == 0) { p = j2DArray(p, level.occlusionMap); }
        else                                        { p = jSkipValue(p); } // ignore unknown keys

        p = jSkip(p);
        if (p && *p == ',') ++p; // skip key-value separator
        p = jSkip(p);
    }

    return true;
}

bool saveLevelToFile(const Level& level, const char* path) {
    std::error_code ec;
    std::filesystem::path p(path);
    if (p.has_parent_path()) std::filesystem::create_directories(p.parent_path(), ec);

    FILE* f = fopen(path, "wb");
    if (!f) return false;

    fprintf(f, "{\n");
    fprintf(f, "  \"name\": \"%s\",\n", jEscape(level.name).c_str());
    fprintf(f, "  \"floor\": %d,\n", level.floor);
    fprintf(f, "  \"x\": %d,\n", level.locX);
    fprintf(f, "  \"y\": %d,\n", level.locY);
    fprintf(f, "  \"width\": %d,\n", level.width);
    fprintf(f, "  \"height\": %d,\n", level.height);
    fprintf(f, "  \"isStatic\": %s,\n", level.isStatic ? "true" : "false");
    fprintf(f, "  \"tileMap\": ");      jWrite2DArray(f, level.tileMap);      fprintf(f, ",\n");
    fprintf(f, "  \"objectMap\": ");    jWrite2DArray(f, level.objectMap);    fprintf(f, ",\n");
    fprintf(f, "  \"entityMap\": ");    jWrite2DArray(f, level.entityMap);    fprintf(f, ",\n");
    fprintf(f, "  \"collisionMap\": "); jWrite2DArray(f, level.collisionMap); fprintf(f, ",\n");
    fprintf(f, "  \"occlusionMap\": "); jWrite2DArray(f, level.occlusionMap); fprintf(f, "\n");
    fprintf(f, "}\n");

    fclose(f);
    return true;
}

// World access
Level* findLevel(int floor, int x, int y) {
    auto it = worldLevels.find(LevelCoord{floor, x, y});
    return (it != worldLevels.end()) ? &it->second : nullptr;
}

Level& getOrCreateLevel(int floor, int x, int y) {
    LevelCoord key{floor, x, y};
    auto it = worldLevels.find(key);
    if (it != worldLevels.end()) return it->second;

    Level level;
    fillEmpty(level);
    level.width  = MAX_WIDTH;
    level.height = MAX_HEIGHT;
    level.isStatic = true;
    snprintf(level.name, sizeof(level.name), "New Location");
    level.floor = floor;
    level.locX  = x;
    level.locY  = y;

    return worldLevels.emplace(key, level).first->second;
}

std::vector<LevelCoord> listWorldLevels() {
    std::vector<LevelCoord> coords;
    coords.reserve(worldLevels.size());
    for (const auto& [key, level] : worldLevels) coords.push_back(key);

    std::sort(coords.begin(), coords.end(), [](const LevelCoord& a, const LevelCoord& b) {
        if (a.floor != b.floor) return a.floor < b.floor;
        if (a.x     != b.x)     return a.x     < b.x;
        return a.y < b.y;
    });
    return coords;
}

/*
initLevels
Called once at startup (before entering GameMode) and again whenever the
editor or /load wants a fresh view of what's on disk. Scans WORLD_DIR for
every LEVEL_<floor>_<x>_<y>.json file and loads it — nothing more. There
is no fallback placeholder: if WORLD_DIR is empty (or (0,0,0) specifically
isn't in it), worldLevels ends up empty (or missing that coordinate), and
callers are expected to handle "nothing loaded here" themselves rather
than silently getting synthesized content.
*/
void initLevels() {
    worldLevels.clear();

    namespace fs = std::filesystem;
    std::error_code ec;
    if (fs::exists(WORLD_DIR, ec) && fs::is_directory(WORLD_DIR, ec)) {
        for (const auto& entry : fs::directory_iterator(WORLD_DIR, ec)) {
            if (ec || !entry.is_regular_file()) continue;

            std::string fname = entry.path().filename().string();
            int floor = 0, x = 0, y = 0;
            if (sscanf(fname.c_str(), "LEVEL_%d_%d_%d.json", &floor, &x, &y) != 3)
                continue; // not a location file — ignore
            // sscanf's %d parses "-01" as -1 regardless of the leading
            // zero, so the write-side zero-padding above needs no
            // matching read-side logic here.

            Level level;
            fillEmpty(level);
            level.width  = MAX_WIDTH;
            level.height = MAX_HEIGHT;
            level.isStatic = true;
            snprintf(level.name, sizeof(level.name), "Location %d/%d/%d", floor, x, y);

            if (loadLevelFromFile(level, entry.path().string().c_str())) {
                /*
                The filename is authoritative for where this location
                lives in the world, regardless of what's in the JSON body
                (guards against a hand-edited/renamed file disagreeing
                with its own coordinate fields).
                */
                level.floor = floor;
                level.locX  = x;
                level.locY  = y;
                worldLevels[LevelCoord{floor, x, y}] = level;
            }
        }
    }
}
