#pragma once

#include <string>
#include <vector>

/*
HelpMode panel layout
Same layout constants as SettingsPanel (settings/settings_panel.h) —
categories in a left column, that category's control legend in a right
column, no fixed-size map to work around.
*/
namespace HelpPanel {

    constexpr int ROW_TITLE      = 1;
    constexpr int ROW_HEADERS    = 3;
    constexpr int ROW_LIST_START = 4;

    constexpr int COL_LEFT  = 2;  // category column
    constexpr int COL_RIGHT = 20; // that category's controls column

    extern const std::string TITLE;
    extern const std::string HEADER_CATEGORY;
    extern const std::string HEADER_CONTROLS;

    // One entry per mode, in the order shown in the left column.
    extern const std::vector<std::string> CATEGORY_NAMES;
    constexpr int CATEGORY_GAME             = 0;
    constexpr int CATEGORY_EDITOR_WORLD     = 1;
    constexpr int CATEGORY_EDITOR_GENERATOR = 2;
    constexpr int CATEGORY_GENERATOR        = 3;
    constexpr int CATEGORY_SETTINGS         = 4;
    constexpr int CATEGORY_CONSOLE          = 5;

    /*
    Control text
    Every string a mode's controls legend is built from, all in this one
    file so editing any mode's wording never means hunting through that
    mode's own panel header — GameMode/EditorMode/FragmentEditorMode/
    SettingsMode/the console no longer hold any control-text constants of
    their own (see each's *_panel.cpp) — this is the only copy, and
    controlsForCategory() below is the only reader.
    */

    // Game
    extern const std::string GAME_MOVE;
    extern const std::string GAME_MISC;

    // Editor world
    extern const std::string EDITOR_WORLD_PLACE;
    extern const std::string EDITOR_WORLD_COLLISION;
    extern const std::string EDITOR_WORLD_LOCATION;
    extern const std::string EDITOR_WORLD_SAVE;
    extern const std::string EDITOR_WORLD_QUIT;

    // Editor generator (fragment editor)
    extern const std::string EDITOR_GENERATOR_PLACE;
    extern const std::string EDITOR_GENERATOR_COLLISION;
    extern const std::string EDITOR_GENERATOR_SIZE;
    extern const std::string EDITOR_GENERATOR_SAVE;
    extern const std::string EDITOR_GENERATOR_QUIT;

    // Generator (test mode) — never drew its own inline legend even
    // before HelpMode existed (see generator/generator_mode.cpp), so
    // this wording has never lived anywhere else.
    extern const std::string GENERATOR_REGENERATE;
    extern const std::string GENERATOR_QUIT;

    // Settings
    extern const std::string SETTINGS_HINT;

    /*
    Console — how to use the backtick prompt itself, then every command
    core/app.cpp's dispatchCommand() understands. Keep this list in sync
    with registerConsoleCommands()'s command tree by hand; nothing
    enforces it automatically.
    */
    extern const std::string CONSOLE_TOGGLE;
    extern const std::string CONSOLE_SUBMIT;
    extern const std::string CONSOLE_HISTORY;
    extern const std::string CONSOLE_AUTOCOMPLETE;
    extern const std::vector<std::string> CONSOLE_COMMANDS;

    // Every category's legend ends with this — [H] works from every mode
    // HelpMode can be reached from, including HelpMode's own return targets.
    extern const std::string HELP_KEY;

    // The full control legend for one category, one line per entry.
    std::vector<std::string> controlsForCategory(int category);

} // namespace HelpPanel
