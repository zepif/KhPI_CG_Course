#include "bmp.h"
#include <cstring>

namespace ct
{

#pragma pack(push, 1)
struct BmpFileHeader {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};

struct BmpInfoHeader {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};
#pragma pack(pop)

static bool is24BitUncompressed(const BmpInfoHeader &ih)
{
    return ih.biBitCount == 24 && ih.biCompression == 0;
}

Status loadBmp(const std::string &path, Image &outImage, std::string &outMessage)
{
    FileData fd = readFile(path);
    if (fd.status != Status::Ok) {
        outMessage = fd.message;
        return fd.status;
    }
    if (fd.bytes.size() < sizeof(BmpFileHeader) + sizeof(BmpInfoHeader)) {
        outMessage = "File too small for BMP headers";
        return Status::ParseError;
    }

    const auto *fh = reinterpret_cast<const BmpFileHeader *>(fd.bytes.data());
    if (fh->bfType != 0x4D42) {  // 'BM'
        outMessage = "Not a BMP file";
        return Status::Unsupported;
    }
    const auto *ih = reinterpret_cast<const BmpInfoHeader *>(fd.bytes.data() + sizeof(BmpFileHeader));
    if (!is24BitUncompressed(*ih)) {
        outMessage = "Only 24-bit uncompressed BMP supported";
        return Status::Unsupported;
    }
    if (ih->biWidth <= 0 || ih->biHeight == 0) {
        outMessage = "Invalid dimensions";
        return Status::ParseError;
    }

    uint32_t width = static_cast<uint32_t>(ih->biWidth);
    uint32_t height = static_cast<uint32_t>(std::abs(ih->biHeight));
    bool bottomUp = ih->biHeight > 0;
    size_t rowStride = ((width * 3u + 3u) / 4u) * 4u;

    size_t dataOffset = fh->bfOffBits;
    if (dataOffset + rowStride * height > fd.bytes.size()) {
        outMessage = "Pixel data outside file";
        return Status::ParseError;
    }
    outImage.width = width;
    outImage.height = height;
    outImage.pixels.assign(width * height, Bgr{});

    for (uint32_t y = 0; y < height; ++y) {
        size_t srcY = bottomUp ? (height - 1 - y) : y;
        const uint8_t *row = fd.bytes.data() + dataOffset + srcY * rowStride;
        for (uint32_t x = 0; x < width; ++x) {
            size_t idx = y * width + x;
            outImage.pixels[idx].b = row[x * 3 + 0];
            outImage.pixels[idx].g = row[x * 3 + 1];
            outImage.pixels[idx].r = row[x * 3 + 2];
        }
    }
    return Status::Ok;
}

Status saveBmp(const std::string &path, const Image &img, std::string &outMessage)
{
    uint32_t width = img.width;
    uint32_t height = img.height;
    size_t rowStride = ((width * 3u + 3u) / 4u) * 4u;
    size_t pixelBytes = rowStride * height;
    BmpFileHeader fh{};
    BmpInfoHeader ih{};
    fh.bfType = 0x4D42;
    fh.bfOffBits = sizeof(BmpFileHeader) + sizeof(BmpInfoHeader);
    fh.bfSize = static_cast<uint32_t>(fh.bfOffBits + pixelBytes);

    ih.biSize = sizeof(BmpInfoHeader);
    ih.biWidth = static_cast<int32_t>(width);
    ih.biHeight = static_cast<int32_t>(height);  // bottom-up
    ih.biPlanes = 1;
    ih.biBitCount = 24;
    ih.biCompression = 0;
    ih.biSizeImage = static_cast<uint32_t>(pixelBytes);

    std::vector<uint8_t> out;
    out.resize(fh.bfOffBits + pixelBytes);
    std::memcpy(out.data(), &fh, sizeof(fh));
    std::memcpy(out.data() + sizeof(fh), &ih, sizeof(ih));

    for (uint32_t y = 0; y < height; ++y) {
        uint32_t srcY = height - 1 - y;
        uint8_t *row = out.data() + fh.bfOffBits + y * rowStride;
        for (uint32_t x = 0; x < width; ++x) {
            const Bgr &p = img.pixels[srcY * width + x];
            row[x * 3 + 0] = p.b;
            row[x * 3 + 1] = p.g;
            row[x * 3 + 2] = p.r;
        }
        for (size_t pad = width * 3; pad < rowStride; ++pad) {
            row[pad] = 0;
        }
    }

    return writeFile(path, out, outMessage);
}

}  // namespace ct
