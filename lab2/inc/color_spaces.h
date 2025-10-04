#ifndef COLOR_SPACES_H
#define COLOR_SPACES_H

#include "bmp.h"
#include <vector>

namespace ct
{

struct Vec3d {
    double x{0}, y{0}, z{0};
};

struct RgbNorm {
    double r, g, b;
};

RgbNorm toRgbNorm(const Bgr &p, bool compressWhite);
Bgr fromRgbNorm(const RgbNorm &rn, bool expandWhite);

Vec3d rgbToLms(const RgbNorm &rn);
Vec3d lmsToLab(const Vec3d &lms);
Vec3d labToLms(const Vec3d &lab);
RgbNorm lmsToRgb(const Vec3d &lms);

Vec3d rgbToLab(const Bgr &p, bool compressWhite);
Bgr labToRgb(const Vec3d &lab, bool expandWhite);

#ifdef ENABLE_YIQ
Vec3d rgbToYiq(const RgbNorm &rn);
RgbNorm yiqToRgb(const Vec3d &yiq);
#endif

#ifdef ENABLE_HSL_HSV
Vec3d rgbToHsl(const RgbNorm &rn);  // h[0..360), s,l [0..1]
RgbNorm hslToRgb(const Vec3d &hsl);
Vec3d rgbToHsv(const RgbNorm &rn);  // h[0..360), s,v [0..1]
RgbNorm hsvToRgb(const Vec3d &hsv);
#endif

}  // namespace ct

#endif  // COLOR_SPACES_H
