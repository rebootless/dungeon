#include "settings_store.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <vector>

#include "json_min.h"

namespace {

namespace fs = std::filesystem;

constexpr const char* kDefaultsPath = "assets/settings_defaults.json";
constexpr const char* kStorageDir   = "storage";
constexpr const char* kStoragePath  = "storage/settings.json";

[[noreturn]] void fail(const std::string& message) {
    fprintf(stderr, "settings: %s\n", message.c_str());
    exit(1);
}

// Reads a whole file into a NUL-terminated buffer, or an empty vector if
// it doesn't exist — the one place this module treats a missing file as
// "nothing to layer" rather than an error, since storage/settings.json
// legitimately doesn't exist until the player changes something for the
// first time.
std::vector<char> readFileOrEmpty(const std::string& path) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return {};
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);
    std::vector<char> buf((size_t)len + 1);
    fread(buf.data(), 1, (size_t)len, f);
    fclose(f);
    buf[(size_t)len] = '\0';
    return buf;
}

// Parses {"resolution": "...", "palette": "...", "panel": "..."},
// overwriting only whichever fields of `out` are actually present in
// `buf` — an empty or partial buffer leaves the rest of `out` exactly as
// the caller passed it in, which is what lets storage/settings.json only
// mention the fields the player has actually changed.
void parseInto(const std::vector<char>& buf, Settings& out) {
    if (buf.empty()) return;

    const char* p = jChar(buf.data(), '{');
    while (p && *p && *p != '}') {
        char key[32] = {};
        p = jString(p, key, sizeof(key));
        p = jChar(p, ':');

        char strBuf[256] = {};
        if      (strcmp(key, "resolution") == 0) { p = jString(p, strBuf, sizeof(strBuf)); out.resolution = strBuf; }
        else if (strcmp(key, "palette")    == 0) { p = jString(p, strBuf, sizeof(strBuf)); out.palette    = strBuf; }
        else if (strcmp(key, "panel")      == 0) { p = jString(p, strBuf, sizeof(strBuf)); out.panel      = strBuf; }
        else                                       { p = jSkipValue(p); } // forward-compatible: ignore unknown keys

        p = jSkip(p);
        if (p && *p == ',') ++p;
        p = jSkip(p);
    }
}

} // namespace

Settings loadSettings() {
    Settings settings;

    std::vector<char> defaultsBuf = readFileOrEmpty(kDefaultsPath);
    if (defaultsBuf.empty()) fail(std::string("could not open \"") + kDefaultsPath + "\"");
    parseInto(defaultsBuf, settings);

    parseInto(readFileOrEmpty(kStoragePath), settings);
    return settings;
}

void saveSettings(const Settings& settings) {
    if (!fs::exists(kStorageDir)) fs::create_directory(kStorageDir);

    FILE* f = fopen(kStoragePath, "wb");
    if (!f) {
        fprintf(stderr, "settings: could not write \"%s\"\n", kStoragePath);
        return;
    }

    fprintf(f, "{\n");
    fprintf(f, "    \"resolution\": \"%s\",\n", jEscape(settings.resolution.c_str()).c_str());
    fprintf(f, "    \"palette\": \"%s\",\n",    jEscape(settings.palette.c_str()).c_str());
    fprintf(f, "    \"panel\": \"%s\"\n",       jEscape(settings.panel.c_str()).c_str());
    fprintf(f, "}\n");
    fclose(f);
}
