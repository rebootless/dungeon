#ifndef TILES_H
#define TILES_H

#include <cstdint>
#include <string>
#include <vector>

/*
Base tile types
TileID used to directly encode X/Y coordinates on a single spritesheet
atlas. Since the assets refactor split every tile into its own file under
assets/tiles/, a TileID is now an opaque handle into the registry loaded
by loadTileRegistry() from assets/tiles/tiles.json — nothing about its
numeric value has any spatial meaning any more (see makeTileId below for
the one exception: real, JSON-authored ids, which pack their own 2-
character name directly).

This is also what makes the asset refactor pay off in practice: adding a
tile is dropping a PNG into assets/tiles/ and adding an entry to
tiles.json, then relaunching — nothing under src/ ever needs to change or
recompile for it.
*/
using TileID = uint16_t;
constexpr TileID EMPTY_ID = 0xFFFF; // Sentinel for empty/null tile

/*
Packs a tiles.json entry's 2-character "id" field into a TileID. Each
character must be from [0-9A-Z] (36 symbols), packed as raw bytes —
c0 in the high byte, c1 in the low byte — so every id maps to a distinct
value in [0x3030, 0x5A5A], nowhere near the sentinel range below. This is
a direct pack, not a hash: two different ids can never collide, and
nothing about it depends on where the entry sits in tiles.json.
*/
constexpr TileID makeTileId(char c0, char c1) {
    return (TileID)(((uint8_t)c0 << 8) | (uint8_t)c1);
}

// Sentinel stored in the collision layer to mark a cell as blocked.
// Never rendered as a tile sprite; drawn as a yellow outline in the editor.
constexpr TileID COLLISION_MARKER = 0xFFFE;

/*
Sentinels stored in the collision layer to mark floor-transition cells.
Same idea as COLLISION_MARKER — a logical marker, not a placeable tile.
The decorative stairs sprites are placed on the Ground/Objects layer
purely for visuals (plain "manual" tiles.json entries, like any other
decoration); these markers are what GameMode actually reads to trigger a
floor change. Drawn as a cyan/magenta outline in the editor.
*/
constexpr TileID STAIRS_UP_MARKER   = 0xFFFD;
constexpr TileID STAIRS_DOWN_MARKER = 0xFFFC;

/*
Sentinel stored in the occlusion layer. Purely a gameplay/rendering hint
(e.g. "fade the roof over this cell when the player stands under it") —
never rendered as a sprite of its own. Drawn as a translucent violet
outline in the editor, invisible in GameMode.
*/
constexpr TileID OCCLUSION_MARKER = 0xFFFB;

// Sentinel stored in a Fragment's connector layer — see generator/fragment.h.
constexpr TileID CONNECTOR_MARKER = 0xFFFA;

/*
Special sprites
Five images (assets/border_horizontal.png, assets/border_vertical.png,
assets/border_corner.png, assets/player.png, assets/cursor.png) are
deliberately NOT part of the tiles.json registry: they are not
user-placeable content, so there is nothing to look up for them. They get
their own reserved TileID constants instead, resolved directly against
hardcoded textures in core/renderer.cpp rather than through
getTileMeta()/the registry. Placed just below CONNECTOR_MARKER, still
nowhere near the [0x3030, 0x5A5A] range makeTileId can ever produce.
*/
constexpr TileID PLAYER            = 0xFFF9; // Entities-layer spawn marker
constexpr TileID FACING_INDICATOR  = 0xFFF8; // Overlay drawn on the cell the player faces
constexpr TileID HORIZONTAL_BORDER = 0xFFF7; // FrameBuilder — see core/renderer.h
constexpr TileID VERTICAL_BORDER   = 0xFFF6;
constexpr TileID CORNER_BORDER     = 0xFFF5;

// Which of the five layers a tile belongs to. Collision markers/occlusion
// markers are sentinels, not registry entries, so they have no LayerType
// of their own — see EditLayer (editor/editor_state.h) for the full
// five-layer picture the editor works with.
enum class LayerType : uint8_t { Ground, Objects, Entities };

// How a tile's concrete TileID gets chosen. See assets/tiles/tiles.json's
// "mode" field.
enum class TileMode : uint8_t {
    Manual,         // one independently palette-pickable tile
    Random,         // one of a group's members, picked uniformly at paint time
    Interactive,    // frame 0 of an object/tier's animation is the only one paletteVisible
    AutotileBlend,  // one piece of a 3x3-plus-corners material blend
    AutotileBlob,   // one cell of a neighbour-driven blob sheet (walls, rails)
};

/*
Resolved metadata for one TileID — either a real tiles.json entry, or a
synthesized sub-cell of one (see subTileId()). Rendering only ever needs
this: `file` + the srcCell rect it names is everything core/renderer.cpp
needs to blit the right pixels, and `layer`/`w`/`h` is everything
placeTile()/eraseTile() need to stamp or clear a footprint. An id with no
registry entry at all (EMPTY_ID, a sentinel, one of the special-sprite
constants above, or simply unknown) resolves to a safe, invisible
default — `file` empty, 1x1, not palette-visible.
*/
struct TileMetadata {
    LayerType   layer          = LayerType::Ground;
    uint8_t     w              = 1, h = 1; // footprint, in cells
    std::string file;                       // filename under assets/tiles/, "" if not a real sprite
    int         srcCellX = 0, srcCellY = 0; // top-left of this tile's art within `file`, in CELL_SIZE units
    bool        paletteVisible = false;
};

