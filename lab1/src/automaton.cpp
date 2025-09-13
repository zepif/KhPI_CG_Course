#include "automaton.h"
#include "config.h"
#include "utils.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <unordered_set>

Automaton::Automaton()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

void Automaton::Resize(int W, int H)
{
    w = std::max(1, W);
    h = std::max(1, H);
    grid.assign(w * h, 0);
    next.assign(w * h, 0);
    init = grid;
    active.clear();
    iter = 0;
}

void Automaton::Clear()
{
    std::fill(grid.begin(), grid.end(), 0);
    active.clear();
    iter = 0;
}

void Automaton::Randomize(double p)
{
    if (p < 0.0) {
        p = 0.0;
    }
    if (p > 1.0) {
        p = 1.0;
    }

    const unsigned threshold = static_cast<unsigned>(p * Cfg::Automaton::RANDOM_SCALE);

    for (int i = 0; i < w * h; ++i) {
        const unsigned r = static_cast<unsigned>(std::rand());
        grid[i] = (r % Cfg::Automaton::RANDOM_SCALE) < threshold ? 1u : 0u;
    }

    RebuildActive();
    iter = 0;
}

void Automaton::SetInitFromCurrent()
{
    init = grid;
}

void Automaton::ResetToInit()
{
    grid = init;
    RebuildActive();
    iter = 0;
}

void Automaton::RebuildActive()
{
    active.clear();
    active.reserve(static_cast<size_t>((w * h) / Cfg::Automaton::ACTIVE_RESERVE_DIVISOR + 1));

    for (int i = 0; i < w * h; ++i) {
        if (grid[i]) {
            active.insert(i);
        }
    }
}

int Automaton::CountNeighbors4(int x, int y) const
{
    int cnt = 0;
    if (wrap) {
        const int yu = (y == 0) ? (h - 1) : (y - 1);
        const int yd = (y == h - 1) ? 0 : (y + 1);
        const int xl = (x == 0) ? (w - 1) : (x - 1);
        const int xr = (x == w - 1) ? 0 : (x + 1);
        cnt += grid[Utils::Index(x, yu, w)] ? 1 : 0;
        cnt += grid[Utils::Index(x, yd, w)] ? 1 : 0;
        cnt += grid[Utils::Index(xl, y, w)] ? 1 : 0;
        cnt += grid[Utils::Index(xr, y, w)] ? 1 : 0;
    } else {
        if (y > 0) {
            cnt += grid[Utils::Index(x, y - 1, w)] ? 1 : 0;
        }
        if (y < h - 1) {
            cnt += grid[Utils::Index(x, y + 1, w)] ? 1 : 0;
        }
        if (x > 0) {
            cnt += grid[Utils::Index(x - 1, y, w)] ? 1 : 0;
        }
        if (x < w - 1) {
            cnt += grid[Utils::Index(x + 1, y, w)] ? 1 : 0;
        }
    }
    return cnt;
}

void Automaton::StepFull()
{
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const int i = Utils::Index(x, y, w);
            const int n = CountNeighbors4(x, y);
            next[i] = NextState(grid[i], n);
        }
    }
    grid.swap(next);
    RebuildActive();
    ++iter;
}

void Automaton::StepSparse()
{
    if (active.empty() && !ZeroZeroSpawnsOne()) {
        ++iter;
        return;
    }
    if (ZeroZeroSpawnsOne()) {
        StepFull();
        return;
    }

    const int W = w, H = h;
    std::unordered_set<int> cand;
    cand.reserve(active.size() * static_cast<size_t>(Cfg::Automaton::SPARSE_CANDIDATE_FACTOR) + 8u);

    auto push = [&](int x, int y) {
        if (x >= 0 && x < W && y >= 0 && y < H) {
            cand.insert(Utils::Index(x, y, W));
        }
    };
    auto wrapx = [&](int x) { return x < 0 ? W - 1 : (x >= W ? 0 : x); };
    auto wrapy = [&](int y) { return y < 0 ? H - 1 : (y >= H ? 0 : y); };

    for (int i : active) {
        const int y = i / W;
        const int x = i % W;
        cand.insert(i);
        if (wrap) {
            push(wrapx(x - 1), y);
            push(wrapx(x + 1), y);
            push(x, wrapy(y - 1));
            push(x, wrapy(y + 1));
        } else {
            if (x > 0) {
                push(x - 1, y);
            }
            if (x < W - 1) {
                push(x + 1, y);
            }
            if (y > 0) {
                push(x, y - 1);
            }
            if (y < H - 1) {
                push(x, y + 1);
            }
        }
    }

    std::unordered_set<int> nextActive;
    nextActive.reserve(cand.size());

    for (int i : cand) {
        const int y = i / W;
        const int x = i % W;
        const int nnz = CountNeighbors4(x, y);
        const uint8_t ns = NextState(grid[i], nnz);
        if (ns) {
            nextActive.insert(i);
        }
    }

    for (int i : active) {
        if (nextActive.find(i) == nextActive.end()) {
            grid[i] = 0;
        }
    }
    for (int i : nextActive) {
        grid[i] = 1;
    }

    active.swap(nextActive);
    ++iter;
}

void Automaton::Step()
{
    const std::size_t total = static_cast<std::size_t>(w) * static_cast<std::size_t>(h);
    const std::size_t k = active.size();
    const bool looksDense = (k * static_cast<std::size_t>(Cfg::Automaton::SPARSE_CANDIDATE_FACTOR) >= total);
    if (looksDense) {
        StepFull();
    } else {
        StepSparse();
    }
}
