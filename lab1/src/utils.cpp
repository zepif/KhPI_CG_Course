#include "utils.h"
#include "automaton.h"

namespace Utils
{

void ForAllCells(const Automaton &a, const std::function<void(int, int, uint8_t)> &fn)
{
    const int w = a.Width();
    const int h = a.Height();
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            fn(x, y, a.Cell(x, y));
        }
    }
}

void ForAllCellsMut(Automaton &a, const std::function<void(int, int, uint8_t &)> &fn)
{
    const int w = a.Width();
    const int h = a.Height();
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            uint8_t &ref = a.Cell(x, y);
            fn(x, y, ref);
        }
    }
}

void ForAllPixels(const Automaton &a, int scale, const std::function<void(int, int, int, int, uint8_t)> &fn)
{
    if (scale < 1) {
        scale = 1;
    }
    const int w = a.Width();
    const int h = a.Height();

    for (int gy = 0; gy < h; ++gy) {
        for (int gx = 0; gx < w; ++gx) {
            const uint8_t s = a.Cell(gx, gy);
            const int ox0 = gx * scale;
            const int oy0 = gy * scale;
            for (int sy = 0; sy < scale; ++sy) {
                const int oy = oy0 + sy;
                for (int sx = 0; sx < scale; ++sx) {
                    const int ox = ox0 + sx;
                    fn(ox, oy, gx, gy, s);
                }
            }
        }
    }
}

}  // namespace utils
