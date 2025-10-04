#ifndef BMP_H
#define BMP_H

#include "utils.h"
#include <cstdint>
#include <string>
#include <vector>

namespace ct
{

struct Bgr {
    uint8_t b{0}, g{0}, r{0};
};

struct Image {
    uint32_t width{0};
    uint32_t height{0};
    std::vector<Bgr> pixels;
};

Status loadBmp(const std::string &path, Image &outImage, std::string &outMessage);
Status saveBmp(const std::string &path, const Image &img, std::string &outMessage);

}  // namespace ct

#endif  // BMP_H
