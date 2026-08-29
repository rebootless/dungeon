#include "dungeon_generator.h"

#include <cstdlib>
#include <vector>

namespace {

struct PlacedFragment {
    int originX, originY;
    int width, height;
};

// One still-available connector cell on an already-placed fragment,
// in absolute canvas coordinates.
struct OpenConnector {
    int placedIndex;
    int canvasX, canvasY;
    ConnectorSide side;
};

// A candidate connector cell on a fragment that ISN'T placed yet,
// relative to that fragment's own (0,0).
struct FragmentConnector {
    int localX, localY;
    ConnectorSide side;
};

void clearCanvas(GeneratedDungeon& out) {
    for (int y = 0; y < MAX_HEIGHT; ++y) {
        for (int x = 0; x < MAX_WIDTH; ++x) {
            out.groundMap[y][x]    = EMPTY_ID;
            out.objectMap[y][x]    = EMPTY_ID;
            out.entityMap[y][x]    = EMPTY_ID;
            out.collisionMap[y][x] = EMPTY_ID;
            out.occlusionMap[y][x] = EMPTY_ID;
        }
    }
}

/*
Copies only the fragment's declared width x height footprint (not the
full MAX_WIDTH x MAX_HEIGHT buffer — see fragment.h's Fragment comment on
why painting outside that rectangle is allowed but not meaningful here).
*/
void blitFragment(const Fragment& fragment, int ox, int oy, GeneratedDungeon& out) {
    for (int y = 0; y < fragment.height; ++y) {
        for (int x = 0; x < fragment.width; ++x) {
            int cx = ox + x, cy = oy + y;
            out.groundMap[cy][cx]    = fragment.tileMap[y][x];
            out.objectMap[cy][cx]    = fragment.objectMap[y][x];
            out.entityMap[cy][cx]    = fragment.entityMap[y][x];
            out.collisionMap[cy][cx] = fragment.collisionMap[y][x];
            out.occlusionMap[cy][cx] = fragment.occlusionMap[y][x];
        }
    }
}

// Every connector cell in `fragment`, with its resolved border side —
// cells that connectorSideAt() rejects (corners/interior) are skipped.
std::vector<FragmentConnector> fragmentConnectors(const Fragment& fragment) {
    std::vector<FragmentConnector> result;
    for (int y = 0; y < fragment.height; ++y) {
        for (int x = 0; x < fragment.width; ++x) {
            if (fragment.connectorMap[y][x] != CONNECTOR_MARKER) continue;

            ConnectorSide side;
            if (connectorSideAt(x, y, fragment.width, fragment.height, side))
                result.push_back({x, y, side});
        }
    }
    return result;
}

bool rectsOverlap(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh) {
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

int dxForSide(ConnectorSide side) { return (side == ConnectorSide::Left) ? -1 : (side == ConnectorSide::Right) ? 1 : 0; }
int dyForSide(ConnectorSide side) { return (side == ConnectorSide::Top)  ? -1 : (side == ConnectorSide::Bottom) ? 1 : 0; }

} // namespace

bool generateDungeon(GeneratedDungeon& out, int maxFragments) {
    clearCanvas(out);
    out.fragmentCount = 0;

    std::vector<int> ids = listFragmentIds();
    if (ids.empty()) return false;

    std::vector<PlacedFragment> placed;
    std::vector<OpenConnector>  open;

    // Seed fragment — placed roughly in the middle of the canvas.
    int seedId = ids[rand() % ids.size()];
    const Fragment& seed = fragments.at(seedId);
    int seedOx = std::max(0, (MAX_WIDTH  - seed.width)  / 2);
    int seedOy = std::max(0, (MAX_HEIGHT - seed.height) / 2);

    blitFragment(seed, seedOx, seedOy, out);
    placed.push_back({seedOx, seedOy, seed.width, seed.height});
    out.fragmentCount = 1;

    for (const FragmentConnector& c : fragmentConnectors(seed))
        open.push_back({0, seedOx + c.localX, seedOy + c.localY, c.side});

    /*
    Bounded by both a fragment-count target and a hard attempt ceiling —
    a run of unlucky rolls (candidates that only ever overlap something)
    would otherwise spin forever once few open connectors are left.
    */
    int attempts = 0;
    constexpr int kMaxAttempts = 400;

    while (out.fragmentCount < maxFragments && !open.empty() && attempts < kMaxAttempts) {
        ++attempts;

        int openIndex = rand() % (int)open.size();
        OpenConnector c = open[openIndex];
        ConnectorSide needSide = oppositeSide(c.side);

        // Every (fragmentId, connector) pair anywhere that could plug into `c`.
        struct Candidate { int fragmentId; FragmentConnector conn; };
        std::vector<Candidate> candidates;
        for (int id : ids) {
            const Fragment& frag = fragments.at(id);
            for (const FragmentConnector& fc : fragmentConnectors(frag))
                if (fc.side == needSide) candidates.push_back({id, fc});
        }

        if (candidates.empty()) {
            // No fragment anywhere offers this side — this connector can
            // never be used; drop it instead of retrying it forever.
            open.erase(open.begin() + openIndex);
            continue;
        }

        const Candidate& pick = candidates[rand() % candidates.size()];
        const Fragment& newFrag = fragments.at(pick.fragmentId);

        int targetX = c.canvasX + dxForSide(c.side);
        int targetY = c.canvasY + dyForSide(c.side);
        int newOx   = targetX - pick.conn.localX;
        int newOy   = targetY - pick.conn.localY;

        bool fits = newOx >= 0 && newOy >= 0 &&
                    newOx + newFrag.width  <= MAX_WIDTH &&
                    newOy + newFrag.height <= MAX_HEIGHT;

        bool overlaps = false;
        if (fits) {
            for (const PlacedFragment& p : placed) {
                if (rectsOverlap(newOx, newOy, newFrag.width, newFrag.height,
                                  p.originX, p.originY, p.width, p.height)) {
                    overlaps = true;
                    break;
                }
            }
        }

        if (!fits || overlaps) {
            /*
            This particular attempt didn't work out, but a different
            candidate (or a rare more-favorable roll of the same one on a
            later attempt) still might — the connector stays open rather
            than being dropped outright, unlike the "nobody offers this
            side at all" case above.
            */
            continue;
        }

        blitFragment(newFrag, newOx, newOy, out);
        int newIndex = (int)placed.size();
        placed.push_back({newOx, newOy, newFrag.width, newFrag.height});
        ++out.fragmentCount;

        open.erase(open.begin() + openIndex);
        for (const FragmentConnector& fc : fragmentConnectors(newFrag)) {
            if (fc.localX == pick.conn.localX && fc.localY == pick.conn.localY) continue; // just consumed
            open.push_back({newIndex, newOx + fc.localX, newOy + fc.localY, fc.side});
        }
    }

    return true;
}
