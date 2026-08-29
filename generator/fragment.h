#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "../core/level.h" // MAX_WIDTH / MAX_HEIGHT / TileID

/*
Fragments directory
Every fragment is one JSON file in this folder, at the project root —
same convention as core/level.h's WORLD_DIR. Created on demand the first
time something is saved (see saveFragmentToFile).
*/
constexpr const char* FRAGMENTS_DIR = "fragments";

// File name for a fragment's JSON, e.g. "fragments/FRAGMENT-03.json".
std::string fragmentFileName(int id);

/*
Fragment data
A hand-authored dungeon piece (room, corridor, hall, ...) meant to be
stitched together later by the procedural generator. Same five layers as
Level, plus a sixth connector layer marking candidate stitching points
(see core/tiles.h's CONNECTOR_MARKER). Unlike Level, a fragment carries
no world position — it's identified purely by a numeric id, since there
is no coordinate grid to place it on ahead of time.
*/
struct Fragment {
    TileID tileMap     [MAX_HEIGHT][MAX_WIDTH]; // Layer 1: Ground
    TileID objectMap   [MAX_HEIGHT][MAX_WIDTH]; // Layer 2: Objects
    TileID entityMap   [MAX_HEIGHT][MAX_WIDTH]; // Layer 3: Entities
    TileID collisionMap[MAX_HEIGHT][MAX_WIDTH]; // Layer 4: Collision markers
    TileID occlusionMap[MAX_HEIGHT][MAX_WIDTH]; // Layer 5: Occlusion markers
    TileID connectorMap[MAX_HEIGHT][MAX_WIDTH]; // Layer 6: Connector markers

    /*
    Intended footprint of the fragment, grown/shrunk from the fixed
    top-left corner (0,0) by FragmentEditorMode's arrow keys. Purely
    descriptive — painting is never restricted to it, and the full
    MAX_WIDTH x MAX_HEIGHT buffer is always saved regardless of what
    width/height currently is (same convention as Level.width/height).
    */
    int  width  = 1;
    int  height = 1;
    char name[64] = "New Fragment"; // Descriptive label — not the file id; rename by hand in the saved JSON
};

/*
All currently loaded fragments, keyed by id. Populated by initFragments()
at startup (and refreshed by the fragment editor after a save).
unordered_map gives stable references/pointers across inserts — safe for
FragmentEditorMode to hold a Fragment* across frames.
*/
extern std::unordered_map<int, Fragment> fragments;

// Scans fragments/*.json, loading every file matching FRAGMENT-<id>.json
// into `fragments`. No fallback content: if fragments/ is empty,
// `fragments` ends up empty too.
void initFragments();

// Looks up an already-loaded fragment. Returns nullptr if none is loaded
// under that id.
Fragment* findFragment(int id);

/*
Returns the fragment with `id`, creating a fresh blank one in-memory
(registered in `fragments`, but NOT written to disk) if it doesn't
already exist. Used by the fragment editor when the selected id doesn't
correspond to a saved file yet.
*/
Fragment& getOrCreateFragment(int id);

// All currently loaded fragment ids, sorted ascending.
std::vector<int> listFragmentIds();

// Load one fragment from a JSON file produced by the fragment editor.
// Returns true on success; fragment is left unchanged on failure.
bool loadFragmentFromFile(Fragment& fragment, const char* path);

// Write `fragment` out as JSON to `path` (creating parent directories as
// needed). Returns true on success.
bool saveFragmentToFile(const Fragment& fragment, const char* path);
