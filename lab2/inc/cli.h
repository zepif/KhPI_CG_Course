#ifndef CLI_H
#define CLI_H

#include "region.h"
#include "utils.h"
#include <cstdint>
#include <string>
#include <vector>

namespace ct
{

enum class ApplyMode : uint8_t {
    Mask = 0,
    Blend = 1
};

struct CliOptions {
    std::string srcPath;
    std::string tgtPath;
    std::string outPrefix;
    std::array<bool, 3> labMask{true, true, true};

    std::vector<Brush> brushes;
    ApplyMode applyMode{ApplyMode::Mask};
    double alpha{1.0};

    std::string saveMaskPath;
};

Result<CliOptions> parseCli(int argc, char **argv);

void printUsage();

}  // namespace ct

#endif  // CLI_H
