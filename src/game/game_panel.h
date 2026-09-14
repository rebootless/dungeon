#pragma once

#include <string>
#include <vector>

/*
GameMode info panel layout
Text drawn into the info box below the map (see GameMode::render() in
game_mode.cpp) and into the two side panels. All three follow the same
shape: a std::vector<std::string>, one row per entry, top to bottom,
drawn by game_mode.cpp's drawPanelLines(). The info box is capped at 12
rows (row 11 as bottom margin — 12 is the hard ceiling, see
core/layout.h's UI_BOX_ROWS); the side panels run the full canvas height,
48 usable rows (row 0 is the window's own top wall, row 49 its bottom
wall — see core/layout.h's CANVAS_H/CELL_SIZE).

Every hardcoded label, placeholder value, and blank reserved row for all
three panels lives in this file — game_mode.cpp only ever reaches into
GamePanel by name, never spells out panel text of its own. Any *_EXTRA_LINES
block below is exactly as many rows as still fit after the content built
above it, ready to hardcode text into or hand a variable to one row at a
time; "" draws as a blank line.
*/
namespace GamePanel {

    /*
    Builds the info box's rows fresh, every call — message goes into row
    0, INFO_BOX_EXTRA_LINES fills rows 1-10, and row 11 stays blank as
    the box's own bottom margin (see this file's class comment above).
    */
    std::vector<std::string> buildInfoBoxLines(const std::string& message);

    // Reserved rows for the info box, drawn directly below the live
    // message line. 10 entries — everything the 12-row cap leaves after
    // the message (row 0) and the fixed bottom-margin row (row 11).
    extern const std::vector<std::string> INFO_BOX_EXTRA_LINES;

    /*
    Left panel: location + stat block
    location gets LOCATION_LABEL_PREFIX prepended automatically. statLines
    is however many pre-built "icon ABBR - value" lines the caller wants —
    see buildStatLine() and the ICON_ / STAT_VALUE_ constants below —
    placed directly after location, one per row. zoomLabel follows the
    stat block; LEFT_PANEL_EXTRA_LINES fills every row still left after
    that up to the panel's 48-row ceiling.
    */
    extern const std::string LOCATION_LABEL_PREFIX; // " Location: "
    std::vector<std::string> buildLeftPanelLines(const std::string& location,
                                                  const std::vector<std::string>& statLines,
                                                  const std::string& zoomLabel);

    // Reserved rows for the left panel, drawn after location/stats/zoom.
    // 35 entries — everything the panel's 48-row ceiling leaves after
    // location (1) + a blank (1) + the 9 stat lines + a blank (1) +
    // zoom (1) = 13 rows already spoken for.
    extern const std::vector<std::string> LEFT_PANEL_EXTRA_LINES;

    /*
    One "icon ABBR - value" stat row, e.g. buildStatLine(ICON_HP, "HP",
    STAT_VALUE_HP) -> " \uf004 HP - 100/100". Icons are plain Unicode
    characters, chosen so the row renders correctly with any font.
    */
    std::string buildStatLine(const std::string& icon, const std::string& abbr, const std::string& value);

    extern const std::string ICON_HP;  // Health
    extern const std::string ICON_STA; // Stamina — rolls, sprinting, heavy attacks
    extern const std::string ICON_MP;  // Mana — spells
    extern const std::string ICON_ATK; // Attack — physical damage
    extern const std::string ICON_DEF; // Defense — damage reduction
    extern const std::string ICON_DEX; // Dexterity — attack speed, bows, dodge chance
    extern const std::string ICON_INT; // Intelligence — magic damage
    extern const std::string ICON_CHA; // Charisma
    extern const std::string ICON_LCK; // Luck — loot, crits, rare events

    // Stat block values, in the same HP/STA/MP/ATK/DEF/DEX/INT/CHA/LCK
    // order game_mode.cpp's render() builds statLines in. Hardcode a
    // real value here, or hand it a variable once actual character stats
    // exist — game_mode.cpp only ever reads these by name.
    extern const std::string STAT_VALUE_HP;
    extern const std::string STAT_VALUE_STA;
    extern const std::string STAT_VALUE_MP;
    extern const std::string STAT_VALUE_ATK;
    extern const std::string STAT_VALUE_DEF;
    extern const std::string STAT_VALUE_DEX;
    extern const std::string STAT_VALUE_INT;
    extern const std::string STAT_VALUE_CHA;
    extern const std::string STAT_VALUE_LCK;

    // Right panel — reserved rows, one entry per row, filling the
    // panel's full 48-row height. "" draws as a blank line.
    extern const std::vector<std::string> RIGHT_PANEL_LINES;

    // Static label text
    extern const std::string ZOOM_LABEL_PREFIX; // " Zoom: " — sandwiches getZoom()
    extern const std::string ZOOM_LABEL_SUFFIX; // "x"

} // namespace GamePanel
