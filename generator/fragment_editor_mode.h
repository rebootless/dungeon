#pragma once

#include "../core/mode.h"

/*
FragmentEditorMode
A second editor, alongside EditorMode (editor/editor_mode.h) — for
hand-authoring the small reusable dungeon pieces (rooms, corridors,
halls, ...) that the procedural generator will later stitch together,
rather than a full world location. Reachable via /mode editor generator
(see core/app.cpp), as opposed to /mode editor world for the regular
world editor.

Shares EditorMode's overall shape (palette + map + right-hand list,
runs inside App's shared SDL_WaitEvent loop, state in extern globals —
see fragment_editor_state.h for why) but arrow keys resize the fragment's
footprint instead of stepping a world coordinate, and painting is not
tied to any (floor, x, y) — see fragment.h's Fragment struct. onEnter()/
onRender() are defined in fragment_editor_mode.cpp; onEvent() is defined
in fragment_editor_controls.cpp — both are member functions of this one
class, split across two .cpp files that share state through
fragment_editor_state.h.
*/
class FragmentEditorMode : public IMode {
public:
    void onEnter() override;
    void onEvent(const SDL_Event& e) override;
    void onRender() override;
};
