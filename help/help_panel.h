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
    constexpr int CATEGORY_GAME            = 0;
    constexpr int CATEGORY_EDITOR_WORLD     = 1;
    constexpr int CATEGORY_EDITOR_GENERATOR = 2;
    constexpr int CATEGORY_GENERATOR        = 3;
    constexpr int CATEGORY_SETTINGS         = 4;

    /*
    The full control legend for one category, one line per entry — mostly
    the same GamePanel/EditorPanel/FragmentEditorPanel::HELP_* strings
    those modes used to draw inline themselves (see each mode's onRender()
    for why they no longer do), so the wording only lives in one place.
    */
    std::vector<std::string> controlsForCategory(int category);

} // namespace HelpPanel
