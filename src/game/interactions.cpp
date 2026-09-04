#include "interactions.h"

#include <unordered_map>

/*
Interactive objects
Only objects listed here respond to anything at all — everything else on
the Objects layer (walls, floor clutter, weapon crates, ...) is purely
decorative as far as GameMode is concerned. Weapon crates are deliberately
NOT registered, so they have no interaction at all.

Interact (E) and Attack (Space) are two SEPARATE tables, each keyed by the
object's current tile — not one shared table filtered by a control field —
because the barrel needs to answer both from the same state (E opens/
closes it, Space breaks it outright) and a single TileID -> Interaction
map can only ever hold one entry per key.

Which control does what, and in which order the frames play, is still
decided right here — that part is genuine game logic, not tile metadata.
Only the underlying TileIDs are no longer written literally: each table
asks core/tiles.h's interactiveFrames(object, tier) for the object's
frame sequence (in tiles.json's own col order) and builds its rows from
that, so a tiles.json edit (new tier, reordered frames, a renamed file)
never needs a matching code change here unless the actual open/close/
break BEHAVIOR is what's changing.
*/

/*
Interact (E) table
Containers/furniture you open: chests, cabinets, nightstands, the
wardrobe, doors, the shrine, and the barrel's lid (breaking it outright is
Attack's job — see below).
*/
static const std::unordered_map<TileID, Interaction>& interactTable() {
    static const std::unordered_map<TileID, Interaction> t = [] {
        std::unordered_map<TileID, Interaction> m;

        // Chests — 7 tiers, 4-frame open animation each (closed -> two
        // opening frames -> opened). Closed opens forward through all 4
        // frames; open closes back through them in reverse.
        for (int tier = 1; tier <= 7; ++tier) {
            std::vector<TileID> f = interactiveFrames("chest", tier);
            if (f.size() != 4) continue; // tiles.json entry missing/incomplete — skip rather than crash
            m[f[0]] = { { f[0], f[1], f[2], f[3] }, "You open the chest." };
            m[f[3]] = { { f[3], f[2], f[1], f[0] }, "You close the chest." };
        }

        // Cabinet — single closed/open frame.
        if (std::vector<TileID> f = interactiveFrames("cabinet"); f.size() == 2) {
            m[f[0]] = { { f[0], f[1] }, "You open the cabinet." };
            m[f[1]] = { { f[1], f[0] }, "You close the cabinet." };
        }

        // Nightstand — single closed/open frame.
        if (std::vector<TileID> f = interactiveFrames("nightstand"); f.size() == 2) {
            m[f[0]] = { { f[0], f[1] }, "You open the nightstand." };
            m[f[1]] = { { f[1], f[0] }, "You close the nightstand." };
        }

        // Wardrobe — 1x2, single closed/open frame.
        if (std::vector<TileID> f = interactiveFrames("wardrobe"); f.size() == 2) {
            m[f[0]] = { { f[0], f[1] }, "You open the wardrobe." };
            m[f[1]] = { { f[1], f[0] }, "You close the wardrobe." };
        }

        // Barrel lid — closed <-> open, freely reversible (unlike the
        // Attack table's break below, this never reaches the broken frame).
        if (std::vector<TileID> f = interactiveFrames("barrel"); f.size() == 3) {
            m[f[0]] = { { f[0], f[1] }, "You open the barrel." };
            m[f[1]] = { { f[1], f[0] }, "You close the barrel." };
        }

        /*
        Doors — 3 tiers, 4-frame open animation each (same shape as the
        chests: closed -> two opening frames -> opened). Closed opens
        forward through all 4 frames; open closes back through them in
        reverse. Blocking itself isn't handled here — see isDoorBlocking()
        below, which movement collision consults directly instead of a
        table lookup, so a door never needs a COLLISION_MARKER painted
        under it.
        */
        for (int tier = 1; tier <= 3; ++tier) {
            std::vector<TileID> f = interactiveFrames("door", tier);
            if (f.size() != 4) continue;
            m[f[0]] = { { f[0], f[1], f[2], f[3] }, "You open the door." };
            m[f[3]] = { { f[3], f[2], f[1], f[0] }, "You close the door." };
        }

        // Shrine — reversible toggle between its two frames.
        if (std::vector<TileID> f = interactiveFrames("shrine"); f.size() == 2) {
            m[f[0]] = { { f[0], f[1] }, "The shrine flickers to life." };
            m[f[1]] = { { f[1], f[0] }, "The shrine falls dark again." };
        }

        return m;
    }();
    return t;
}

/*
Attack (Space) table
Things you break: the barrel (from either lid state — closed or already
opened, one hit reduces it straight to splinters) and both vase sizes.
All terminal/one-way: the resulting tile has no entry in either table, so
a broken object doesn't respond to anything further.
*/
static const std::unordered_map<TileID, Interaction>& attackTable() {
    static const std::unordered_map<TileID, Interaction> t = [] {
        std::unordered_map<TileID, Interaction> m;

        // Barrel — smashed from closed plays through the open frame on
        // the way to broken for a meatier animation; smashed from
        // already-open skips straight to broken.
        if (std::vector<TileID> f = interactiveFrames("barrel"); f.size() == 3) {
            m[f[0]] = { { f[0], f[1], f[2] }, "You smash the barrel to splinters." };
            m[f[1]] = { { f[1], f[2] },       "You smash the barrel to splinters." };
        }

        // Vases — whole -> broken (terminal, one-way).
        if (std::vector<TileID> f = interactiveFrames("vase"); f.size() == 2)
            m[f[0]] = { { f[0], f[1] }, "You shatter the vase." };

        if (std::vector<TileID> f = interactiveFrames("vase_small"); f.size() == 2)
            m[f[0]] = { { f[0], f[1] }, "You shatter the small vase." };

        return m;
    }();
    return t;
}

/*
Step table
The trap is the only object triggered by walking onto it rather than by
Interact/Attack — see interactions.h's findStepTrigger and
game/game_controls.cpp's post-move check. Terminal, one-way: nothing
re-arms a trap once sprung.
*/
static const std::unordered_map<TileID, Interaction>& stepTable() {
    static const std::unordered_map<TileID, Interaction> t = [] {
        std::unordered_map<TileID, Interaction> m;

        if (std::vector<TileID> f = interactiveFrames("trap"); f.size() == 2)
            m[f[0]] = { { f[0], f[1] }, "The trap snaps shut!" };

        return m;
    }();
    return t;
}

const Interaction* findInteraction(TileID obj, Control control) {
    const auto& t = (control == Control::Interact) ? interactTable() : attackTable();
    auto it = t.find(obj);
    if (it == t.end()) return nullptr;
    return &it->second;
}

const Interaction* findStepTrigger(TileID obj) {
    const auto& t = stepTable();
    auto it = t.find(obj);
    if (it == t.end()) return nullptr;
    return &it->second;
}

bool isDoorBlocking(TileID obj) {
    /*
    A door blocks unless it's showing its LAST registered frame (fully
    open) — tileObjectName/tileObjectFrameIndex (core/tiles.h) already
    know how to answer this for a sub-tile (e.g. a door's bottom half)
    just as well as for the anchor, since tiles.cpp propagates that
    information onto every synthesized sub-id at load time. `tier` comes
    along with it, so this works uniformly across all 3 door tiers
    without hardcoding any of them here.
    */
    if (tileObjectName(obj) != "door") return false;

    int tier = tileObjectTier(obj);
    int frameIndex = tileObjectFrameIndex(obj);
    int frameCount = (int)interactiveFrames("door", tier).size();
    return frameIndex < frameCount - 1;
}
