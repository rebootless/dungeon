#include "fragment_editor_mode.h"

#include <algorithm>

#include "../core/display.h"
#include "../core/level.h"
#include "../core/renderer.h"
#include "../core/tiles.h"
#include "../help/help_mode.h"
#include "../settings/settings_mode.h"
#include "fragment_editor_state.h"

// frPlaceTile
void frPlaceTile(int gx, int gy) {
    if (frActiveLayer == FragmentEditLayer::COLLISION) {
        frCollisionMap[gy][gx] = (frActiveCollisionTool == FragmentCollisionTool::STAIRS_UP)   ? STAIRS_UP_MARKER
                                : (frActiveCollisionTool == FragmentCollisionTool::STAIRS_DOWN) ? STAIRS_DOWN_MARKER
                                                                                                 : COLLISION_MARKER;
        return;
    }

    if (frActiveLayer == FragmentEditLayer::OCCLUSION) {
        frOcclusionMap[gy][gx] = OCCLUSION_MARKER;
        return;
    }

    if (frActiveLayer == FragmentEditLayer::CONNECTOR) {
        // Single-cell, exactly like the collision/occlusion markers above —
        // no auto-stamping, one click marks one candidate stitching point.
        frConnectorMap[gy][gx] = CONNECTOR_MARKER;
        return;
    }

    TileMetadata meta = getTileMeta(frSelectedTile);

    if (gx + meta.w > MAX_WIDTH || gy + meta.h > MAX_HEIGHT) return;

    TileID (*map)[MAX_WIDTH] = frGetLayerMap(frActiveLayer);

    for (int dy = 0; dy < meta.h; ++dy) {
        for (int dx = 0; dx < meta.w; ++dx) {
            int cx = gx + dx, cy = gy + dy;
            if (frMtMap[cy][cx].occupied) {
                int ax = frMtMap[cy][cx].anchorX;
                int ay = frMtMap[cy][cx].anchorY;
                TileMetadata old = getTileMeta(map[ay][ax]);
                for (int oy = 0; oy < old.h && ay + oy < MAX_HEIGHT; ++oy)
                    for (int ox = 0; ox < old.w && ax + ox < MAX_WIDTH; ++ox) {
                        map[ay + oy][ax + ox] = EMPTY_ID;
                        frMtMap[ay + oy][ax + ox] = { false, 0, 0 };
                    }
            }
        }
    }

    for (int dy = 0; dy < meta.h; ++dy) {
        for (int dx = 0; dx < meta.w; ++dx) {
            int cx = gx + dx, cy = gy + dy;
            map[cy][cx] = makeTile(tileX(frSelectedTile) + dx, tileY(frSelectedTile) + dy);
            if (dx == 0 && dy == 0) {
                frMtMap[cy][cx] = { false, 0, 0 };
            } else {
                frMtMap[cy][cx] = { true, gx, gy };
            }
        }
    }
}

// frEraseTile
void frEraseTile(int gx, int gy) {
    if (frActiveLayer == FragmentEditLayer::COLLISION) {
        frCollisionMap[gy][gx] = EMPTY_ID;
        return;
    }

    if (frActiveLayer == FragmentEditLayer::OCCLUSION) {
        frOcclusionMap[gy][gx] = EMPTY_ID;
        return;
    }

    if (frActiveLayer == FragmentEditLayer::CONNECTOR) {
        frConnectorMap[gy][gx] = EMPTY_ID;
        return;
    }

    TileID (*map)[MAX_WIDTH] = frGetLayerMap(frActiveLayer);

    int ax = gx, ay = gy;
    if (frMtMap[gy][gx].occupied) {
        ax = frMtMap[gy][gx].anchorX;
        ay = frMtMap[gy][gx].anchorY;
    }

    TileMetadata meta = getTileMeta(map[ay][ax]);
    for (int dy = 0; dy < meta.h; ++dy) {
        for (int dx = 0; dx < meta.w; ++dx) {
            int cx = ax + dx, cy = ay + dy;
            if (cx < MAX_WIDTH && cy < MAX_HEIGHT) {
                map[cy][cx] = EMPTY_ID;
                frMtMap[cy][cx] = { false, 0, 0 };
            }
        }
    }
}

