#include "color_spaces.h"
#include "utils.h"
#include <algorithm>
#include <cmath>

namespace ct
{

static constexpr double MIN_VAL = 3.0 / 255.0;           // 0.01176 to avoid log(0)
static constexpr double WHITE_COMPRESS = 235.0 / 255.0;  // 0.92157

RgbNorm toRgbNorm(const Bgr &p, bool compressWhite)
{
    auto scale = compressWhite ? WHITE_COMPRESS : 1.0;
    return RgbNorm{clamp((double)p.r / 255.0 * scale, MIN_VAL, scale),
                   clamp((double)p.g / 255.0 * scale, MIN_VAL, scale),
                   clamp((double)p.b / 255.0 * scale, MIN_VAL, scale)};
}

Bgr fromRgbNorm(const RgbNorm &rn, bool expandWhite)
{
    const double scale = expandWhite ? WHITE_COMPRESS : 1.0;

    auto toByte = [scale](double v) -> uint8_t {
        double clamped = clamp(v, 0.0, scale);
        double byteVal = (clamped / scale) * 255.0;
        return clampToByte(byteVal);
    };

    return Bgr{toByte(rn.b), toByte(rn.g), toByte(rn.r)};
}

// RGB->LMS using matrix from lab
Vec3d rgbToLms(const RgbNorm &rn)
{
    double L = 0.3811 * rn.r + 0.5783 * rn.g + 0.0402 * rn.b;
    double M = 0.1967 * rn.r + 0.7244 * rn.g + 0.0782 * rn.b;
    double S = 0.0241 * rn.r + 0.1288 * rn.g + 0.8444 * rn.b;
    // log10
    L = std::log10(std::max(MIN_VAL, L));
    M = std::log10(std::max(MIN_VAL, M));
    S = std::log10(std::max(MIN_VAL, S));
    return Vec3d{L, M, S};
}

Vec3d lmsToLab(const Vec3d &lms)
{
    // Multiply by matrix:
    // [  1/sqrt(3)   1/sqrt(3)   1/sqrt(3)]
    // [  1/sqrt(6)   1/sqrt(6)  -2/sqrt(6)]
    // [  1/sqrt(2)  -1/sqrt(2)   0        ]
    const double a = 1.0 / std::sqrt(3.0);
    const double b = 1.0 / std::sqrt(6.0);
    const double c = 1.0 / std::sqrt(2.0);

    double l = a * (lms.x + lms.y + lms.z);
    double alpha = b * (lms.x + lms.y - 2.0 * lms.z);
    double beta = c * (lms.x - lms.y);
    return Vec3d{l, alpha, beta};
}

Vec3d labToLms(const Vec3d &lab)
{
    const double a = 1.0 / std::sqrt(3.0);
    const double b = 1.0 / std::sqrt(6.0);
    const double c = 1.0 / std::sqrt(2.0);

    double logL = a * lab.x + b * lab.y + c * lab.z;
    double logM = a * lab.x + b * lab.y + (-c) * lab.z;
    double logS = a * lab.x + (-2.0) * b * lab.y + 0.0 * lab.z;
    return Vec3d{logL, logM, logS};
}

RgbNorm lmsToRgb(const Vec3d &lmsLog)
{
    // 10^log10 -> LMS
    double L = std::pow(10.0, lmsLog.x);
    double M = std::pow(10.0, lmsLog.y);
    double S = std::pow(10.0, lmsLog.z);
    // LMS->RGB using matrix from lab
    double r = 4.4679 * L - 3.5873 * M + 0.1193 * S;
    double g = -1.2186 * L + 2.3809 * M - 0.1624 * S;
    double b = 0.0497 * L - 0.2439 * M + 1.2045 * S;
    return RgbNorm{r, g, b};
}

Vec3d rgbToLab(const Bgr &p, bool compressWhite)
{
    auto rn = toRgbNorm(p, compressWhite);
    auto lms = rgbToLms(rn);
    return lmsToLab(lms);
}

Bgr labToRgb(const Vec3d &lab, bool expandWhite)
{
    auto lms = labToLms(lab);
    auto rn = lmsToRgb(lms);
    return fromRgbNorm(rn, expandWhite);
}

#ifdef ENABLE_YIQ
Vec3d rgbToYiq(const RgbNorm &rn)
{
    double Y = 0.299 * rn.r + 0.587 * rn.g + 0.114 * rn.b;
    double I = 0.596 * rn.r - 0.275 * rn.g - 0.321 * rn.b;
    double Q = 0.212 * rn.r - 0.528 * rn.g + 0.311 * rn.b;
    return Vec3d{Y, I, Q};
}
RgbNorm yiqToRgb(const Vec3d &yiq)
{
    double r = yiq.x + 0.956 * yiq.y + 0.621 * yiq.z;
    double g = yiq.x - 0.272 * yiq.y - 0.647 * yiq.z;
    double b = yiq.x - 1.106 * yiq.y - 1.703 * yiq.z;
    return RgbNorm{r, g, b};
}
#endif

#ifdef ENABLE_HSL_HSV
static double hueToRgb(double p, double q, double t)
{
    if (t < 0) {
        t += 1;
    }
    if (t > 1) {
        t -= 1;
    }
    if (t < 1.0 / 6) {
        return p + (q - p) * 6 * t;
    }
    if (t < 1.0 / 2) {
        return q;
    }
    if (t < 2.0 / 3) {
        return p + (q - p) * (2.0 / 3 - t) * 6;
    }
    return p;
}

Vec3d rgbToHsl(const RgbNorm &rn)
{
    double r = rn.r, g = rn.g, b = rn.b;
    double maxv = std::max({r, g, b});
    double minv = std::min({r, g, b});
    double h = 0, s = 0, l = (maxv + minv) / 2.0;
    if (maxv != minv) {
        double d = maxv - minv;
        s = l > 0.5 ? d / (2.0 - maxv - minv) : d / (maxv + minv);
        if (maxv == r) {
            h = (g - b) / d + (g < b ? 6 : 0);
        } else if (maxv == g) {
            h = (b - r) / d + 2;
        } else {
            h = (r - g) / d + 4;
        }
        h *= 60.0;
    }
    return Vec3d{h, s, l};
}

RgbNorm hslToRgb(const Vec3d &hsl)
{
    double h = hsl.x, s = hsl.y, l = hsl.z;
    h = std::fmod(std::fmod(h, 360.0) + 360.0, 360.0) / 360.0;
    double r, g, b;
    if (s == 0) {
        r = g = b = l;
    } else {
        double q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        double p = 2 * l - q;
        r = hueToRgb(p, q, h + 1.0 / 3);
        g = hueToRgb(p, q, h);
        b = hueToRgb(p, q, h - 1.0 / 3);
    }
    return RgbNorm{r, g, b};
}

Vec3d rgbToHsv(const RgbNorm &rn)
{
    double r = rn.r, g = rn.g, b = rn.b;
    double maxv = std::max({r, g, b});
    double minv = std::min({r, g, b});
    double v = maxv;
    double d = maxv - minv;
    double s = maxv == 0 ? 0 : d / maxv;
    double h = 0;
    if (d != 0) {
        if (maxv == r) {
            h = (g - b) / d + (g < b ? 6 : 0);
        } else if (maxv == g) {
            h = (b - r) / d + 2;
        } else {
            h = (r - g) / d + 4;
        }
        h *= 60.0;
    }
    return Vec3d{h, s, v};
}

RgbNorm hsvToRgb(const Vec3d &hsv)
{
    double h = hsv.x, s = hsv.y, v = hsv.z;
    h = std::fmod(std::fmod(h, 360.0) + 360.0, 360.0) / 60.0;
    int i = (int)std::floor(h);
    double f = h - i;
    double p = v * (1 - s);
    double q = v * (1 - s * f);
    double t = v * (1 - s * (1 - f));
    double r, g, b;
    switch (i % 6) {
        case 0:
            r = v;
            g = t;
            b = p;
            break;
        case 1:
            r = q;
            g = v;
            b = p;
            break;
        case 2:
            r = p;
            g = v;
            b = t;
            break;
        case 3:
            r = p;
            g = q;
            b = v;
            break;
        case 4:
            r = t;
            g = p;
            b = v;
            break;
        default:
            r = v;
            g = p;
            b = q;
            break;
    }
    return RgbNorm{r, g, b};
}
#endif

}  // namespace ct
