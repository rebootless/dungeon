#pragma once

#include <string>
#include <vector>

#include <SDL2/SDL.h>

#include "../core/display.h"
#include "../core/layout.h"
#include "../core/level.h"
#include "../core/renderer.h"
#include "../core/tiles.h"
#include "fragment.h"

/*
Shared fragment-editor runtime state
Same split as EditorMode/editor_state.h: fragment_editor_mode.cpp owns
onEnter()/onRender() and the save/load persistence helpers;
fragment_editor_controls.cpp owns onEvent(). Both are member functions of
FragmentEditorMode, sharing state through the extern globals below —
fragment_editor_mode.cpp is the one place that defines (allocates) each
of them.

Kept fully independent from editor_state.h's ed*Map/mtMap/EditLayer —
the two editors never run at the same time, but each owns its own
buffers rather than sharing mutable global state across unrelated modes.
*/

constexpr int PALETTE_CELL   = CELL_SIZE;
/*
The palette grid starts at cell (1,1) instead of (0,0) — a one-cell
margin on its top and left, same convention as EditorMode's.
*/
constexpr int PALETTE_MARGIN = CELL_SIZE;

/*
Row offset
Canvas row 0 sits exactly under the outer Frame's own top wall, and the
map's last row sits exactly under the map/info-box divider row — both
drawn last, after everything else, in every mode that shares this layout
(see fragment_editor_mode.cpp's Frame section, and layout.h's row-budget
comment). GameMode/EditorMode never notice, since world levels are
authored with those two rows treated as permanent border wall — but a
fragment is anchored at its OWN (0,0) by design (arrow keys grow it from
the top-left corner), so without this offset its top edge — tiles and
the red border overlay alike — would always render invisible, no matter
what's actually painted there.

Shifting every fr*Map cell down by one before drawing clears the top
wall; MAX_FRAGMENT_HEIGHT below then keeps the new bottom-most visible
row clear of the divider in turn. Purely a render/hit-test offset — the
buffers underneath still start at row 0, and fragment.h's Fragment JSON
format is completely unaffected.
*/
constexpr int FRAGMENT_ROW_OFFSET = CELL_SIZE;
constexpr int MAX_FRAGMENT_HEIGHT = MAX_HEIGHT - 2;

// Fragment-editor map state — same five layers as EditorMode, plus a
// sixth connector layer marking candidate stitching points.
extern TileID frGroundMap    [MAX_HEIGHT][MAX_WIDTH];
extern TileID frObjectMap    [MAX_HEIGHT][MAX_WIDTH];
extern TileID frEntityMap    [MAX_HEIGHT][MAX_WIDTH];
extern TileID frCollisionMap [MAX_HEIGHT][MAX_WIDTH];
extern TileID frOcclusionMap [MAX_HEIGHT][MAX_WIDTH];
extern TileID frConnectorMap [MAX_HEIGHT][MAX_WIDTH];

// Multitile occupancy tracking — same shape as EditorMode's MultiTileCell.
struct FragmentMultiTileCell {
    bool occupied;
    int  anchorX;
    int  anchorY;
};
extern FragmentMultiTileCell frMtMap[MAX_HEIGHT][MAX_WIDTH];

// Tile palette
extern std::vector<TileID> frAvailableTiles;
extern TileID               frSelectedTile;

/*
Layer management
CONNECTOR is the one addition over EditorMode's EditLayer — core/tiles.h
has no sprite for it, only the CONNECTOR_MARKER sentinel (see frPlaceTile
in fragment_editor_controls.cpp), same idea as COLLISION/OCCLUSION.
*/
enum class FragmentEditLayer { GROUND = 1, OBJECTS = 2, ENTITIES = 3, COLLISION = 4, OCCLUSION = 5, CONNECTOR = 6 };
extern FragmentEditLayer frActiveLayer;

TileID (*frGetLayerMap(FragmentEditLayer layer))[MAX_WIDTH];
FragmentEditLayer frLayerForTile(TileID id);

// Editor session state
extern std::string frEditorStatus;
extern int         frEditorStatusTTL;
extern int         frPaletteScroll;
extern bool        frMouseDown;
extern bool        frRightMouseDown;

/*
Fragment selection
selectedFragmentId is the id F5 saves to and F9 loads from — stepped
with PageUp/PageDown, since the arrow keys are reserved for
fragmentWidth/fragmentHeight below. No text-entry naming: fragments are
identified purely by number; renaming the descriptive Fragment::name
field (if wanted) is done by hand in the saved JSON file. Clicking a row
in the right-panel list (rendered from listFragmentIds()) jumps straight
to that id instead of stepping to it.
*/
extern int selectedFragmentId;
extern int fragmentListScroll; // independent of the palette's frPaletteScroll

/*
Fragment size
The intended footprint, grown/shrunk from the fixed top-left corner
(0,0) by the arrow keys (see fragment_editor_controls.cpp). Purely
descriptive, exactly like Fragment::width/height (fragment.h) —
painting is never restricted to it; it only affects where the
checkerboard background is drawn in FragmentEditorMode::onRender().
fragmentHeight is capped at MAX_FRAGMENT_HEIGHT, not MAX_HEIGHT — see
that constant's comment above for why.
*/
extern int fragmentWidth;
extern int fragmentHeight;

/*
Collision layer carries three different logical markers, same as
EditorMode's CollisionTool — which one 1/2/3 places is tracked here.
*/
enum class FragmentCollisionTool { BLOCK, STAIRS_UP, STAIRS_DOWN };
extern FragmentCollisionTool frActiveCollisionTool;

/*
Palette layout (shelf packing) — same algorithm as EditorMode's
computePaletteLayout (editor_state.h), kept as its own instance since it
reads/writes frAvailableTiles rather than availableTiles.
*/
struct FragmentPaletteEntry { TileID id; SDL_Rect rect; };
std::vector<FragmentPaletteEntry> computeFragmentPaletteLayout(int panelW);

/*
Painting actions
Mutate the fr*Map/frMtMap state above — implemented in
fragment_editor_controls.cpp, since only mouse input ever calls them.
*/
void frPlaceTile(int gx, int gy);
void frEraseTile(int gx, int gy);

/*
Persistence
F5/F9 — implemented in fragment_editor_mode.cpp (alongside
clearFragmentEditorMaps/rebuildFragmentMultiTileOccupancy, which they
share with onEnter()'s startup load), called from
fragment_editor_controls.cpp's onEvent().
*/
void saveFragmentEditorTo(int id);
void loadFragmentIntoEditor(int id);
