#include "color_transfer.h"
#include "utils.h"
#include <cmath>
#include <numeric>

namespace ct
{

static std::string vecToStr(const Vec3d &v)
{
    return "(" + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + ")";
}

static void accumulateLab(const Image &img, Vec3d &sum, Vec3d &sumSq)
{
    sum = {0, 0, 0};
    sumSq = {0, 0, 0};
    const bool compress = true;
    for (const auto &p : img.pixels) {
        Vec3d lab = rgbToLab(p, compress);
        sum.x += lab.x;
        sum.y += lab.y;
        sum.z += lab.z;
        sumSq.x += lab.x * lab.x;
        sumSq.y += lab.y * lab.y;
        sumSq.z += lab.z * lab.z;
    }
    CT_DEBUG("accumulateLab: sum=" + vecToStr(sum) + " sumSq=" + vecToStr(sumSq) +
             " n=" + std::to_string(img.pixels.size()));
}

static Vec3d finalizeMean(size_t n, const Vec3d &sum)
{
    Vec3d m{sum.x / n, sum.y / n, sum.z / n};
    CT_DEBUG("finalizeMean: n=" + std::to_string(n) + " mean=" + vecToStr(m));
    return m;
}

static Vec3d finalizeVar(size_t n, const Vec3d &sum, const Vec3d &sumSq)
{
    Vec3d mean = finalizeMean(n, sum);
    Vec3d v{sumSq.x / n - mean.x * mean.x, sumSq.y / n - mean.y * mean.y, sumSq.z / n - mean.z * mean.z};
    CT_DEBUG("finalizeVar: var=" + vecToStr(v));
    return v;
}

Stats computeLabStats(const Image &img)
{
    CT_DEBUG("computeLabStats: begin (w=" + std::to_string(img.width) + ", h=" + std::to_string(img.height) +
             ", n=" + std::to_string(img.pixels.size()) + ")");
    Vec3d sum, sumSq;
    accumulateLab(img, sum, sumSq);
    Stats s;
    s.mean = finalizeMean(img.pixels.size(), sum);
    s.var = finalizeVar(img.pixels.size(), sum, sumSq);
    CT_DEBUG("computeLabStats: mean=" + vecToStr(s.mean) + " var=" + vecToStr(s.var));
    return s;
}

static inline double safeStd(double v)
{
    return std::sqrt(v < 1e-12 ? 1e-12 : v);
}

static Vec3d transferPixelLab(const Vec3d &lab, const Stats &src, const Stats &tgt, const std::array<bool, 3> &mask)
{
    Vec3d out = lab;
    double srcStd0 = safeStd(src.var.x), srcStd1 = safeStd(src.var.y), srcStd2 = safeStd(src.var.z);
    double tgtStd0 = safeStd(tgt.var.x), tgtStd1 = safeStd(tgt.var.y), tgtStd2 = safeStd(tgt.var.z);

    if (mask[0]) {
        out.x = (lab.x - tgt.mean.x) * (srcStd0 / tgtStd0) + src.mean.x;
    }
    if (mask[1]) {
        out.y = (lab.y - tgt.mean.y) * (srcStd1 / tgtStd1) + src.mean.y;
    }
    if (mask[2]) {
        out.z = (lab.z - tgt.mean.z) * (srcStd2 / tgtStd2) + src.mean.z;
    }
    return out;
}

static inline Bgr blendRgb(const Bgr &a, const Bgr &b, double alpha)
{
    double ar = a.r, ag = a.g, ab = a.b;
    double br = b.r, bg = b.g, bb = b.b;
    uint8_t r = clampToByte(alpha * ar + (1.0 - alpha) * br);
    uint8_t g = clampToByte(alpha * ag + (1.0 - alpha) * bg);
    uint8_t b8 = clampToByte(alpha * ab + (1.0 - alpha) * bb);
    return Bgr{b8, g, r};
}

static Bgr transferOne(const Bgr &in, const Stats &srcStats, const Stats &tgtStats, const std::array<bool, 3> &mask)
{
    const bool compress = true;
    const bool expand = true;
    Vec3d lab = rgbToLab(in, compress);
    auto safeStd = [](double v) { return std::sqrt(v < 1e-12 ? 1e-12 : v); };
    Vec3d outLab = lab;
    double srcStd0 = safeStd(srcStats.var.x), srcStd1 = safeStd(srcStats.var.y), srcStd2 = safeStd(srcStats.var.z);
    double tgtStd0 = safeStd(tgtStats.var.x), tgtStd1 = safeStd(tgtStats.var.y), tgtStd2 = safeStd(tgtStats.var.z);
    if (mask[0]) {
        outLab.x = (lab.x - tgtStats.mean.x) * (srcStd0 / tgtStd0) + srcStats.mean.x;
    }
    if (mask[1]) {
        outLab.y = (lab.y - tgtStats.mean.y) * (srcStd1 / tgtStd1) + srcStats.mean.y;
    }
    if (mask[2]) {
        outLab.z = (lab.z - tgtStats.mean.z) * (srcStd2 / tgtStd2) + srcStats.mean.z;
    }
    return labToRgb(outLab, expand);
}

Image applyColorTransferLab(const Image &target,
                            const Stats &srcStats,
                            const Stats &tgtStats,
                            const std::array<bool, 3> &channelMask)
{
    return applyColorTransferLabMasked(target, srcStats, tgtStats, channelMask, nullptr, ApplyMode::Mask, 1.0);
}

Image applyColorTransferLabMasked(const Image &target,
                                  const Stats &srcStats,
                                  const Stats &tgtStats,
                                  const std::array<bool, 3> &channelMask,
                                  const RegionMask *regionMask,
                                  ApplyMode mode,
                                  double alpha)
{
    Image out{target.width, target.height, std::vector<Bgr>(target.pixels.size())};

    const bool haveMask = (regionMask && regionMask->width == target.width && regionMask->height == target.height &&
                           !regionMask->bytes.empty());
    const size_t n = target.pixels.size();

    if (alpha < 0.0) {
        alpha = 0.0;
    }
    if (alpha > 1.0) {
        alpha = 1.0;
    }

    for (size_t i = 0; i < n; ++i) {
        if (!haveMask || regionMask->bytes[i]) {
            Bgr transferred = transferOne(target.pixels[i], srcStats, tgtStats, channelMask);
            if (haveMask && mode == ApplyMode::Blend && alpha < 1.0) {
                out.pixels[i] = blendRgb(transferred, target.pixels[i], alpha);
            } else {
                out.pixels[i] = transferred;
            }
        } else {
            out.pixels[i] = target.pixels[i];
        }
    }

#ifdef ENABLE_DEBUG_LOG
    CT_DEBUG(std::string("applyColorTransferLabMasked: mode=") + (mode == ApplyMode::Mask ? "mask" : "blend") +
             " alpha=" + std::to_string(alpha) + " haveMask=" + (haveMask ? "yes" : "no"));
#endif
    return out;
}

}  // namespace ct
