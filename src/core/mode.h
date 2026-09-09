#pragma once

#include <functional>
#include <memory>

#include <SDL2/SDL.h>

class IMode; // forward declaration — AppContext is handed to IMode instances

/*
Callbacks App gives every mode right after constructing it, so a mode can
ask to switch to another mode or quit without holding a pointer back to
App itself. game and editor are both IMode instances owned by the same
App, so mode switching is a plain callback rather than a nested call.
*/
struct AppContext {
    std::function<void()>                       requestQuit;
    std::function<void(std::unique_ptr<IMode>)>  switchMode;
};

// Implemented by GameMode (game/) and EditorMode (editor/). App owns exactly
// one active IMode at a time and drives it every frame — see core/app.h.
class IMode {
public:
    virtual ~IMode() = default;

    // Called once when this mode becomes active (including on startup).
    virtual void onEnter() {}

    // Called once when this mode stops being active — either because the
    // console switched to another mode, or the app is shutting down.
    virtual void onExit() {}

    // Handle a single SDL event. Never called while the console overlay is
    // open — App routes input to the console instead in that case.
    virtual void onEvent(const SDL_Event& e) = 0;

    /*
    Draw into the logical canvas. App has already called beginFrame();
    the mode must not call beginFrame()/endFrame() itself. Every mode
    draws at the one fixed CANVAS_W x CANVAS_H virtual resolution
    (core/layout.h) — there is no per-mode canvas size to report any
    more; core/display.h scales the finished frame to the real window.
    */
    virtual void onRender() = 0;

    /*
    Result of a console /lightMap command (core/app.cpp), which just
    delegates straight here — each mode decides for itself what "toggle
    the lighting preview" even means: GameMode flips the loaded
    location's own authored Level::lightMap flag, while a mode with no
    single "current level" to persist a flag on (EditorMode,
    FragmentEditorMode, GeneratorMode) keeps its own ephemeral,
    always-false-on-entry preview flag instead, purely so an author can
    eyeball a light layout without leaving the editor.
    */
    enum class LightMapToggleResult { Unsupported, NoLocation, TurnedOn, TurnedOff };

    // Default: unsupported, for any mode that doesn't implement lighting
    // at all (e.g. SettingsMode, HelpMode).
    virtual LightMapToggleResult toggleLightMapPreview() { return LightMapToggleResult::Unsupported; }

    // Called by App right after construction, before onEnter().
    void setContext(AppContext ctx) { context_ = std::move(ctx); }

protected:
    AppContext context_;
};
