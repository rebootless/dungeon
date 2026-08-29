#include "help_panel.h"

#include "../editor/editor_panel.h"
#include "../game/game_panel.h"
#include "../generator/fragment_editor_panel.h"
#include "../settings/settings_panel.h"

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
};

// Every category's legend ends with this — [H] works from every mode
// HelpMode can be reached from, including HelpMode's own return targets.
static const std::string HELP_KEY = "Help: [H]";

std::vector<std::string> controlsForCategory(int category) {
    switch (category) {
        case CATEGORY_GAME:
            return { GamePanel::CONTROLS_MOVE, GamePanel::CONTROLS_MISC, HELP_KEY };

        case CATEGORY_EDITOR_WORLD:
            return { EditorPanel::HELP_PLACE, EditorPanel::HELP_COLLISION,
                     EditorPanel::HELP_LOCATION, EditorPanel::HELP_SAVE,
                     EditorPanel::HELP_QUIT, HELP_KEY };

        case CATEGORY_EDITOR_GENERATOR:
            return { FragmentEditorPanel::HELP_PLACE, FragmentEditorPanel::HELP_COLLISION,
                     FragmentEditorPanel::HELP_SIZE, FragmentEditorPanel::HELP_SAVE,
                     FragmentEditorPanel::HELP_QUIT, HELP_KEY };

        /*
        Generator (test mode) never drew its own inline control legend —
        it has no text at all (see generator/generator_mode.cpp) — so
        its wording lives here and only here.
        */
        case CATEGORY_GENERATOR:
            return { "Regenerate: [R]", "Quit: [Q]", HELP_KEY };

        case CATEGORY_SETTINGS:
            return { SettingsPanel::HINT, HELP_KEY };

        default:
            return {};
    }
}

} // namespace HelpPanel
