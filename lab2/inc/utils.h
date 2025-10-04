#ifndef UTILS_H
#define UTILS_H

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace ct
{

enum class Status : uint8_t {
    Ok = 0,
    IoError,
    InvalidArgument,
    Unsupported,
    ParseError,
    Overflow,
    Unknown
};

template<typename T>
struct Result {
    T value{};
    Status status{Status::Ok};
    std::string message{};
    bool ok() const
    {
        return status == Status::Ok;
    }
};

#ifdef ENABLE_DEBUG_LOG
#define CT_DEBUG(msg) ::ct::debugLog(__FILE__, __LINE__, (msg))
#else
#define CT_DEBUG(msg)                                                                                                  \
    do {                                                                                                               \
    } while (0)
#endif

void debugLog(const char *file, int line, const std::string &msg);

constexpr double clamp(double x, double lo, double hi)
{
    return x < lo ? lo : (x > hi ? hi : x);
}

constexpr uint8_t clampToByte(double x)
{
    return static_cast<uint8_t>(x < 0.0 ? 0.0 : (x > 255.0 ? 255.0 : x));
}

struct FileData {
    std::vector<uint8_t> bytes;
    Status status{Status::Ok};
    std::string message;
};

FileData readFile(const std::string &path);
Status writeFile(const std::string &path, const std::vector<uint8_t> &data, std::string &outMessage);

struct Range {
    size_t begin;
    size_t end;
};
std::vector<Range> makeChunks(size_t total, size_t targetChunkSize);

using H256 = std::array<uint32_t, 256>;

std::string toLower(std::string s);
bool endsWith(const std::string &s, const std::string &suffix);

}  // namespace ct

#endif  // UTILS_H
