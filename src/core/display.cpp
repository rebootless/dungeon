#include "display.h"

#include <algorithm>

const std::vector<Resolution>& displayResolutionPresets() {
    /*
    First entry is always the minimum supported resolution (see
    instructions/minimum resolution.txt / MIN_WINDOW_W/H above). Every
    later preset is a common real-world monitor/laptop resolution, purely
    for convenience in the settings screen — the fixed CANVAS_W x
    CANVAS_H canvas scales down or up to fit any of them without ever
    changing its own layout.
    */
    static const std::vector<Resolution> kPresets = {
        { 1264, 800,  "1264x800"  }, // minimum supported resolution
        { 1280, 800,  "1280x800"  },
        { 1280, 1024, "1280x1024" },
        { 1440, 900,  "1440x900"  },
        { 1600, 900,  "1600x900"  },
        { 1600, 1024, "1600x1024" },
        { 1680, 1050, "1680x1050" },
        { 1920, 1080, "1920x1080" },
        { 1920, 1200, "1920x1200" },
        { 2048, 1152, "2048x1152" },
        { 2560, 1440, "2560x1440" },
        { 2560, 1600, "2560x1600" },
        { 2880, 1800, "2880x1800" },
        { 3000, 2000, "3000x2000" },
        { 3440, 1440, "3440x1440" },
        { 3840, 1600, "3840x1600" },
        { 3840, 2160, "3840x2160" },
        { 4096, 2160, "4096x2160" },
        { 5120, 1440, "5120x1440" },
        { 5120, 2880, "5120x2880" },
        { 7680, 4320, "7680x4320" },
    };
    return kPresets;
}

bool displayFindResolution(const std::string& name, Resolution& out) {
    for (const Resolution& r : displayResolutionPresets()) {
        if (name == r.label) { out = r; return true; }
    }
    return false;
}

std::vector<std::string> displayResolutionNames() {
    std::vector<std::string> names;
    for (const Resolution& r : displayResolutionPresets()) names.push_back(r.label);
    return names;
}

bool displayFitsMinimum(int windowW, int windowH) {
    return windowW >= MIN_WINDOW_W && windowH >= MIN_WINDOW_H;
}

int displayComputeScale(int windowW, int windowH) {
    /*
    Plain integer division — this floors both ratios for free, giving the
    largest whole number of times the canvas fits each axis. The
    max(..., 1) floor keeps the one narrower-than-canvas resolution (see
    this file's class comment) at a clean 1x instead of falling through
    to 0.
    */
    int scaleW = windowW / CANVAS_W;
    int scaleH = windowH / CANVAS_H;
    int scale  = std::min(scaleW, scaleH);
    return std::max(scale, 1);
}

SDL_Rect displayComputeDestRect(int windowW, int windowH) {
    int scale = displayComputeScale(windowW, windowH);

    int dstW = CANVAS_W * scale;
    int dstH = CANVAS_H * scale;

    SDL_Rect r;
    r.w = dstW;
    r.h = dstH;
    r.x = (windowW - dstW) / 2; // negative at the one narrower-than-canvas resolution — an even crop, not a bug
    r.y = (windowH - dstH) / 2;
    return r;
}

bool displayWindowToLogical(int windowX, int windowY, int windowW, int windowH,
                             int& outX, int& outY) {
    SDL_Rect dst = displayComputeDestRect(windowW, windowH);
    if (windowX < dst.x || windowY < dst.y ||
        windowX >= dst.x + dst.w || windowY >= dst.y + dst.h) {
        return false; // click landed in the letterbox border (or the cropped-off edge)
    }

    /*
    scale is always the same whole number on both axes here (see
    displayComputeScale), so integer division is an exact inverse — no
    float round-trip drift to worry about the way a fractional scale
    would have needed.
    */
    int scale = displayComputeScale(windowW, windowH);

    outX = std::clamp((windowX - dst.x) / scale, 0, CANVAS_W - 1);
    outY = std::clamp((windowY - dst.y) / scale, 0, CANVAS_H - 1);
    return true;
}
