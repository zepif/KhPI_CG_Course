#include "histogram.h"

namespace ct
{

void buildRgbHistograms(const Image &img, H256 &r, H256 &g, H256 &b)
{
    r.fill(0);
    g.fill(0);
    b.fill(0);
    for (const auto &p : img.pixels) {
        r[p.r]++;
        g[p.g]++;
        b[p.b]++;
    }
}

Status saveHistogramCsv(const std::string &path, const H256 &h, std::string &outMessage)
{
    std::vector<uint8_t> data;
    std::string s;
    s.reserve(4096);
    for (int i = 0; i < 256; ++i) {
        s += std::to_string(i);
        s += ",";
        s += std::to_string(h[i]);
        s += "\n";
    }
    data.assign(s.begin(), s.end());
    return writeFile(path, data, outMessage);
}

ChannelImages makeRgbChannelViews(const Image &img)
{
    ChannelImages out;
    out.ch0 = {img.width, img.height, std::vector<Bgr>(img.pixels.size())};
    out.ch1 = out.ch0;
    out.ch2 = out.ch0;
    for (size_t i = 0; i < img.pixels.size(); ++i) {
        uint8_t r = img.pixels[i].r;
        uint8_t g = img.pixels[i].g;
        uint8_t b = img.pixels[i].b;
        out.ch0.pixels[i] = Bgr{r, r, r};
        out.ch1.pixels[i] = Bgr{g, g, g};
        out.ch2.pixels[i] = Bgr{b, b, b};
    }
    return out;
}

}  // namespace ct
