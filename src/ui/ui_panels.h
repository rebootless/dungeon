#pragma once

#include <string>
#include <vector>

/*
UI panel text — every mode, one file
Every mode's on-screen text — panel labels, reserved/placeholder rows,
transient status-message wording, help-screen content — used to live in
its own mode/*_panel.h + .cpp pair (game/game_panel, editor/editor_panel,
generator/fragment_editor_panel, settings/settings_panel,
help/help_panel). All five are merged into this one file instead, so
editing any on-screen wording is always "open ui_panels.cpp" rather than
"remember which of five files owns this string". Each mode still gets
its own namespace below, unchanged from its old file — GameMode reaches
into GamePanel::, EditorMode into EditorPanel::, and so on, exactly as
before; only the file (and, for editor_controls.cpp/
fragment_editor_controls.cpp, whether the text was reachable by name at
all) changed.
*/

// ============================================================
// GameMode
// ============================================================

/*
GameMode info panel layout
Text drawn into the info box below the map (see GameMode::render() in
game_mode.cpp) and into the two side panels. All three follow the same
shape: a std::vector<std::string>, one row per entry, top to bottom,
drawn by game_mode.cpp's drawPanelLines(). The info box is capped at 12
rows (row 11 as bottom margin — 12 is the hard ceiling, see
core/layout.h's UI_BOX_ROWS); the side panels run the full canvas height,
48 usable rows (row 0 is the window's own top wall, row 49 its bottom
wall — see core/layout.h's CANVAS_H/CELL_SIZE).

Every hardcoded label, placeholder value, and blank reserved row for all
three panels lives here — game_mode.cpp only ever reaches into GamePanel
by name, never spells out panel text of its own. Any *_EXTRA_LINES block
below is exactly as many rows as still fit after the content built above
it, ready to hardcode text into or hand a variable to one row at a time;
"" draws as a blank line.
*/
namespace GamePanel {

    /*
    Builds the info box's rows fresh, every call — message goes into row
    0, INFO_BOX_EXTRA_LINES fills rows 1-10, and row 11 stays blank as
    the box's own bottom margin (see this section's comment above).
    */
    std::vector<std::string> buildInfoBoxLines(const std::string& message);

    // Reserved rows for the info box, drawn directly below the live
    // message line. 10 entries — everything the 12-row cap leaves after
    // the message (row 0) and the fixed bottom-margin row (row 11).
    extern const std::vector<std::string> INFO_BOX_EXTRA_LINES;

    /*
    Left panel: location + stat block
    location gets LOCATION_LABEL_PREFIX prepended automatically. statLines
    is however many pre-built "icon ABBR - value" lines the caller wants —
    see buildStatLine() and the ICON_ / STAT_VALUE_ constants below —
    placed directly after location, one per row. zoomLabel follows the
    stat block; LEFT_PANEL_EXTRA_LINES fills every row still left after
    that up to the panel's 48-row ceiling.
    */
    extern const std::string LOCATION_LABEL_PREFIX; // " Location: "
    std::vector<std::string> buildLeftPanelLines(const std::string& location,
                                                  const std::vector<std::string>& statLines,
                                                  const std::string& zoomLabel);

    // Reserved rows for the left panel, drawn after location/stats/zoom.
    // 35 entries — everything the panel's 48-row ceiling leaves after
    // location (1) + a blank (1) + the 9 stat lines + a blank (1) +
    // zoom (1) = 13 rows already spoken for.
    extern const std::vector<std::string> LEFT_PANEL_EXTRA_LINES;

    /*
    One "icon ABBR - value" stat row, e.g. buildStatLine(ICON_HP, "HP",
    STAT_VALUE_HP) -> " \uf004 HP - 100/100". Icons are plain Unicode
    characters, chosen so the row renders correctly with any font.
    */
    std::string buildStatLine(const std::string& icon, const std::string& abbr, const std::string& value);

    extern const std::string ICON_HP;  // Health
    extern const std::string ICON_STA; // Stamina — rolls, sprinting, heavy attacks
    extern const std::string ICON_MP;  // Mana — spells
    extern const std::string ICON_ATK; // Attack — physical damage
    extern const std::string ICON_DEF; // Defense — damage reduction
    extern const std::string ICON_DEX; // Dexterity — attack speed, bows, dodge chance
    extern const std::string ICON_INT; // Intelligence — magic damage
    extern const std::string ICON_CHA; // Charisma
    extern const std::string ICON_LCK; // Luck — loot, crits, rare events

    // Stat block values, in the same HP/STA/MP/ATK/DEF/DEX/INT/CHA/LCK
    // order game_mode.cpp's render() builds statLines in. Hardcode a
    // real value here, or hand it a variable once actual character stats
    // exist — game_mode.cpp only ever reads these by name.
    extern const std::string STAT_VALUE_HP;
    extern const std::string STAT_VALUE_STA;
    extern const std::string STAT_VALUE_MP;
    extern const std::string STAT_VALUE_ATK;
    extern const std::string STAT_VALUE_DEF;
    extern const std::string STAT_VALUE_DEX;
    extern const std::string STAT_VALUE_INT;
    extern const std::string STAT_VALUE_CHA;
    extern const std::string STAT_VALUE_LCK;

