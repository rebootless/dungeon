#include "game_panel.h"

namespace GamePanel {

/*
Reused by HelpMode's Game category (see help/help_panel.cpp) rather than
duplicated there — no longer drawn in this info box itself (see
buildInfoBoxLines below).
*/
const std::string CONTROLS_MOVE =
    "Move: [W/A/S/D] / [\u2190\u2191\u2193\u2192] \u2503 Interact: [E] \u2503 Attack: [SPACE]";

const std::string CONTROLS_MISC =
    "Zoom In: [+] \u2503 Zoom Out: [-] \u2503 Settings: [ESC] \u2503 Quit: [Q]";

const std::string ZOOM_LABEL_PREFIX = "Zoom: ";
const std::string ZOOM_LABEL_SUFFIX = "x";

const std::string LOCATION_LABEL_PREFIX = "Location: ";

// Only the interaction message remains — the control legend that used to
// fill rows 8/9 lives in HelpMode now (reachable with [H]).
std::vector<std::string> buildInfoBoxLines(const std::string& message) {
    return { message };
}

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

const std::vector<std::string> RIGHT_PANEL_LINES = {
    "",
};

} // namespace GamePanel
