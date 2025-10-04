#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "bmp.h"
#include "utils.h"

namespace ct
{

struct ChannelImages {
    Image ch0;
    Image ch1;
    Image ch2;
};

void buildRgbHistograms(const Image &img, H256 &r, H256 &g, H256 &b);

Status saveHistogramCsv(const std::string &path, const H256 &h, std::string &outMessage);

ChannelImages makeRgbChannelViews(const Image &img);

}  // namespace ct

#endif  // HISTOGRAM_H
