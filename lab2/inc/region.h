#ifndef REGION_H
#define REGION_H

#include "bmp.h"
#include "utils.h"
#include <cstdint>
#include <string>
#include <vector>

namespace ct
{

enum class BrushType : uint8_t {
    Rect = 0,
    Circle = 1
};

struct Brush {
    BrushType type{BrushType::Rect};

    int x{0}, y{0}, w{0}, h{0};
    int cx{0}, cy{0}, r{0};
};

struct RegionMask {
    std::vector<uint8_t> bytes;
    uint32_t width{0};
    uint32_t height{0};
};

RegionMask makeEmptyMask(uint32_t width, uint32_t height);
void applyBrush(RegionMask &m, const Brush &b);
void applyBrushes(RegionMask &m, const std::vector<Brush> &brushes);

Status saveMaskBmp(const std::string &path, const RegionMask &m, std::string &outMessage);

}  // namespace ct

#endif  // REGION_H
