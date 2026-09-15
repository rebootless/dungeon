#include "ui_panels.h"

// =======================================================================================================
// GameMode
// =======================================================================================================

namespace GamePanel {

const std::string ZOOM_LABEL_PREFIX = "Zoom: ";
const std::string ZOOM_LABEL_SUFFIX = "x";

const std::string LOCATION_LABEL_PREFIX = "Location: ";

// Stat block values — see ui_panels.h's comment on this block. HP/MP
// carry a trailing space so every value lines up to the same 5-character
// width as the others ("[sta]", "[atk]", ...).
const std::string STAT_VALUE_HP  = "[HP] ";
const std::string STAT_VALUE_STA = "[STA]";
const std::string STAT_VALUE_MP  = "[MP] ";
const std::string STAT_VALUE_ATK = "[ATK]";
const std::string STAT_VALUE_DEF = "[DEF]";
const std::string STAT_VALUE_DEX = "[DEX]";
const std::string STAT_VALUE_INT = "[INT]";
const std::string STAT_VALUE_CHA = "[CHA]";
const std::string STAT_VALUE_LCK = "[LCK]";

/*
Info box reserved rows
10 entries, rows 1-10 of the box (row 0 is the live message, row 11 is
the fixed bottom margin — see ui_panels.h's class comment and
buildInfoBoxLines below). Hardcode text directly into any entry, or
replace one with a variable at the buildInfoBoxLines call site.
*/
const std::vector<std::string> INFO_BOX_EXTRA_LINES = {
//  "", // row 0 (row 0 is the live message)
    "", // row 1
    "", // row 2
    "", // row 3
    "", // row 4
    "", // row 5
    "", // row 6
    "", // row 7
    "", // row 8
    "", // row 9
    "H - Help | ESC - Settings", // row 10
//  "", // row 11 (row 11 is the fixed bottom margin)
};

std::vector<std::string> buildInfoBoxLines(const std::string& message) {
    std::vector<std::string> lines;
    lines.push_back(message);
    for (const std::string& line : INFO_BOX_EXTRA_LINES) lines.push_back(line);
    lines.push_back(""); // row 11 — fixed bottom margin, never part of the reserved block above
    return lines;
}

/*
Left panel reserved rows
35 entries, filling the panel's 48-row ceiling out from row 13 (location,
a blank, the 9 stat lines, a blank, and zoom already fill rows 0-12 —
see ui_panels.h's class comment and buildLeftPanelLines below).
*/
const std::vector<std::string> LEFT_PANEL_EXTRA_LINES = {
    "", // row 13
    "", // row 14
    "", // row 15
    "", // row 16
    "", // row 17
    "", // row 18
    "", // row 19
    "", // row 20
    "", // row 21
    "", // row 22
    "", // row 23
    "", // row 24
    "", // row 25
    "", // row 26
    "", // row 27
    "", // row 28
    "", // row 29
    "", // row 30
    "", // row 31
    "", // row 32
    "", // row 33
    "", // row 34
    "", // row 35
    "", // row 36
    "", // row 37
    "", // row 38
    "", // row 39
    "", // row 40
    "", // row 41
    "", // row 42
    "", // row 43
    "", // row 44
    "", // row 45
    "", // row 46
    "", // row 47
};

std::vector<std::string> buildLeftPanelLines(const std::string& location,
                                             const std::vector<std::string>& statLines,
                                             const std::string& zoomLabel) {
    std::vector<std::string> lines;

    lines.push_back(LOCATION_LABEL_PREFIX + location);
    lines.push_back("");

    for (const std::string& line : statLines)
        lines.push_back(line);

    lines.push_back("");
    lines.push_back(zoomLabel);

    for (const std::string& line : LEFT_PANEL_EXTRA_LINES) lines.push_back(line);

    return lines;
}

std::string buildStatLine(const std::string& icon, const std::string& abbr, const std::string& value) {
    return " " + icon + " " + abbr + " - " + value;
}

// Plain Unicode glyphs, chosen to render correctly with any font.
const std::string ICON_HP   = "\uf004";
const std::string ICON_STA  = "\uf0e7";
const std::string ICON_MP   = "\U000F058C";
const std::string ICON_ATK  = "\U000F0787";
const std::string ICON_DEF  = "\U000F0498";
const std::string ICON_DEX  = "\U000F1841";
const std::string ICON_INT  = "\U000F09D1";
const std::string ICON_CHA  = "\U000F01A5";
const std::string ICON_LCK  = "\U000F01CE";

/*
Right panel reserved rows
48 entries — the panel's entire usable height, top to bottom (row 0 is
the window's own top wall and never drawn into; see ui_panels.h's class
comment). Nothing else builds this panel's content, so every row the
panel will ever show is exactly one entry here.
*/
const std::vector<std::string> RIGHT_PANEL_LINES = {
    "", // row 1
    "", // row 2
    "", // row 3
    "", // row 4
    "", // row 5
    "", // row 6
    "", // row 7
    "", // row 8
    "", // row 9
    "", // row 10
    "", // row 11
    "", // row 12
    "", // row 13
    "", // row 14
    "", // row 15
    "", // row 16
    "", // row 17
    "", // row 18
    "", // row 19
    "", // row 20
    "", // row 21
    "", // row 22
    "", // row 23
    "", // row 24
    "", // row 25
    "", // row 26
    "", // row 27
    "", // row 28
    "", // row 29
    "", // row 30
    "", // row 31
    "", // row 32
    "", // row 33
    "", // row 34
    "", // row 35
    "", // row 36
    "", // row 37
    "", // row 38
    "", // row 39
    "", // row 40
    "", // row 41
    "", // row 42
    "", // row 43
    "", // row 44
    "", // row 45
    "", // row 46
    "", // row 47
    "", // row 48
};

} // namespace GamePanel

