#pragma once

#include "../core/level.h" // MAX_WIDTH / MAX_HEIGHT / TileID
#include "fragment.h"

/*
Dungeon generator
Assembles a MAX_WIDTH x MAX_HEIGHT canvas out of the fragments currently
loaded in `fragments` (generator/fragment.h), stitched together at their
connector markers. Used by GeneratorMode ([R] to (re)run it) — this file
holds only the placement logic, no rendering or input handling, same
split as game/interactions.h from game/game_mode.cpp.

Placement rule: two fragments' bounding boxes (their width x height
rectangle — see fragment_editor_mode.cpp's red border overlay) may never
overlap; a new fragment is only ever placed so that one of its connector
cells lands exactly one cell beyond an existing fragment's connector cell,
in the direction that connector's border faces — touching, never sharing
a cell. See fragment.h's connectorSideAt()/oppositeSide().
*/
struct GeneratedDungeon {
    TileID groundMap    [MAX_HEIGHT][MAX_WIDTH];
    TileID objectMap    [MAX_HEIGHT][MAX_WIDTH];
    TileID entityMap    [MAX_HEIGHT][MAX_WIDTH];
    TileID collisionMap [MAX_HEIGHT][MAX_WIDTH];
    TileID occlusionMap [MAX_HEIGHT][MAX_WIDTH];
    int fragmentCount = 0; // how many fragments actually got placed
};

/*
Picks a random fragment from `fragments` as a seed, then repeatedly picks
a random still-open connector on an already-placed fragment and tries to
attach a random other fragment (or the same one again) to it, until
either maxFragments are placed, no open connectors remain, or too many
consecutive placement attempts fail. Returns false only if `fragments` is
empty — an unfavorable random layout still returns true with whatever it
managed to place (fragmentCount may be as low as 1, the seed alone).
*/
bool generateDungeon(GeneratedDungeon& out, int maxFragments = 12);
