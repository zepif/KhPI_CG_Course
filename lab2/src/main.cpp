#include "bmp.h"
#include "cli.h"
#include "color_transfer.h"
#include "histogram.h"
#include "region.h"
#include "utils.h"
#include <cstdio>

using namespace ct;

int main(int argc, char **argv)
{
    auto parsed = parseCli(argc, argv);
    if (!parsed.ok()) {
        printUsage();
        std::fprintf(stderr, "Error: %s\n", parsed.message.c_str());
        return 1;
    }
    const CliOptions &opt = parsed.value;

    Image srcImg, tgtImg;
    std::string msg;
    if (loadBmp(opt.srcPath, srcImg, msg) != Status::Ok) {
        std::fprintf(stderr, "Source load error: %s\n", msg.c_str());
        return 2;
    }
    if (loadBmp(opt.tgtPath, tgtImg, msg) != Status::Ok) {
        std::fprintf(stderr, "Target load error: %s\n", msg.c_str());
        return 3;
    }

    Stats srcStats = computeLabStats(srcImg);
    Stats tgtStats = computeLabStats(tgtImg);

    RegionMask mask;
    RegionMask *maskPtr = nullptr;
    if (!opt.brushes.empty()) {
        mask = makeEmptyMask(tgtImg.width, tgtImg.height);
        applyBrushes(mask, opt.brushes);
        maskPtr = &mask;
        if (!opt.saveMaskPath.empty()) {
            saveMaskBmp(opt.saveMaskPath, mask, msg);
        }
    }

    Image resImg =
        applyColorTransferLabMasked(tgtImg, srcStats, tgtStats, opt.labMask, maskPtr, opt.applyMode, opt.alpha);

    if (saveBmp(opt.outPrefix + "_result.bmp", resImg, msg) != Status::Ok) {
        std::fprintf(stderr, "Save result error: %s\n", msg.c_str());
        return 4;
    }

    H256 r, g, b;
    buildRgbHistograms(srcImg, r, g, b);
    saveHistogramCsv(opt.outPrefix + "_src_r.csv", r, msg);
    saveHistogramCsv(opt.outPrefix + "_src_g.csv", g, msg);
    saveHistogramCsv(opt.outPrefix + "_src_b.csv", b, msg);

    buildRgbHistograms(tgtImg, r, g, b);
    saveHistogramCsv(opt.outPrefix + "_tgt_r.csv", r, msg);
    saveHistogramCsv(opt.outPrefix + "_tgt_g.csv", g, msg);
    saveHistogramCsv(opt.outPrefix + "_tgt_b.csv", b, msg);

    buildRgbHistograms(resImg, r, g, b);
    saveHistogramCsv(opt.outPrefix + "_res_r.csv", r, msg);
    saveHistogramCsv(opt.outPrefix + "_res_g.csv", g, msg);
    saveHistogramCsv(opt.outPrefix + "_res_b.csv", b, msg);

#ifdef ENABLE_DEBUG_LOG
    {
        Bgr mid{128, 128, 128};
        auto lab = rgbToLab(mid, true);
        auto mid2 = labToRgb(lab, true);
        CT_DEBUG(std::string("roundtrip gray 128 -> (") + std::to_string(mid2.r) + "," + std::to_string(mid2.g) + "," +
                 std::to_string(mid2.b) + ")");
    }
#endif

    std::printf("Done. Output prefix: %s\n", opt.outPrefix.c_str());
    return 0;
}
