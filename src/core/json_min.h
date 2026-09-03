#pragma once

#include <cstdio>
#include <string>

#include "level.h" // MAX_WIDTH / MAX_HEIGHT / TileID

/*
Minimal JSON parser/writer
Handles only the flat object-of-strings/ints/bools/2D-arrays shape used by
Level and Fragment's JSON files — no nesting, no floats, no external
dependencies. Shared by core/level.cpp and generator/fragment.cpp so both
map formats (which store identically-shaped MAX_HEIGHT x MAX_WIDTH tile
layers) parse/write through the same code.
*/

const char* jSkip(const char* p);

// Advance past one expected character (skipping leading whitespace).
const char* jChar(const char* p, char c);

// Parse a JSON string literal into out[0..maxLen-1]; returns p past closing ".
const char* jString(const char* p, char* out, int maxLen);

// Parse a JSON integer (optionally negative) into *v.
const char* jInt(const char* p, long long* v);

// Parse a JSON bool (true / false) into *v.
const char* jBool(const char* p, bool* v);

// Skip one complete JSON value (used to ignore unknown keys).
const char* jSkipValue(const char* p);

// Parse a JSON 2D array [[uint,...], ...] into map[MAX_HEIGHT][MAX_WIDTH].
// Always reads exactly MAX_HEIGHT rows and MAX_WIDTH columns.
const char* j2DArray(const char* p, TileID map[][MAX_WIDTH]);

// Write map[MAX_HEIGHT][MAX_WIDTH] out as a JSON 2D array.
void jWrite2DArray(FILE* f, const TileID map[MAX_HEIGHT][MAX_WIDTH]);

// Escapes '"' and '\\' so a name typed with either doesn't corrupt the JSON.
// Not full JSON string escaping — the name field is short, editor-typed text.
std::string jEscape(const char* s);
