#include "help_panel.h"

namespace HelpPanel {

    const std::string TITLE           = "HELP MENU";

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
        { "ESC",              "Settings" },
        { "Q",                "Quit" },
        { "H",                "Help" },
    };

    const std::vector<HelpControl> EDITOR_WORLD_CONTROLS = {
        { "LMB",              "Place" },
        { "RMB",              "Erase" },
        { "W A S D / ARROWS", "Move" },
        { "PGUP / PGDN",      "Change Floor" },
        { "1",                "Collision" },
        { "2",                "Stairs Down" },
        { "3",                "Stairs Up" },
        { "4",                "Occlusion" },
        { "F5",               "Save" },
        { "F9",               "Load" },
        { "ESC",              "Settings" },
        { "Q",                "Quit" },
        { "H",                "Help" },
    };

    const std::vector<HelpControl> EDITOR_GENERATOR_CONTROLS = {
        { "LMB",              "Place" },
        { "RMB",              "Erase" },
        { "W A S D / ARROWS", "Resize" },
        { "PGUP / PGDN",      "Change Fragment" },
        { "1",                "Collision" },
        { "2",                "Stairs Down" },
        { "3",                "Stairs Up" },
        { "4",                "Occlusion" },
        { "5",                "Connector" },
        { "F5",               "Save" },
        { "F9",               "Load" },
        { "ESC",              "Settings" },
        { "Q",                "Quit" },
        { "H",                "Help" },
    };

    const std::vector<HelpControl> GENERATOR_CONTROLS = {
        { "R", "Regenerate" },
        { "Q", "Quit" },
        { "H", "Help" },
    };

    const std::vector<HelpControl> SETTINGS_CONTROLS = {
        { "W / S", "Navigate" },
        { "A / D", "Change Column" },
        { "SPACE", "Select" },
        { "ESC",   "Close" },
        { "Q",     "Quit" },
    };

    const std::vector<HelpControl> CONSOLE_CONTROLS = {
        { "~",        "Open / Close" },
        { "ENTER",    "Submit" },
        { "ESC",      "Close" },
        { "UP / DOWN", "History" },
        { "TAB",      "Autocomplete" },
        { "H",        "Help" },
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
