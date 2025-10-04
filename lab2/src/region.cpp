#include "region.h"
#include <cmath>

namespace ct
{

RegionMask makeEmptyMask(uint32_t width, uint32_t height)
{
    RegionMask m;
    m.width = width;
    m.height = height;
    m.bytes.assign(width * height, 0);
    return m;
}

static inline bool insideRect(int x, int y, const Brush &b)
{
    return x >= b.x && y >= b.y && x < (b.x + b.w) && y < (b.y + b.h);
}

static inline bool insideCircle(int x, int y, const Brush &b)
{
    int dx = x - b.cx;
    int dy = y - b.cy;
    long long d2 = 1LL * dx * dx + 1LL * dy * dy;
    long long r2 = 1LL * b.r * b.r;
    return d2 <= r2;
}

void applyBrush(RegionMask &m, const Brush &b)
{
    const uint32_t w = m.width;
    const uint32_t h = m.height;
    if (w == 0 || h == 0) {
        return;
    }

    int minx = 0, miny = 0, maxx = (int)w - 1, maxy = (int)h - 1;
    if (b.type == BrushType::Rect) {
        minx = std::max(0, b.x);
        miny = std::max(0, b.y);
        maxx = std::min((int)w - 1, b.x + b.w - 1);
        maxy = std::min((int)h - 1, b.y + b.h - 1);
    } else {
        minx = std::max(0, b.cx - b.r);
        miny = std::max(0, b.cy - b.r);
        maxx = std::min((int)w - 1, b.cx + b.r);
        maxy = std::min((int)h - 1, b.cy + b.r);
    }
    if (minx > maxx || miny > maxy) {
        return;
    }

    for (int y = miny; y <= maxy; ++y) {
        for (int x = minx; x <= maxx; ++x) {
            bool inside = (b.type == BrushType::Rect) ? insideRect(x, y, b) : insideCircle(x, y, b);
            if (inside) {
                m.bytes[y * w + x] = 1;
            }
        }
    }
}

void applyBrushes(RegionMask &m, const std::vector<Brush> &brushes)
{
    for (const auto &b : brushes) {
        applyBrush(m, b);
    }
}

Status saveMaskBmp(const std::string &path, const RegionMask &m, std::string &outMessage)
{
    Image img;
    img.width = m.width;
    img.height = m.height;
    img.pixels.assign(m.width * m.height, Bgr{0, 0, 0});
    for (uint32_t i = 0; i < m.width * m.height; ++i) {
        uint8_t v = m.bytes[i] ? 255 : 0;
        img.pixels[i] = Bgr{v, v, v};
    }
    return saveBmp(path, img, outMessage);
}

}  // namespace ct
