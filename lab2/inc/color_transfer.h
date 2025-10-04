#ifndef COLOR_TRANSFER_H
#define COLOR_TRANSFER_H

#include "bmp.h"
#include "cli.h"
#include "color_spaces.h"
#include "region.h"
#include "utils.h"

namespace ct
{

struct Stats {
    Vec3d mean{0, 0, 0};
    Vec3d var{0, 0, 0};
};

Stats computeLabStats(const Image &img);

Image applyColorTransferLab(const Image &target,
                            const Stats &srcStats,
                            const Stats &tgtStats,
                            const std::array<bool, 3> &channelMask);

Image applyColorTransferLabMasked(const Image &target,
                                  const Stats &srcStats,
                                  const Stats &tgtStats,
                                  const std::array<bool, 3> &channelMask,
                                  const RegionMask *regionMask,
                                  ApplyMode mode,
                                  double alpha);

}  // namespace ct

#endif  // COLOR_TRANSFER_H
