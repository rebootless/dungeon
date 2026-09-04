#include "fragment.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <system_error>

#include "../core/json_min.h"

/*
Map authoring notes
Fragments are stored one-per-file in FRAGMENTS_DIR/FRAGMENT_<id>.json and
loaded at startup by initFragments(). In the fragment editor: F5 saves
the buffer currently being edited to the selected id, F9 loads the
selected id's file back into the editor. See
generator/fragment_editor_mode.cpp.
*/

std::unordered_map<int, Fragment> fragments;

std::string fragmentFileName(int id) {
    char buf[96];
    // id is never negative (FRAGMENT_ID_MIN is 0), so plain %02d already
    // zero-pads correctly with no sign to account for.
    snprintf(buf, sizeof(buf), "%s/FRAGMENT_%02d.json", FRAGMENTS_DIR, id);
    return std::string(buf);
}

static void fillEmpty(Fragment& fragment) {
    for (int y = 0; y < MAX_HEIGHT; ++y)
        for (int x = 0; x < MAX_WIDTH; ++x) {
            fragment.tileMap[y][x]      = EMPTY_ID;
            fragment.objectMap[y][x]    = EMPTY_ID;
            fragment.entityMap[y][x]    = EMPTY_ID;
            fragment.collisionMap[y][x] = EMPTY_ID;
            fragment.occlusionMap[y][x] = EMPTY_ID;
            fragment.connectorMap[y][x] = EMPTY_ID;
        }
}

bool loadFragmentFromFile(Fragment& fragment, const char* path) {
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

        if      (strcmp(key, "name")         == 0) { p = jString(p, fragment.name, 64); }
        else if (strcmp(key, "width")        == 0) { long long v; p = jInt(p, &v); fragment.width  = (int)v; }
        else if (strcmp(key, "height")       == 0) { long long v; p = jInt(p, &v); fragment.height = (int)v; }
        else if (strcmp(key, "tileMap")      == 0) { p = j2DArray(p, fragment.tileMap); }
        else if (strcmp(key, "objectMap")    == 0) { p = j2DArray(p, fragment.objectMap); }
        else if (strcmp(key, "entityMap")    == 0) { p = j2DArray(p, fragment.entityMap); }
        else if (strcmp(key, "collisionMap") == 0) { p = j2DArray(p, fragment.collisionMap); }
        else if (strcmp(key, "occlusionMap") == 0) { p = j2DArray(p, fragment.occlusionMap); }
        else if (strcmp(key, "connectorMap") == 0) { p = j2DArray(p, fragment.connectorMap); }
        else                                        { p = jSkipValue(p); } // ignore unknown keys

        p = jSkip(p);
        if (p && *p == ',') ++p; // skip key-value separator
        p = jSkip(p);
    }

    return true;
}

bool saveFragmentToFile(const Fragment& fragment, const char* path) {
    std::error_code ec;
    std::filesystem::path p(path);
    if (p.has_parent_path()) std::filesystem::create_directories(p.parent_path(), ec);

    FILE* f = fopen(path, "wb");
    if (!f) return false;

    fprintf(f, "{\n");
    fprintf(f, "  \"name\": \"%s\",\n", jEscape(fragment.name).c_str());
    fprintf(f, "  \"width\": %d,\n", fragment.width);
    fprintf(f, "  \"height\": %d,\n", fragment.height);
    fprintf(f, "  \"tileMap\": ");      jWrite2DArray(f, fragment.tileMap);      fprintf(f, ",\n");
    fprintf(f, "  \"objectMap\": ");    jWrite2DArray(f, fragment.objectMap);    fprintf(f, ",\n");
    fprintf(f, "  \"entityMap\": ");    jWrite2DArray(f, fragment.entityMap);    fprintf(f, ",\n");
    fprintf(f, "  \"collisionMap\": "); jWrite2DArray(f, fragment.collisionMap); fprintf(f, ",\n");
    fprintf(f, "  \"occlusionMap\": "); jWrite2DArray(f, fragment.occlusionMap); fprintf(f, ",\n");
    fprintf(f, "  \"connectorMap\": "); jWrite2DArray(f, fragment.connectorMap); fprintf(f, "\n");
    fprintf(f, "}\n");

    fclose(f);
    return true;
}

// Registry access
Fragment* findFragment(int id) {
    auto it = fragments.find(id);
    return (it != fragments.end()) ? &it->second : nullptr;
}

Fragment& getOrCreateFragment(int id) {
    auto it = fragments.find(id);
    if (it != fragments.end()) return it->second;

    Fragment fragment;
    fillEmpty(fragment);
    fragment.width  = 1;
    fragment.height = 1;
    snprintf(fragment.name, sizeof(fragment.name), "New Fragment");

    return fragments.emplace(id, fragment).first->second;
}

std::vector<int> listFragmentIds() {
    std::vector<int> ids;
    ids.reserve(fragments.size());
    for (const auto& [id, fragment] : fragments) ids.push_back(id);

    std::sort(ids.begin(), ids.end());
    return ids;
}

/*
initFragments
Called once at startup (alongside initLevels()) and again whenever the
fragment editor wants a fresh view of what's on disk. Scans fragments/
for every FRAGMENT-<id>.json file and loads it — nothing more. No
fallback placeholder: if fragments/ is empty, `fragments` ends up empty
too, and callers are expected to handle "nothing loaded yet" themselves.
*/
bool connectorSideAt(int x, int y, int width, int height, ConnectorSide& side) {
    bool onLeft   = (x == 0);
    bool onRight  = (x == width - 1);
    bool onTop    = (y == 0);
    bool onBottom = (y == height - 1);

    int edgeCount = (int)onLeft + (int)onRight + (int)onTop + (int)onBottom;
    if (edgeCount != 1) return false; // corner, interior, or degenerate axis — ambiguous

    if      (onLeft)   side = ConnectorSide::Left;
    else if (onRight)  side = ConnectorSide::Right;
    else if (onTop)    side = ConnectorSide::Top;
    else               side = ConnectorSide::Bottom;
    return true;
}

ConnectorSide oppositeSide(ConnectorSide side) {
    switch (side) {
        case ConnectorSide::Left:   return ConnectorSide::Right;
        case ConnectorSide::Right:  return ConnectorSide::Left;
        case ConnectorSide::Top:    return ConnectorSide::Bottom;
        case ConnectorSide::Bottom: return ConnectorSide::Top;
    }
    return ConnectorSide::Left;
}

void initFragments() {
    fragments.clear();

    namespace fs = std::filesystem;
    std::error_code ec;
    if (fs::exists(FRAGMENTS_DIR, ec) && fs::is_directory(FRAGMENTS_DIR, ec)) {
        for (const auto& entry : fs::directory_iterator(FRAGMENTS_DIR, ec)) {
            if (ec || !entry.is_regular_file()) continue;

            std::string fname = entry.path().filename().string();
            int id = 0;
            if (sscanf(fname.c_str(), "FRAGMENT_%d.json", &id) != 1)
                continue; // not a fragment file — ignore

            Fragment fragment;
            fillEmpty(fragment);
            fragment.width  = 1;
            fragment.height = 1;
            snprintf(fragment.name, sizeof(fragment.name), "Fragment %d", id);

            if (loadFragmentFromFile(fragment, entry.path().string().c_str()))
                fragments[id] = fragment;
        }
    }
}
