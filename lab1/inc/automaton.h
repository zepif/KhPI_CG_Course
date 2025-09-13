#pragma once
#include "config.h"
#include "utils.h"
#include <cstdint>
#include <unordered_set>
#include <vector>

class Automaton
{
public:
    Automaton();

    void Resize(int w, int h);

    inline int Width() const noexcept
    {
        return w;
    }
    inline int Height() const noexcept
    {
        return h;
    }

    inline void SetWrap(bool Wrap) noexcept
    {
        wrap = Wrap;
    }
    inline bool Wrap() const noexcept
    {
        return wrap;
    }

    inline void SetRuleBits(uint16_t bits) noexcept
    {
        ruleBits = bits;
    }
    inline uint16_t RuleBits() const noexcept
    {
        return ruleBits;
    }

    inline uint32_t Iteration() const noexcept
    {
        return iter;
    }

    inline uint8_t Cell(int x, int y) const
    {
        return grid[Utils::Index(x, y, w)];
    }
    inline uint8_t &Cell(int x, int y)
    {
        return grid[Utils::Index(x, y, w)];
    }

    inline void Set(int x, int y, uint8_t v)
    {
        const int i = Utils::Index(x, y, w);
        const uint8_t old = grid[i];
        const uint8_t nv = v ? 1u : 0u;
        if (old != nv) {
            grid[i] = nv;
            if (nv) {
                active.insert(i);
            } else {
                active.erase(i);
            }
        }
    }

    const std::vector<uint8_t> &Data() const
    {
        return grid;
    }

    void Clear();
    void Randomize(double p);
    void SetInitFromCurrent();
    void ResetToInit();
    void Step();

private:
    void RebuildActive();
    int CountNeighbors4(int x, int y) const;

    inline uint8_t NextState(uint8_t curr, int nnz) const
    {
        const int idxRow = (curr ? Cfg::Automaton::RULE_ROWS_PER_CURR : 0) + nnz;  // 0..9
        const int bitpos = Cfg::Automaton::RULE_TOP_BIT_POS - idxRow;              // 9-idxRow
        return (ruleBits >> bitpos) & 1u ? 1u : 0u;
    }
    inline bool ZeroZeroSpawnsOne() const
    {
        return ((ruleBits >> Cfg::Automaton::RULE_TOP_BIT_POS) & 1u) != 0u;  // (0,0)
    }

    void StepFull();
    void StepSparse();

private:
    int w{0};
    int h{0};
    bool wrap{true};
    uint16_t ruleBits{Cfg::Automaton::DEFAULT_RULE};
    uint32_t iter{0};

    std::vector<uint8_t> grid;
    std::vector<uint8_t> next;
    std::vector<uint8_t> init;
    std::unordered_set<int> active;
};
