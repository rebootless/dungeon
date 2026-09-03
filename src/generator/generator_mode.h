#pragma once

#include "../core/mode.h"

/*
GeneratorMode
The [R]/[Q]-only test screen for dungeon_generator.h's stitching
algorithm — no HUD, no side panels, no info box text at all (see
onRender() in generator_mode.cpp); everything about how to use it lives
in HelpMode instead (reachable with [H], see help/help_mode.h).
Reachable via /mode generator (see core/app.cpp), distinct from
/mode editor generator (FragmentEditorMode, generator/fragment_editor_mode.h)
which authors the fragments this mode assembles.
*/
class GeneratorMode : public IMode {
public:
    void onEnter() override;
    void onEvent(const SDL_Event& e) override;
    void onRender() override;
};
