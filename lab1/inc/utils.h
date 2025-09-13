#pragma once
#include <cstdint>
#include <functional>

class Automaton;

namespace Utils
{

void ForAllCells(const Automaton &a, const std::function<void(int gx, int gy, uint8_t state)> &fn);

void ForAllCellsMut(Automaton &a, const std::function<void(int gx, int gy, uint8_t &state)> &fn);

void ForAllPixels(const Automaton &a,
                  int scale,
                  const std::function<void(int ox, int oy, int gx, int gy, uint8_t state)> &fn);

inline constexpr int Index(int x, int y, int w) noexcept
{
    return y * w + x;
}

}  // namespace Utils
