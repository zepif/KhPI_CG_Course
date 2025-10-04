#include "utils.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>

namespace ct
{

void debugLog(const char *file, int line, const std::string &msg)
{
#ifdef ENABLE_DEBUG_LOG
    std::fprintf(stderr, "[DEBUG] %s:%d %s\n", file, line, msg.c_str());
#else
    (void)file;
    (void)line;
    (void)msg;
#endif
}

FileData readFile(const std::string &path)
{
    FileData fd;
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) {
        fd.status = Status::IoError;
        fd.message = "Failed to open file: " + path;
        return fd;
    }
    ifs.seekg(0, std::ios::end);
    std::streampos end = ifs.tellg();
    if (end < 0) {
        fd.status = Status::IoError;
        fd.message = "tellg failed";
        return fd;
    }
    fd.bytes.resize(static_cast<size_t>(end));
    ifs.seekg(0, std::ios::beg);
    if (!ifs.read(reinterpret_cast<char *>(fd.bytes.data()), fd.bytes.size())) {
        fd.status = Status::IoError;
        fd.message = "read failed";
        return fd;
    }
    fd.status = Status::Ok;
    return fd;
}

Status writeFile(const std::string &path, const std::vector<uint8_t> &data, std::string &outMessage)
{
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) {
        outMessage = "Failed to open for write: " + path;
        return Status::IoError;
    }
    ofs.write(reinterpret_cast<const char *>(data.data()), data.size());
    if (!ofs.good()) {
        outMessage = "Write failed: " + path;
        return Status::IoError;
    }
    return Status::Ok;
}

std::vector<Range> makeChunks(size_t total, size_t targetChunkSize)
{
    std::vector<Range> out;
    if (targetChunkSize == 0) {
        targetChunkSize = 1024;
    }
    for (size_t i = 0; i < total; i += targetChunkSize) {
        size_t end = i + targetChunkSize;
        if (end > total) {
            end = total;
        }
        out.push_back(Range{i, end});
    }
    return out;
}

std::string toLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return char(std::tolower(c)); });
    return s;
}

bool endsWith(const std::string &s, const std::string &suffix)
{
    if (suffix.size() > s.size()) {
        return false;
    }
    return std::equal(suffix.rbegin(), suffix.rend(), s.rbegin());
}

}  // namespace ct
