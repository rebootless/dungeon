#pragma once

#include <string>
#include <vector>

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
