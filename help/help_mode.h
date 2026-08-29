#pragma once

#include "../core/mode.h"

/*
HelpMode
A single reference screen listing every mode's controls, laid out the
same way as SettingsMode: a left column of categories (one per mode) and
a right column showing that category's full control legend. Reachable
with [H] from every other mode (Game, EditorMode, FragmentEditorMode,
GeneratorMode, SettingsMode) — see each of their onEvent()s.

Like SettingsMode, ESC needs to know which mode to return to; see
setReturnMode()'s comment on why that's file-scope state in
help_mode.cpp rather than a constructor argument.
*/
class HelpMode : public IMode {
public:
    enum class ReturnMode { Game, EditorWorld, FragmentEditor, Generator, Settings };

    // Sets which mode ESC returns to. Called right before switching TO
    // HelpMode — see e.g. game_controls.cpp's SDL_SCANCODE_H handling.
    static void setReturnMode(ReturnMode mode);

    void onEnter() override;
    void onEvent(const SDL_Event& e) override;
    void onRender() override;
};
