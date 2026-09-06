#include "panel.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "json_min.h"

namespace {

constexpr const char* kRegistryPath = "assets/panels/panels.json";

std::vector<PanelInfo> g_panels;
std::string            g_activeFile;

[[noreturn]] void fail(const std::string& message) {
    fprintf(stderr, "panels: %s\n", message.c_str());
    exit(1);
}

// Parses one {"file": "...", "name": "..."} object — same manual
// key-dispatch style as core/tiles.cpp's parseEntry.
const char* parseEntry(const char* p, PanelInfo& out) {
    p = jChar(p, '{');
    while (p && *p && *p != '}') {
        char key[32] = {};
        p = jString(p, key, sizeof(key));
        p = jChar(p, ':');

        char strBuf[256] = {};
        if      (strcmp(key, "file") == 0) { p = jString(p, strBuf, sizeof(strBuf)); out.file = strBuf; }
        else if (strcmp(key, "name") == 0) { p = jString(p, strBuf, sizeof(strBuf)); out.name = strBuf; }
        else                                 { p = jSkipValue(p); } // forward-compatible: ignore unknown keys

        p = jSkip(p);
        if (p && *p == ',') ++p;
        p = jSkip(p);
    }
    p = jChar(p, '}');
    return p;
}

} // namespace

void loadPanelRegistry() {
    g_panels.clear();

    FILE* f = fopen(kRegistryPath, "rb");
    if (!f) fail(std::string("could not open \"") + kRegistryPath + "\"");
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);
    std::vector<char> buf((size_t)len + 1);
    fread(buf.data(), 1, (size_t)len, f);
    fclose(f);
    buf[(size_t)len] = '\0';

    const char* p = jChar(buf.data(), '[');
    p = jSkip(p);
    while (p && *p && *p != ']') {
        PanelInfo info;
        p = parseEntry(p, info);

        if (info.file.empty()) fail("a panel entry has no \"file\"");
        if (info.name.empty()) fail("panel \"" + info.file + "\" has no \"name\"");
        g_panels.push_back(info);

        p = jSkip(p);
        if (p && *p == ',') ++p;
        p = jSkip(p);
    }

    if (g_panels.empty()) fail(std::string(kRegistryPath) + " defines no panel themes");

    g_activeFile = g_panels.front().file;
}

const std::vector<PanelInfo>& getPanels() { return g_panels; }
const std::string& getActivePanelFile() { return g_activeFile; }

bool setActivePanel(const std::string& file) {
    for (const PanelInfo& pi : g_panels) {
        if (pi.file != file) continue;
        g_activeFile = file;
        return true;
    }
    return false;
}