/*
handlePointerAction
Same shape as editor_controls.cpp's version — the right panel picks a
fragment id (row -> selectedFragmentId) instead of a world coordinate,
and painting is not bounded to the top/bottom rows the way EditorMode's
is (fragments have no player-facing play-area restriction to protect).
*/
static void handlePointerAction(int mx, int my, int panelContentW, int mapOriginX,
                                 int mapRightEdgeX, int rightPanelX) {
    if (mx >= rightPanelX) {
        if (!frMouseDown) return;
        int rowH       = CELL_SIZE;
        int listStartY = PALETTE_MARGIN + rowH * 2; // header + legend rows
        int rowIndex   = (my + fragmentListScroll - listStartY) / rowH;

        std::vector<int> ids = listFragmentIds();
        if (rowIndex >= 0 && rowIndex < (int)ids.size()) {
            selectedFragmentId = ids[rowIndex];
            loadFragmentIntoEditor(selectedFragmentId);
        }
        return;
    }

    if (mx < panelContentW) {
        if (frMouseDown) {
            int adjustedX = mx;
            int adjustedY = my + frPaletteScroll;

            for (const FragmentPaletteEntry& entry : computeFragmentPaletteLayout(panelContentW)) {
                const SDL_Rect& r = entry.rect;
                if (adjustedX >= r.x && adjustedX < r.x + r.w &&
                    adjustedY >= r.y && adjustedY < r.y + r.h) {
                    frSelectedTile = entry.id;
                    FragmentEditLayer detected = frLayerForTile(frSelectedTile);
                    if (detected != FragmentEditLayer::COLLISION)
                        frActiveLayer = detected;
                    break;
                }
            }
        }
    }
    else if (mx >= mapOriginX && mx < mapRightEdgeX) {
        int gx = (mx - mapOriginX) / CELL_SIZE;
        int gy = my / CELL_SIZE;

        if (gx >= 0 && gx < MAX_WIDTH && gy >= 0 && gy < MAX_HEIGHT) {
            if (frRightMouseDown) frEraseTile(gx, gy);
            else                  frPlaceTile(gx, gy);
        }
    }
}

// Converts a real-window mouse position to logical-canvas coordinates and
// dispatches it, ignoring clicks that land in the letterbox border.
static void handlePointerEvent(int windowX, int windowY) {
    int winW = 0, winH = 0;
    SDL_GetWindowSize(getWindow(), &winW, &winH);

    int lx, ly;
    if (!displayWindowToLogical(windowX, windowY, winW, winH, lx, ly))
        return;

    handlePointerAction(lx, ly, PANEL_W, MAP_ORIGIN_X, MAP_RIGHT_EDGE_X, RIGHT_PANEL_X);
}