TileMetadata getTileMeta(TileID id);

/*
Loads assets/tiles/tiles.json into the registry. Must be called exactly
once at startup (see core/app.cpp's App::run()), before any other
function in this header is used. Aborts the process with a message on
stderr if the file is missing/malformed, an id isn't exactly two
characters from [0-9A-Z], or two entries share an id — these are all
authoring mistakes in tiles.json, not situations to silently paper over.
*/
void loadTileRegistry();

/*
Every palette-pickable TileID, in tiles.json's own order: every "manual"
entry, one representative per "random" group (its first member), and the
frame-0 entry of every "interactive" object/tier — but NOT the other
frames of an interactive object (see TileMode::Interactive), and NOT the
individual pieces of an autotile_blend/autotile_blob material (those are
painted via their group's single representative — see
getAutotilePaletteTiles()). Used by editor/editor_state.h's
computePaletteLayout() and the fragment editor's equivalent.
*/
std::vector<TileID> getPaletteTiles();

/*
One representative TileID per autotile material (blend or blob), in
tiles.json order — the single palette entry that stands in for the whole
group. Painting with one of these picks the correct concrete piece for
each affected cell automatically; see resolveAutotileBlend()/
resolveAutotileBlob() below.
*/
std::vector<TileID> getAutotilePaletteTiles();

/*
Sub-cell addressing for multi-cell tiles
Every cell of a >1x1 tile needs its own TileID stored in the map (see
editor/editor_controls.cpp's placeTile/eraseTile) so that any render loop
can stay a dumb "draw whatever TileID is at (x,y)" per-cell pass with no
footprint awareness of its own — exactly like the game's render loops
already are. subTileId(anchorId, 0, 0) is anchorId itself; every other
offset is synthesized once, at registry-load time, into an id from a
reserved range that never overlaps a real tiles.json id or a sentinel.
Returns EMPTY_ID if `anchorId` isn't a registered multi-cell entry or
(dx, dy) falls outside its footprint.
*/
TileID subTileId(TileID anchorId, int dx, int dy);

/*
The inverse of subTileId: given ANY raw TileID off the map (anchor or
synthesized sub-cell), returns the anchor id it belongs to and sets
(dx, dy) to its offset within that anchor's footprint — (0, 0) if `id`
already is an anchor (or isn't part of any multi-cell entry at all).
Lets a caller that only has one raw cell's TileID recover which grid
position actually anchors it, without needing an editor-style mtMap
occupancy table of its own — see game/game_controls.cpp's
resolveInteractionAnchor.
*/
TileID anchorOf(TileID id, int& dx, int& dy);

/*
Random-fill groups
If `id` is the palette representative (or any member) of a "random"
group, returns a uniformly random member of that same group — otherwise
returns `id` unchanged. editor_controls.cpp's placeTile calls this once
per stamp, so painting with e.g. "grass" drops a different blade pattern
on every click without the caller needing to know groups exist at all.
*/
TileID pickRandomVariant(TileID id);

/*
Autotile resolution
Given the palette's representative id for an autotile_blend/autotile_blob
material (or any already-placed member of the same material — re-running
this on a neighbour after an edit uses that cell's own current id, not
the palette selection) and which of its 8 neighbours currently belong to
the SAME material, returns the correct concrete piece to store there.
Called by editor_controls.cpp's placeTile/eraseTile on the edited cell and
every orthogonal neighbour that also belongs to the material, exactly
like a classic paint-time autotiler (Tiled/RPG Maker) — GameMode never
needs to know autotiling exists at all, since by the time a level reaches
disk every cell already holds its final, concrete TileID.
*/
TileID resolveAutotileBlend(TileID id, bool n, bool s, bool e, bool w,
                             bool ne, bool nw, bool se, bool sw);
TileID resolveAutotileBlob(TileID id, bool n, bool s, bool e, bool w);

// True if `id` belongs to an autotile_blend/autotile_blob material at
// all — editor_controls.cpp uses this to decide whether placing/erasing a
// tile needs the neighbour re-evaluation dance above. The Blend/Blob
// variants tell it which of resolveAutotileBlend/resolveAutotileBlob to
// call for that cell.
bool isAutotileTile(TileID id);
bool isAutotileBlend(TileID id);
bool isAutotileBlob(TileID id);

// True if `a` and `b` belong to the same autotile/random group — the
// neighbour test resolveAutotileBlend/Blob and pickRandomVariant need.
bool sameTileGroup(TileID a, TileID b);

/*
Interactive objects
`object` is the stable family name (e.g. "chest", "door", "barrel") and
`tier` distinguishes sub-variants that share that name (e.g. chest_tier1
.. chest_tier7); tier is 1 for objects with only one variant. Frames are
returned in tiles.json's own col order — frame 0 is always the resting/
closed state and the only one the editor can place (see
game/interactions.cpp, the one place that turns these frame sequences
into actual open/close/break behaviour).
*/
std::vector<TileID> interactiveFrames(const std::string& object, int tier = 1);

// "" / 1 / "" if `id` isn't part of any registered interactive object —
// used by game/interactions.h's isDoorBlocking() to recognize a door
// tile (anchor OR sub-tile) without hardcoding a TileID range.
std::string tileObjectName(TileID id);
int         tileObjectTier(TileID id);
int         tileObjectFrameIndex(TileID id); // position within interactiveFrames()

#endif // TILES_H
