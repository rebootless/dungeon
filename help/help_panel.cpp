#include "help_panel.h"

namespace HelpPanel {

const std::string TITLE           = "HELP MENU";
const std::string HEADER_CATEGORY = "MODE";
const std::string HEADER_CONTROLS = "CONTROLS";

const std::vector<std::string> CATEGORY_NAMES = {
    "Game",
    "Editor world",
    "Editor generator",
    "Generator",
    "Settings",
    "Console",
};

// Game
const std::string GAME_MOVE =
    "Move: [W/A/S/D] / [\u2190\u2191\u2193\u2192] \u2503 Interact: [E] \u2503 Attack: [SPACE]";

const std::string GAME_MISC =
    "Zoom In: [+] \u2503 Zoom Out: [-] \u2503 Settings: [ESC] \u2503 Quit: [Q]";

// Editor world
const std::string EDITOR_WORLD_PLACE =
    "Place: [LMB] \u2503 Erase: [RMB] \u2503 Zoom: [Wheel]";

const std::string EDITOR_WORLD_COLLISION =
    "Collision: [1] \u2503 Stairs Down: [2] \u2503 Stairs Up: [3] \u2503 Occlusion: [4]";

const std::string EDITOR_WORLD_LOCATION =
    "Move: [W/A/S/D] / [\u2190\u2191\u2193\u2192] \u2503 Floor: [PgUp/PgDn]";

const std::string EDITOR_WORLD_SAVE =
    "Save: [F5] \u2503 Load: [F9]";

const std::string EDITOR_WORLD_QUIT =
    "Settings: [ESC] \u2503 Quit: [Q]";

// Editor generator (fragment editor)
const std::string EDITOR_GENERATOR_PLACE =
    "Place: [LMB] \u2503 Erase: [RMB] \u2503 Zoom: [Wheel]";

const std::string EDITOR_GENERATOR_COLLISION =
    "Collision: [1] \u2503 Stairs Down: [2] \u2503 Stairs Up: [3] \u2503 Occlusion: [4] \u2503 Connector: [5]";

const std::string EDITOR_GENERATOR_SIZE =
    "Size: [\u2190\u2191\u2193\u2192] \u2503 Fragment: [PgUp/PgDn]";

const std::string EDITOR_GENERATOR_SAVE =
    "Save: [F5] \u2503 Load: [F9]";

const std::string EDITOR_GENERATOR_QUIT =
    "Settings: [ESC] \u2503 Quit: [Q]";

// Generator (test mode)
const std::string GENERATOR_REGENERATE = "Regenerate: [R]";
const std::string GENERATOR_QUIT       = "Quit: [Q]";

// Settings
const std::string SETTINGS_HINT =
    "Navigate: [W/S] / [\u2191\u2193] \u2503 Column: [A/D] / [\u2190\u2192] \u2503 Select: [SPACE] \u2503 Close: [ESC] \u2503 Quit: [Q]";

// Console
const std::string CONSOLE_TOGGLE       = "Open/Close: [`]";
const std::string CONSOLE_SUBMIT       = "Submit: [ENTER] \u2503 Close: [ESC]";
const std::string CONSOLE_HISTORY      = "History: [\u2191\u2193]";
const std::string CONSOLE_AUTOCOMPLETE = "Autocomplete: [TAB]";

/*
Possible commands
Kept in sync by hand with core/app.cpp's registerConsoleCommands()/
dispatchCommand() — nothing enforces it automatically, so a new command
added there needs a matching line added here.
*/
const std::vector<std::string> CONSOLE_COMMANDS = {
    "Possible commands:",
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

const std::string HELP_KEY = "Help: [H]";

std::vector<std::string> controlsForCategory(int category) {
    switch (category) {
        case CATEGORY_GAME:
            return { GAME_MOVE, GAME_MISC, HELP_KEY };

        case CATEGORY_EDITOR_WORLD:
            return { EDITOR_WORLD_PLACE, EDITOR_WORLD_COLLISION,
                     EDITOR_WORLD_LOCATION, EDITOR_WORLD_SAVE,
                     EDITOR_WORLD_QUIT, HELP_KEY };

        case CATEGORY_EDITOR_GENERATOR:
            return { EDITOR_GENERATOR_PLACE, EDITOR_GENERATOR_COLLISION,
                     EDITOR_GENERATOR_SIZE, EDITOR_GENERATOR_SAVE,
                     EDITOR_GENERATOR_QUIT, HELP_KEY };

        case CATEGORY_GENERATOR:
            return { GENERATOR_REGENERATE, GENERATOR_QUIT, HELP_KEY };

        case CATEGORY_SETTINGS:
            return { SETTINGS_HINT, HELP_KEY };

        case CATEGORY_CONSOLE: {
            std::vector<std::string> lines = {
                CONSOLE_TOGGLE, CONSOLE_SUBMIT, CONSOLE_HISTORY, CONSOLE_AUTOCOMPLETE, "",
            };
            lines.insert(lines.end(), CONSOLE_COMMANDS.begin(), CONSOLE_COMMANDS.end());
            lines.push_back(HELP_KEY);
            return lines;
        }

        default:
            return {};
    }
}

} // namespace HelpPanel
