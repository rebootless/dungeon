#pragma once

#include <string>

/*
Persisted settings
Two layers. assets/settings_defaults.json ships with the game and is
never written to — it's what a fresh install falls back to for any
setting the player has never touched. storage/settings.json holds only
the player's own overrides, layered on top of those defaults; it doesn't
exist until saveSettings() is first called (see settings/settings_mode.cpp,
which calls it right after applying any option), at which point storage/
itself is created next to the executable if it isn't already there.

Neither loadSettings() nor saveSettings() validates a value against what's
actually loaded (a resolution preset, a palette, a panel theme) — that's
the caller's job, exactly like SettingsMode already does for Resolution
via displayFindResolution(). A stale or unknown value here just gets
handed back as-is; App::run() falls back to that field's own default
system (first preset, first palette, ...) if it doesn't resolve to
anything real.
*/
struct Settings {
    std::string resolution; // Resolution preset label, e.g. "1280x800" — see core/display.h
    std::string palette;    // Palette PNG filename, e.g. "default.png" — see core/palette.h
    std::string panel;      // Panel theme PNG filename, e.g. "simple.png" — see core/panel.h
};

// Reads assets/settings_defaults.json, then layers storage/settings.json
// on top of it field by field wherever storage/settings.json exists and
// mentions that field. Exits with an error, the same way loadTileRegistry()
// does, if assets/settings_defaults.json itself can't be read — every
// field must have a shipped default.
Settings loadSettings();

// Writes `settings` out to storage/settings.json in full, creating
// storage/ next to the executable first if this is the very first
// setting ever changed.
void saveSettings(const Settings& settings);
