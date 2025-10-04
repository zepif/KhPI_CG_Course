#include "cli.h"
#include <cstdio>
#include <cstdlib>

namespace ct
{

static bool parseInt(const char *s, int &out)
{
    if (!s) {
        return false;
    }
    char *end = nullptr;
    long v = std::strtol(s, &end, 10);
    if (!end || *end != '\0') {
        return false;
    }
    out = (int)v;
    return true;
}

static bool parseDouble(const char *s, double &out)
{
    if (!s) {
        return false;
    }
    char *end = nullptr;
    double v = std::strtod(s, &end);
    if (!end || *end != '\0') {
        return false;
    }
    out = v;
    return true;
}

static std::array<bool, 3> parseLabMask(const char *s)
{
    std::array<bool, 3> m{true, true, true};
    if (!s) {
        return m;
    }
    std::string str(s);
    if (str.size() == 3) {
        m[0] = (str[0] == '1');
        m[1] = (str[1] == '1');
        m[2] = (str[2] == '1');
    }
    return m;
}

void printUsage()
{
    std::printf("Usage:\n");
    std::printf("  color_transfer <source.bmp> <target.bmp> <out_prefix> [labMask]\n");
    std::printf("  Optional brushes and options:\n");
    std::printf("    --rect  x y w h         (add rectangular brush)\n");
    std::printf("    --circle cx cy r        (add circular brush)\n");
    std::printf("    --mode  mask|blend      (how to apply inside region; default mask)\n");
    std::printf("    --alpha A               (0..1, for blend mode; default 1.0)\n");
    std::printf("    --save-mask path.bmp    (save region mask as BMP)\n");
    std::printf("\nExamples:\n");
    std::printf("  color_transfer a.bmp b.bmp out 111 --rect 10 10 60 40 --circle 100 70 25 --mode blend --alpha 0.6 "
                "--save-mask out_mask.bmp\n");
}

Result<CliOptions> parseCli(int argc, char **argv)
{
    Result<CliOptions> r;
    if (argc < 4) {
        r.status = Status::InvalidArgument;
        r.message = "Not enough args";
        return r;
    }
    CliOptions opt;
    opt.srcPath = argv[1];
    opt.tgtPath = argv[2];
    opt.outPrefix = argv[3];
    opt.labMask = parseLabMask(argc >= 5 ? argv[4] : nullptr);

    int i = 5;
    while (i < argc) {
        std::string a = argv[i];

        if (a == "--rect") {
            if (i + 4 >= argc) {
                r.status = Status::InvalidArgument;
                r.message = "--rect needs 4 ints";
                return r;
            }
            Brush b;
            b.type = BrushType::Rect;
            if (!parseInt(argv[i + 1], b.x) || !parseInt(argv[i + 2], b.y) || !parseInt(argv[i + 3], b.w) ||
                !parseInt(argv[i + 4], b.h)) {
                r.status = Status::InvalidArgument;
                r.message = "invalid --rect numbers";
                return r;
            }
            opt.brushes.push_back(b);
            i += 5;
            continue;
        }
        if (a == "--circle") {
            if (i + 3 >= argc) {
                r.status = Status::InvalidArgument;
                r.message = "--circle needs 3 ints";
                return r;
            }
            Brush b;
            b.type = BrushType::Circle;
            if (!parseInt(argv[i + 1], b.cx) || !parseInt(argv[i + 2], b.cy) || !parseInt(argv[i + 3], b.r)) {
                r.status = Status::InvalidArgument;
                r.message = "invalid --circle numbers";
                return r;
            }
            opt.brushes.push_back(b);
            i += 4;
            continue;
        }
        if (a == "--mode") {
            if (i + 1 >= argc) {
                r.status = Status::InvalidArgument;
                r.message = "--mode needs arg";
                return r;
            }
            std::string m = argv[i + 1];
            if (m == "mask") {
                opt.applyMode = ApplyMode::Mask;
            } else if (m == "blend") {
                opt.applyMode = ApplyMode::Blend;
            } else {
                r.status = Status::InvalidArgument;
                r.message = "mode must be mask|blend";
                return r;
            }
            i += 2;
            continue;
        }
        if (a == "--alpha") {
            if (i + 1 >= argc) {
                r.status = Status::InvalidArgument;
                r.message = "--alpha needs value";
                return r;
            }
            double A = 1.0;
            if (!parseDouble(argv[i + 1], A) || A < 0.0 || A > 1.0) {
                r.status = Status::InvalidArgument;
                r.message = "alpha must be 0..1";
                return r;
            }
            opt.alpha = A;
            i += 2;
            continue;
        }
        if (a == "--save-mask") {
            if (i + 1 >= argc) {
                r.status = Status::InvalidArgument;
                r.message = "--save-mask needs path";
                return r;
            }
            opt.saveMaskPath = argv[i + 1];
            i += 2;
            continue;
        }

        r.status = Status::InvalidArgument;
        r.message = "Unknown argument: " + a;
        return r;
    }

    r.value = opt;
    r.status = Status::Ok;
    return r;
}

}  // namespace ct
