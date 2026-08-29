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
            categoryIndex_ = clampIndex(categoryIndex_ - 1, (int)HelpPanel::CATEGORY_NAMES.size());
            return;

        case SDL_SCANCODE_S:
        case SDL_SCANCODE_DOWN:
            categoryIndex_ = clampIndex(categoryIndex_ + 1, (int)HelpPanel::CATEGORY_NAMES.size());
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

    drawStringPx(HelpPanel::HEADER_CATEGORY, HelpPanel::COL_LEFT * CELL_SIZE,
                 HelpPanel::ROW_HEADERS * CELL_SIZE);
    drawStringPx(HelpPanel::HEADER_CONTROLS, HelpPanel::COL_RIGHT * CELL_SIZE,
                 HelpPanel::ROW_HEADERS * CELL_SIZE);

    const SDL_Color kFocused = SDL_Color{255, 255, 0, 255};
    const SDL_Color kPlain   = SDL_Color{200, 200, 200, 255};

    // Categories (left column)
    for (size_t i = 0; i < HelpPanel::CATEGORY_NAMES.size(); ++i) {
        bool isSelected = ((int)i == categoryIndex_);
        std::string prefix = isSelected ? " > " : "   ";
        drawStringPx(prefix + HelpPanel::CATEGORY_NAMES[i],
                     HelpPanel::COL_LEFT * CELL_SIZE,
                     (HelpPanel::ROW_LIST_START + (int)i) * CELL_SIZE,
                     isSelected ? kFocused : kPlain);
    }

    // Controls (right column) — whichever category is currently selected
    std::vector<std::string> lines = HelpPanel::controlsForCategory(categoryIndex_);
    for (size_t i = 0; i < lines.size(); ++i) {
        drawStringPx(lines[i], HelpPanel::COL_RIGHT * CELL_SIZE,
                     (HelpPanel::ROW_LIST_START + (int)i) * CELL_SIZE, kPlain);
    }
}
