#pragma once

#include <string>

/*
FragmentEditorMode panel layout
Same idea as EditorPanel (editor/editor_panel.h) — row indices for the
text drawn into the info box below the map. Row 0 is the panel's own top
row; UI_BOX_ROWS (core/layout.h) is 13, so row indices here must stay
within [0, 11].
*/
namespace FragmentEditorPanel {

    // Info box rows
    constexpr int ROW_STATUS          = 2; // transient F5/F9 save-load status
    constexpr int ROW_HELP_PLACE      = 4;
    constexpr int ROW_HELP_COLLISION  = 5;
    constexpr int ROW_HELP_SIZE       = 6;
    constexpr int ROW_HELP_SAVE       = 7;
    constexpr int ROW_HELP_QUIT       = 8;

    // Right panel (fragment list) rows
    constexpr int ROW_FRAGMENT_HEADER = 1; // " Fragment N (WxH) [new]"
    constexpr int ROW_FRAGMENT_LEGEND = 2; // PageUp/PageDown legend
    constexpr int FRAGMENT_LIST_START_ROW = 2;

    // Static label text
    extern const std::string FRAGMENT_LEGEND;
    extern const std::string HELP_PLACE;
    extern const std::string HELP_COLLISION;
    extern const std::string HELP_SIZE;
    extern const std::string HELP_SAVE;
    extern const std::string HELP_QUIT;

} // namespace FragmentEditorPanel