    // Right panel — reserved rows, one entry per row, filling the
    // panel's full 48-row height. "" draws as a blank line.
    extern const std::vector<std::string> RIGHT_PANEL_LINES;

    // Static label text
    extern const std::string ZOOM_LABEL_PREFIX; // " Zoom: " — sandwiches getZoom()
    extern const std::string ZOOM_LABEL_SUFFIX; // "x"

} // namespace GamePanel

// ============================================================
// EditorMode
// ============================================================

/*
EditorMode panel layout
Row indices for the text drawn into the info box below the map (see
EditorMode::onRender() in editor_mode.cpp). Row 0 is the panel's own top
row; UI_BOX_ROWS (core/layout.h) is 13, so row indices here must stay
within [0, 11] to leave the box's last row as a clean bottom margin — 12
rows is the hard ceiling.

The STATUS_* strings are the transient F5/F9/tool-selection messages
editor_controls.cpp/editor_mode.cpp assign into editorStatus — the
wording lives here, the live assembly (which one, plus a level name or
filename) still happens at the call site, since only that file has the
data at the moment it's needed.
*/
namespace EditorPanel {

    // Info box rows
    constexpr int ROW_STATUS = 0; // transient F5/F9 save-load status — fixed to the box's first row, same as GamePanel's live message

    // Right panel (world location list) rows
    constexpr int ROW_WORLD_HEADER = 1; // " Floor N (x, y) [new]"
    constexpr int ROW_WORLD_LEGEND = 2; // arrow-key / PageUp/PageDown legend
    /*
    The list itself starts two rows down (header + legend), same offset
    used by both the click hit-test (editor_controls.cpp) and the render
    (editor_mode.cpp) — kept as a named constant so the two can't drift
    apart from each other.
    */
    constexpr int WORLD_LIST_START_ROW = 2;

    // Static label text
    extern const std::string WORLD_LEGEND;

    // F6 loaded an empty (never-saved) coordinate.
    extern const std::string STATUS_EMPTY_CANVAS;
    // F6 loaded a real one — sandwiches the level's name.
    extern const std::string STATUS_LOADED_PREFIX;
    extern const std::string STATUS_LOADED_SUFFIX;
    // F5 saved successfully — sandwiches the level's name, then its file path.
    extern const std::string STATUS_SAVED_PREFIX;
    extern const std::string STATUS_SAVED_MID;
    // F5 failed to write the file.
    extern const std::string STATUS_SAVE_FAILED;

    // 1-6 tool-selection status line, one per marker tool.
    extern const std::string STATUS_PLACING_COLLISION;
    extern const std::string STATUS_PLACING_STAIRS_DOWN;
    extern const std::string STATUS_PLACING_STAIRS_UP;
    extern const std::string STATUS_PLACING_OCCLUSION;
    extern const std::string STATUS_PLACING_LIGHT;
    extern const std::string STATUS_PLACING_SPAWN;

} // namespace EditorPanel

// ============================================================
// FragmentEditorMode
// ============================================================

/*
FragmentEditorMode panel layout
Same idea as EditorPanel above — row indices for the text drawn into the
info box below the map. Row 0 is the panel's own top row; UI_BOX_ROWS
(core/layout.h) is 13, so row indices here must stay within [0, 11].
*/
namespace FragmentEditorPanel {

    // Info box rows
    constexpr int ROW_STATUS = 0; // transient F5/F9 save-load status — fixed to the box's first row, same as GamePanel's live message

    // Right panel (fragment list) rows
    constexpr int ROW_FRAGMENT_HEADER = 1; // " Fragment N (WxH) [new]"
    constexpr int ROW_FRAGMENT_LEGEND = 2; // PageUp/PageDown legend
    constexpr int FRAGMENT_LIST_START_ROW = 2;

    // Static label text
    extern const std::string FRAGMENT_LEGEND;

    // F9 loaded an empty (never-saved) fragment id.
    extern const std::string STATUS_EMPTY_CANVAS;
    // F9 loaded a real one — sandwiches the fragment's name.
    extern const std::string STATUS_LOADED_PREFIX;
    extern const std::string STATUS_LOADED_SUFFIX;
    // F5 saved successfully — sandwiches the fragment's name, then its file path.
    extern const std::string STATUS_SAVED_PREFIX;
    extern const std::string STATUS_SAVED_MID;
    // F5 failed to write the file.
    extern const std::string STATUS_SAVE_FAILED;

