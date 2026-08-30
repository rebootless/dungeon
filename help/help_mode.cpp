#include "help_mode.h"

#include <algorithm>

#include "../core/display.h"
#include "../core/layout.h"
#include "../core/renderer.h"
#include "../editor/editor_mode.h"
#include "../game/game_mode.h"
#include "../generator/fragment_editor_mode.h"
#include "../generator/generator_mode.h"
#include "../settings/settings_mode.h"
#include "help_panel.h"

namespace {

/*
Only the left column (categories) ever has keyboard focus — unlike
SettingsMode's two columns, the right column here is read-only text, not
a list of values to pick from, so there's no focusedColumn_ to track.

Holds a LEAF id (HelpPanel::CATEGORY_*), never a group — group rows
(CONTROLS, CONSOLE COMMANDS) are headers only, never the current
selection. Up/Down in onEvent() just increments/decrements this across
the full contiguous [0, LEAF_COUNT) range, which walks every leaf across
both groups in display order without needing to know where one group
ends and the next begins.
*/
int categoryIndex_ = HelpPanel::CATEGORY_GAME;

/*
See settings_mode.cpp's returnMode_ comment — file-scope for the same
reason: a fresh HelpMode instance is constructed every time App::
switchMode() opens this screen, which would otherwise reset it right
back to the default before onEvent() ever read it.
*/
HelpMode::ReturnMode returnMode_ = HelpMode::ReturnMode::Game;

int clampIndex(int index, int count) {
    if (count <= 0) return 0;
    return std::max(0, std::min(index, count - 1));
}

/*
Builds the right-column legend for the selected leaf.

CATEGORY_CONSOLE_COMMANDS pulls from CONSOLE_COMMANDS instead of
controlsForCategory(), since commands are plain strings rather than
key/description pairs — each is wrapped into a HelpControl with an
empty description so the renderer can treat both cases uniformly.
*/
std::vector<HelpPanel::HelpControl> controlsForSelection(int category) {
    if (category == HelpPanel::CATEGORY_CONSOLE_COMMANDS) {
        std::vector<HelpPanel::HelpControl> controls;
        for (const std::string& command : HelpPanel::CONSOLE_COMMANDS)
            controls.push_back({ command, "" });
        return controls;
    }

    return HelpPanel::controlsForCategory(category);
}

/*
Widest key across every category, computed once. The description column
sits a fixed distance past this width regardless of which category is
selected, so switching categories never shifts the description column —
every table lines up the same way.
*/
int keyColumnWidth() {
    static const int width = [] {
        int w = 0;
        auto scan = [&](const std::vector<HelpPanel::HelpControl>& controls) {
            for (const HelpPanel::HelpControl& control : controls)
                w = std::max(w, (int)control.key.size());
        };
        scan(HelpPanel::GAME_CONTROLS);
        scan(HelpPanel::EDITOR_WORLD_CONTROLS);
        scan(HelpPanel::EDITOR_GENERATOR_CONTROLS);
        scan(HelpPanel::GENERATOR_CONTROLS);
        scan(HelpPanel::SETTINGS_CONTROLS);
        scan(HelpPanel::CONSOLE_CONTROLS);
        return w;
    }();
    return width;
}

} // namespace

void HelpMode::setReturnMode(ReturnMode mode) {
    returnMode_ = mode;
}

void HelpMode::onEnter() {
    categoryIndex_ = HelpPanel::CATEGORY_GAME;
}