// =======================================================================================================
// EditorMode
// =======================================================================================================

namespace EditorPanel {

const std::string WORLD_LEGEND = "X \u2503 Y \u2503 Z";

const std::string STATUS_EMPTY_CANVAS  = "No saved map at this location - empty canvas";
const std::string STATUS_LOADED_PREFIX = "Loaded \"";
const std::string STATUS_LOADED_SUFFIX = "\"";
const std::string STATUS_SAVED_PREFIX  = " Saved \"";
const std::string STATUS_SAVED_MID     = "\" to ";
const std::string STATUS_SAVE_FAILED   = "Save FAILED - check world/ is writable";

const std::string STATUS_PLACING_COLLISION   = "Placing: Collision marker";
const std::string STATUS_PLACING_STAIRS_DOWN = "Placing: Stairs DOWN marker";
const std::string STATUS_PLACING_STAIRS_UP   = "Placing: Stairs UP marker";
const std::string STATUS_PLACING_OCCLUSION   = "Placing: Occlusion marker";
const std::string STATUS_PLACING_LIGHT       = "Placing: Light marker";
const std::string STATUS_PLACING_SPAWN       = "Placing: Spawn marker";

} // namespace EditorPanel

// =======================================================================================================
// FragmentEditorMode
// =======================================================================================================

namespace FragmentEditorPanel {

const std::string FRAGMENT_LEGEND = "ID:";

const std::string STATUS_EMPTY_CANVAS  = "No saved fragment at this id - empty canvas";
const std::string STATUS_LOADED_PREFIX = "Loaded \"";
const std::string STATUS_LOADED_SUFFIX = "\"";
const std::string STATUS_SAVED_PREFIX  = " Saved \"";
const std::string STATUS_SAVED_MID     = "\" to ";
const std::string STATUS_SAVE_FAILED   = "Save FAILED - check fragments/ is writable";

const std::string STATUS_PLACING_COLLISION   = "Placing: Collision marker";
const std::string STATUS_PLACING_STAIRS_DOWN = "Placing: Stairs DOWN marker";
const std::string STATUS_PLACING_STAIRS_UP   = "Placing: Stairs UP marker";
const std::string STATUS_PLACING_OCCLUSION   = "Placing: Occlusion marker";
const std::string STATUS_PLACING_LIGHT       = "Placing: Light marker";
const std::string STATUS_PLACING_CONNECTOR   = "Placing: Connector marker";
const std::string STATUS_PLACING_SPAWN       = "Placing: Spawn marker";

} // namespace FragmentEditorPanel

// =======================================================================================================
// SettingsMode
// =======================================================================================================

namespace SettingsPanel {

const std::string TITLE = "SETTINGS MENU";

const std::string HEADER_SETTING = "SETTING";
const std::string HEADER_OPTION  = "OPTION";

const std::string VOLUME_PLACEHOLDER = "(coming soon)";
const std::string VOLUME_STATUS      = "Sound volume isn't wired up yet";

} // namespace SettingsPanel

// =======================================================================================================
// HelpMode
// =======================================================================================================

namespace HelpPanel {

    const std::string TITLE = "HELP MENU";

    const std::vector<HelpTreeGroup> CATEGORY_TREE = {
        {
            "CONTROLS",
            {
                { CATEGORY_GAME,             "Game" },
                { CATEGORY_EDITOR_WORLD,     "Editor world" },
                { CATEGORY_EDITOR_GENERATOR, "Editor generator" },
                { CATEGORY_GENERATOR,        "Generator" },
                { CATEGORY_SETTINGS,         "Settings" },
                { CATEGORY_CONSOLE,          "Console" },
            },
        },
        {
            "CONSOLE COMMANDS",
            {
                { CATEGORY_CONSOLE_COMMANDS, "Possible commands" },
            },
        },
    };

