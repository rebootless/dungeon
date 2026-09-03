#include "json_min.h"

#include <cstring>

const char* jSkip(const char* p) {
    if (!p) return p;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') ++p;
    return p;
}

const char* jChar(const char* p, char c) {
    p = jSkip(p);
    return (p && *p == c) ? p + 1 : p;
}

const char* jString(const char* p, char* out, int maxLen) {
    p = jChar(p, '"');
    int i = 0;
    while (p && *p && *p != '"' && i < maxLen - 1) out[i++] = *p++;
    out[i] = '\0';
    if (p && *p == '"') ++p;
    return p;
}

const char* jInt(const char* p, long long* v) {
    p = jSkip(p);
    bool neg = (p && *p == '-');
    if (neg) ++p;
    *v = 0;
    while (p && *p >= '0' && *p <= '9') *v = *v * 10 + (*p++ - '0');
    if (neg) *v = -*v;
    return p;
}

const char* jBool(const char* p, bool* v) {
    p = jSkip(p);
    if (p && strncmp(p, "true",  4) == 0) { *v = true;  return p + 4; }
    if (p && strncmp(p, "false", 5) == 0) { *v = false; return p + 5; }
    return p;
}

const char* jSkipValue(const char* p) {
    p = jSkip(p);
    if (!p || !*p) return p;

    if (*p == '"') {
        // String
        ++p;
        while (*p && *p != '"') { if (*p == '\\') ++p; if (*p) ++p; }
        if (*p == '"') ++p;
    } else if (*p == '[' || *p == '{') {
        // Array or object — track bracket depth
        char open = *p, close = (open == '[') ? ']' : '}';
        ++p;
        int depth = 1;
        while (*p && depth > 0) {
            if (*p == '"') {
                ++p;
                while (*p && *p != '"') { if (*p == '\\') ++p; if (*p) ++p; }
                if (*p == '"') ++p;
            } else if (*p == open)  { ++depth; ++p; }
            else if (*p == close) { --depth; ++p; }
            else                  { ++p; }
        }
    } else if (*p == 't') { p += 4; }  // true
    else if (*p == 'f')   { p += 5; }  // false
    else if (*p == 'n')   { p += 4; }  // null
    else {
        // Number: skip all numeric characters
        while (*p && *p != ',' && *p != ']' && *p != '}') ++p;
    }
    return p;
}

const char* j2DArray(const char* p, TileID map[][MAX_WIDTH]) {
    p = jChar(p, '[');
    for (int y = 0; y < MAX_HEIGHT; ++y) {
        p = jChar(p, '[');
        for (int x = 0; x < MAX_WIDTH; ++x) {
            long long v = 0;
            p = jInt(p, &v);
            map[y][x] = (TileID)(unsigned short)v;
            p = jSkip(p);
            if (p && *p == ',') ++p; // skip value separator
        }
        p = jChar(p, ']');
        p = jSkip(p);
        if (p && *p == ',') ++p; // skip row separator
    }
    p = jChar(p, ']');
    return p;
}

void jWrite2DArray(FILE* f, const TileID map[MAX_HEIGHT][MAX_WIDTH]) {
    fprintf(f, "[\n");
    for (int y = 0; y < MAX_HEIGHT; ++y) {
        fprintf(f, "    [");
        for (int x = 0; x < MAX_WIDTH; ++x)
            fprintf(f, "%u%s", (unsigned)map[y][x], (x + 1 < MAX_WIDTH) ? "," : "");
        fprintf(f, "]%s\n", (y + 1 < MAX_HEIGHT) ? "," : "");
    }
    fprintf(f, "  ]");
}

std::string jEscape(const char* s) {
    std::string out;
    for (const char* p = s; *p; ++p) {
        if (*p == '"' || *p == '\\') out += '\\';
        out += *p;
    }
    return out;
}