void HelpMode::onEvent(const SDL_Event& e) {
    if (e.type != SDL_KEYDOWN) return;

    switch (e.key.keysym.scancode) {
        /*
        [H] toggles: it's what opened HelpMode from every other mode (see
        e.g. GameMode's SDL_SCANCODE_H handler), so pressing it again in
        here closes HelpMode the same way ESCAPE does — same idea as
        SettingsMode, where ESCAPE both opens (well, [ESC] specifically
        for settings) and closes.
        */
        case SDL_SCANCODE_H:
        case SDL_SCANCODE_ESCAPE:
            switch (returnMode_) {
                case ReturnMode::EditorWorld:    context_.switchMode(std::make_unique<EditorMode>());         return;
                case ReturnMode::FragmentEditor: context_.switchMode(std::make_unique<FragmentEditorMode>()); return;
                case ReturnMode::Generator:      context_.switchMode(std::make_unique<GeneratorMode>());      return;
                case ReturnMode::Settings:       context_.switchMode(std::make_unique<SettingsMode>());       return;
                case ReturnMode::Game:           context_.switchMode(std::make_unique<GameMode>());           return;
            }
            return;

        case SDL_SCANCODE_Q:
            context_.requestQuit();
            return;

        case SDL_SCANCODE_W:
        case SDL_SCANCODE_UP:
            categoryIndex_ = clampIndex(categoryIndex_ - 1, HelpPanel::LEAF_COUNT);
            return;

        case SDL_SCANCODE_S:
        case SDL_SCANCODE_DOWN:
            categoryIndex_ = clampIndex(categoryIndex_ + 1, HelpPanel::LEAF_COUNT);
            return;

        default:
            return;
    }
}

void HelpMode::onRender() {
    int w = 0, h = 0;
    getCanvasSize(w, h);

    // Frame — single full-window border, same as SettingsMode's.
    {
        FrameBuilder fb;
        fb.markRow(0, 0, w);
        fb.markRow(h - CELL_SIZE, 0, w);
        fb.markCol(0, 0, h);
        fb.markCol(w - CELL_SIZE, 0, h);
        fb.draw();
    }

    drawStringPx(HelpPanel::TITLE, HelpPanel::COL_LEFT * CELL_SIZE, HelpPanel::ROW_TITLE * CELL_SIZE);

    // Right header tracks whichever group owns the selected leaf — reads
    // "CONTROLS" browsing a mode's legend, "CONSOLE COMMANDS" browsing
    // the command list.
    const SDL_Color kFocused = SDL_Color{255, 255, 0, 255};
    const SDL_Color kPlain   = SDL_Color{200, 200, 200, 255};
    const SDL_Color kWhite   = SDL_Color{255, 255, 255, 255};

    drawStringPx(HelpPanel::headerForCategory(categoryIndex_), HelpPanel::COL_RIGHT * CELL_SIZE,
                 HelpPanel::ROW_HEADERS * CELL_SIZE, kWhite);

    // Left column: group header rows (not selectable) followed by their
    // leaves (indented, selectable) — see HelpPanel::CATEGORY_TREE. The
    // selection prefix matches SettingsMode's " > " / "   " convention.
    int row = 0;
    for (const HelpPanel::HelpTreeGroup& group : HelpPanel::CATEGORY_TREE) {
        drawStringPx(group.name, HelpPanel::COL_LEFT * CELL_SIZE,
                     (HelpPanel::ROW_LIST_START + row) * CELL_SIZE, kWhite);
        ++row;

        for (const HelpPanel::HelpTreeLeaf& leaf : group.leaves) {
            bool isSelected = (leaf.id == categoryIndex_);
            std::string prefix = isSelected ? " > " : "   ";
            drawStringPx(prefix + leaf.name, HelpPanel::COL_LEFT * CELL_SIZE,
                         (HelpPanel::ROW_LIST_START + row) * CELL_SIZE,
                         isSelected ? kFocused : kPlain);
            ++row;
        }
    }

    // Right column — key/description legend for the currently selected
    // leaf. The description column sits at a fixed offset shared by every
    // category (see keyColumnWidth()), so the table looks the same no
    // matter which category is selected.
    std::vector<HelpPanel::HelpControl> controls = controlsForSelection(categoryIndex_);
    int descX = (HelpPanel::COL_RIGHT + keyColumnWidth() + 2) * CELL_SIZE;

    for (size_t i = 0; i < controls.size(); ++i) {
        int rowY = (HelpPanel::ROW_CONTROLS_START + (int)i) * CELL_SIZE;
        drawStringPx(controls[i].key, HelpPanel::COL_RIGHT * CELL_SIZE, rowY, kPlain);

        if (!controls[i].description.empty())
            drawStringPx(controls[i].description, descX, rowY, kPlain);
    }
}