    std::string headerForCategory(int category) {
        for (const HelpTreeGroup& group : CATEGORY_TREE) {
            for (const HelpTreeLeaf& leaf : group.leaves) {
                if (leaf.id == category)
                    return group.name;
            }
        }

        return "";
    }

    const std::vector<HelpControl> GAME_CONTROLS = {
        { "W A S D / ARROWS", "Move" },
        { "E",                "Interact" },
        { "SPACE",            "Attack" },
        { "+",                "Zoom In" },
        { "-",                "Zoom Out" },
        { "ESC",              "Toggle Settings" },
        { "Q",                "Quit" },
        { "H",                "Help" },
    };

    const std::vector<HelpControl> EDITOR_WORLD_CONTROLS = {
        { "LMB",              "Place" },
        { "RMB",              "Erase" },
        { "W A S D / ARROWS", "Select Coordinate" },
        { "PGUP / PGDN",      "Select Floor" },
        { "1",                "Select Collision Marker" },
        { "2",                "Select Stairs Down Marker" },
        { "3",                "Select Stairs Up Marker" },
        { "4",                "Select Occlusion Marker" },
        { "5",                "Select Light Marker" },
        { "6",                "Select Spawn Marker" },
        { "F5",               "Save" },
        { "F9",               "Load" },
        { "ESC",              "Toggle Settings" },
        { "Q",                "Quit" },
        { "H",                "Help" },
    };

    const std::vector<HelpControl> EDITOR_GENERATOR_CONTROLS = {
        { "LMB",              "Place" },
        { "RMB",              "Erase" },
        { "W A S D / ARROWS", "Resize" },
        { "PGUP / PGDN",      "Select Fragment" },
        { "1",                "Select Collision Marker" },
        { "2",                "Select Stairs Down Marker" },
        { "3",                "Select Stairs Up Marker" },
        { "4",                "Select Occlusion Marker" },
        { "5",                "Select Light Marker" },
        { "6",                "Select Spawn Marker" },
        { "0",                "Select Connector Marker" },
        { "F5",               "Save" },
        { "F9",               "Load" },
        { "ESC",              "Toggle Settings" },
        { "Q",                "Quit" },
        { "H",                "Help" },
    };

    /*
    GeneratorMode really is R/Q/H-only by design (see its own class
    comment in generator/generator_mode.h) — no ESC/Settings binding, so
    that entry doesn't belong here the way it does for every other mode.
    */
    const std::vector<HelpControl> GENERATOR_CONTROLS = {
        { "R",                "Regenerate" },
        { "Q",                "Quit" },
        { "H",                "Help" },
    };

    const std::vector<HelpControl> SETTINGS_CONTROLS = {
        { "W / S",            "Navigate" },
        { "A / D",            "Change Column" },
        { "SPACE",            "Select" },
        { "ESC",              "Toggle Settings" },
        { "Q",                "Quit" },
        { "H",                "Help" },
    };

    /*
    Console::onEvent (core/console.cpp) consumes every SDL_KEYDOWN
    unconditionally while open — it's a free-text input field, not a mode
    with shortcut keys — so Q/H do NOT quit/open Help while typing, they
    just type "q"/"h"; ESC closes the console itself, not Settings. Those
    three don't belong in this list the way they do for every other mode.
    */
    const std::vector<HelpControl> CONSOLE_CONTROLS = {
        { "~",                "Toggle Console" },
        { "ENTER",            "Submit" },
        { "BACKSPACE",        "Delete Character" },
        { "UP / DOWN",        "Scroll History" },
        { "TAB",              "Autocomplete" },
        { "ESC",              "Close Console" },
    };

    const std::vector<std::string> CONSOLE_COMMANDS = {
        "/mode game",
        "/mode editor world",
        "/mode editor generator",
        "/mode generator",
        "/mode settings",
        "/mode help",
        "/zoom <1-4>",
        "/load <floor>-<x>-<y>",
        "/borderMap",
        "/lightMap",
        "/debugGrid",
        "/exit",
    };

    std::vector<HelpControl> controlsForCategory(int category) {
        switch (category) {
            case CATEGORY_GAME:
                return GAME_CONTROLS;

            case CATEGORY_EDITOR_WORLD:
                return EDITOR_WORLD_CONTROLS;

            case CATEGORY_EDITOR_GENERATOR:
                return EDITOR_GENERATOR_CONTROLS;

            case CATEGORY_GENERATOR:
                return GENERATOR_CONTROLS;

            case CATEGORY_SETTINGS:
                return SETTINGS_CONTROLS;

            case CATEGORY_CONSOLE:
                return CONSOLE_CONTROLS;

            default:
                return {};
        }
    }

} // namespace HelpPanel