/*
FragmentEditorMode::onEvent
Arrow keys resize the fragment's footprint from its fixed top-left
corner instead of stepping a world coordinate (see EditorMode's
selectedCoord stepping for the analogous but different EditorMode
behaviour); PageUp/PageDown step the fragment id, taking WASD/arrow's
place in that role since there's no (x,y,floor) triple to walk through.
*/
void FragmentEditorMode::onEvent(const SDL_Event& e) {
    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.scancode) {
            case SDL_SCANCODE_ESCAPE:
                SettingsMode::setReturnMode(SettingsMode::ReturnMode::FragmentEditor);
                context_.switchMode(std::make_unique<SettingsMode>());
                return;

            case SDL_SCANCODE_Q:
                context_.requestQuit();
                return;

            case SDL_SCANCODE_H:
                HelpMode::setReturnMode(HelpMode::ReturnMode::FragmentEditor);
                context_.switchMode(std::make_unique<HelpMode>());
                return;

            case SDL_SCANCODE_1:
                frActiveLayer         = FragmentEditLayer::COLLISION;
                frActiveCollisionTool = FragmentCollisionTool::BLOCK;
                frEditorStatus        = " Placing: Collision marker";
                frEditorStatusTTL     = 90;
                break;

            case SDL_SCANCODE_2:
                frActiveLayer         = FragmentEditLayer::COLLISION;
                frActiveCollisionTool = FragmentCollisionTool::STAIRS_DOWN;
                frEditorStatus        = " Placing: Stairs DOWN marker";
                frEditorStatusTTL     = 90;
                break;
            case SDL_SCANCODE_3:
                frActiveLayer         = FragmentEditLayer::COLLISION;
                frActiveCollisionTool = FragmentCollisionTool::STAIRS_UP;
                frEditorStatus        = " Placing: Stairs UP marker";
                frEditorStatusTTL     = 90;
                break;

            case SDL_SCANCODE_4:
                frActiveLayer     = FragmentEditLayer::OCCLUSION;
                frEditorStatus    = " Placing: Occlusion marker";
                frEditorStatusTTL = 90;
                break;

            case SDL_SCANCODE_5:
                frActiveLayer     = FragmentEditLayer::CONNECTOR;
                frEditorStatus    = " Placing: Connector marker";
                frEditorStatusTTL = 90;
                break;

            /*
            Fragment size — grown/shrunk from the fixed top-left corner
            (0,0). Clamped to [1, MAX_WIDTH]/[1, MAX_HEIGHT]; painting
            itself is never restricted to this rectangle (see frPlaceTile).
            */
            case SDL_SCANCODE_LEFT:
                fragmentWidth = std::max(1, fragmentWidth - 1);
                break;

            case SDL_SCANCODE_RIGHT:
                fragmentWidth = std::min(MAX_WIDTH, fragmentWidth + 1);
                break;

            case SDL_SCANCODE_UP:
                fragmentHeight = std::max(1, fragmentHeight - 1);
                break;

            case SDL_SCANCODE_DOWN:
                fragmentHeight = std::min(MAX_HEIGHT, fragmentHeight + 1);
                break;

            /*
            Fragment id selection
            Steps selectedFragmentId — the F5/F9 target. Works whether or
            not that id has a saved file yet, exactly like EditorMode's
            selectedCoord stepping (step to an unused id, draw, F5).
            */
            case SDL_SCANCODE_PAGEUP:
                ++selectedFragmentId;
                loadFragmentIntoEditor(selectedFragmentId);
                break;

            case SDL_SCANCODE_PAGEDOWN:
                --selectedFragmentId;
                loadFragmentIntoEditor(selectedFragmentId);
                break;

            case SDL_SCANCODE_F5:
                saveFragmentEditorTo(selectedFragmentId);
                break;

            case SDL_SCANCODE_F9:
                loadFragmentIntoEditor(selectedFragmentId);
                break;

            default: break;
        }
        return;
    }

    if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (e.button.button == SDL_BUTTON_LEFT)  frMouseDown      = true;
        if (e.button.button == SDL_BUTTON_RIGHT) frRightMouseDown = true;
        handlePointerEvent(e.button.x, e.button.y);
        return;
    }

    if (e.type == SDL_MOUSEBUTTONUP) {
        if (e.button.button == SDL_BUTTON_LEFT)  frMouseDown      = false;
        if (e.button.button == SDL_BUTTON_RIGHT) frRightMouseDown = false;
        return;
    }

    if (e.type == SDL_MOUSEMOTION) {
        if (frMouseDown || frRightMouseDown) handlePointerEvent(e.motion.x, e.motion.y);
        return;
    }

    if (e.type == SDL_MOUSEWHEEL) {
        int winMx = 0, winMy = 0;
        SDL_GetMouseState(&winMx, &winMy);

        int winW = 0, winH = 0;
        SDL_GetWindowSize(getWindow(), &winW, &winH);

        int lx = 0, ly = 0;
        bool onCanvas = displayWindowToLogical(winMx, winMy, winW, winH, lx, ly);

        if (onCanvas && lx >= RIGHT_PANEL_X) {
            fragmentListScroll -= e.wheel.y * CELL_SIZE * 3;
            if (fragmentListScroll < 0) fragmentListScroll = 0;
        } else {
            frPaletteScroll -= e.wheel.y * PALETTE_CELL * 3;
            if (frPaletteScroll < 0) frPaletteScroll = 0;
        }
        return;
    }
}