    // 1-6/0 tool-selection status line, one per marker tool.
    extern const std::string STATUS_PLACING_COLLISION;
    extern const std::string STATUS_PLACING_STAIRS_DOWN;
    extern const std::string STATUS_PLACING_STAIRS_UP;
    extern const std::string STATUS_PLACING_OCCLUSION;
    extern const std::string STATUS_PLACING_LIGHT;
    extern const std::string STATUS_PLACING_CONNECTOR;
    extern const std::string STATUS_PLACING_SPAWN;

} // namespace FragmentEditorPanel

// ============================================================
// SettingsMode
// ============================================================

/*
SettingsMode panel layout
Row/column indices (grid cells, not pixels) for the single full-window
panel SettingsMode draws — see settings_mode.cpp's onRender(). Unlike
GamePanel/EditorPanel there's no fixed-size map to work around, so these
are just "where things go" rather than a hard row budget.
*/
namespace SettingsPanel {

    constexpr int ROW_TITLE       = 1;
    constexpr int ROW_STATUS      = 2; // transient status message — fixed row, same idea as GamePanel/EditorPanel/FragmentEditorPanel's info box message (see their class comments), not computed from the category/option list length
    constexpr int ROW_HEADERS     = 3;
    constexpr int ROW_LIST_START  = 4;

    constexpr int COL_LEFT  = 2;  // "SETTING" column (categories)
    constexpr int COL_RIGHT = 20; // "OPTION" column (that category's values)

    extern const std::string TITLE;
    extern const std::string HEADER_SETTING;
    extern const std::string HEADER_OPTION;
    extern const std::string VOLUME_PLACEHOLDER;
    extern const std::string VOLUME_STATUS;

} // namespace SettingsPanel

// ============================================================
// HelpMode
// ============================================================

/*
HelpMode panel layout

Left column is a two-level tree:
CONTROLS
  Game
  Editor world
  Editor generator
  Generator
  Settings
  Console
CONSOLE COMMANDS
  Possible commands

Group rows are headers and are not selectable. Only leaves are
selectable.

The right column displays the controls/commands belonging to the
currently selected leaf. Its header tracks the group owning that leaf:
CONTROLS or CONSOLE COMMANDS.
*/
namespace HelpPanel {

    extern const std::string TITLE;

    constexpr int ROW_TITLE          = 1;
    constexpr int ROW_HEADERS        = 3;
    constexpr int ROW_LIST_START     = 3;
    constexpr int ROW_CONTROLS_START = 4;

    constexpr int COL_LEFT  = 2;
    constexpr int COL_RIGHT = 20;

    /*
    Leaf ids

    Contiguous across both groups (0..LEAF_COUNT-1), so keyboard Up/Down
    can increment/decrement one int while the tree itself skips
    non-selectable group-header rows.
    */
    constexpr int CATEGORY_GAME             = 0;
    constexpr int CATEGORY_EDITOR_WORLD     = 1;
    constexpr int CATEGORY_EDITOR_GENERATOR = 2;
    constexpr int CATEGORY_GENERATOR        = 3;
    constexpr int CATEGORY_SETTINGS         = 4;
    constexpr int CATEGORY_CONSOLE          = 5;
    constexpr int CATEGORY_CONSOLE_COMMANDS = 6;
    constexpr int LEAF_COUNT                = 7;

    struct HelpTreeLeaf {
        int id;
        std::string name;
    };

    struct HelpTreeGroup {
        std::string name;
        std::vector<HelpTreeLeaf> leaves;
    };

    /*
    One control entry.

    key:
    Keyboard/mouse input.

    description:
    Action performed by that input.

    The renderer can use these two fields to create a clean aligned
    two-column legend instead of storing pre-formatted strings.
    */
    struct HelpControl {
        std::string key;
        std::string description;
    };

    extern const std::vector<HelpTreeGroup> CATEGORY_TREE;

    /*
    Returns the group owning the specified category.

    Examples:
    CATEGORY_GAME             -> "CONTROLS"
    CATEGORY_EDITOR_WORLD     -> "CONTROLS"
    CATEGORY_CONSOLE          -> "CONTROLS"
    CATEGORY_CONSOLE_COMMANDS -> "CONSOLE COMMANDS"
    */
    std::string headerForCategory(int category);

    extern const std::vector<HelpControl> GAME_CONTROLS;

    extern const std::vector<HelpControl> EDITOR_WORLD_CONTROLS;

    extern const std::vector<HelpControl> EDITOR_GENERATOR_CONTROLS;

    extern const std::vector<HelpControl> GENERATOR_CONTROLS;

    extern const std::vector<HelpControl> SETTINGS_CONTROLS;

    extern const std::vector<HelpControl> CONSOLE_CONTROLS;

    /*
    Possible console commands.

    Kept in sync by hand with core/app.cpp's registerConsoleCommands()/
    dispatchCommand().
    */
    extern const std::vector<std::string> CONSOLE_COMMANDS;

    /*
    Returns the complete control legend for one category.

    The returned entries contain separate key and description fields,
    allowing the renderer to align them into columns.
    */
    std::vector<HelpControl> controlsForCategory(int category);

} // namespace HelpPanel
