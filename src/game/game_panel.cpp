#include "game_panel.h"

namespace GamePanel {

const std::string ZOOM_LABEL_PREFIX = "Zoom: ";
const std::string ZOOM_LABEL_SUFFIX = "x";

const std::string LOCATION_LABEL_PREFIX = "Location: ";

// Stat block values — see game_panel.h's comment on this block. HP/MP
// carry a trailing space so every value lines up to the same 5-character
// width as the others ("[sta]", "[atk]", ...).
const std::string STAT_VALUE_HP  = "[hp] ";
const std::string STAT_VALUE_STA = "[sta]";
const std::string STAT_VALUE_MP  = "[mp] ";
const std::string STAT_VALUE_ATK = "[atk]";
const std::string STAT_VALUE_DEF = "[def]";
const std::string STAT_VALUE_DEX = "[dex]";
const std::string STAT_VALUE_INT = "[int]";
const std::string STAT_VALUE_CHA = "[cha]";
const std::string STAT_VALUE_LCK = "[lck]";

/*
Info box reserved rows
10 entries, rows 1-10 of the box (row 0 is the live message, row 11 is
the fixed bottom margin — see game_panel.h's class comment and
buildInfoBoxLines below). Hardcode text directly into any entry, or
replace one with a variable at the buildInfoBoxLines call site.
*/
const std::vector<std::string> INFO_BOX_EXTRA_LINES = {
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
see game_panel.h's class comment and buildLeftPanelLines below).
*/
const std::vector<std::string> LEFT_PANEL_EXTRA_LINES = {
    "", "", "", "", "", "", "", "", "", "", // rows 13-22
    "", "", "", "", "", "", "", "", "", "", // rows 23-32
    "", "", "", "", "", "", "", "", "", "", // rows 33-42
    "", "", "", "", "",                     // rows 43-47
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
the window's own top wall and never drawn into; see game_panel.h's class
comment). Nothing else builds this panel's content, so every row the
panel will ever show is exactly one entry here.
*/
const std::vector<std::string> RIGHT_PANEL_LINES = {
    "", "", "", "", "", "", "", "", "", "", // rows 1-10
    "", "", "", "", "", "", "", "", "", "", // rows 11-20
    "", "", "", "", "", "", "", "", "", "", // rows 21-30
    "", "", "", "", "", "", "", "", "", "", // rows 31-40
    "", "", "", "", "", "", "", "",         // rows 41-48
};

} // namespace GamePanel
