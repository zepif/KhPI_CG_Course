#include "render.h"
#include "config.h"
#include "utils.h"
#include <vector>

Renderer::Renderer() = default;

void Renderer::Paint(HDC hdc, const RECT &drawRc, const Automaton &a, COLORREF c0, COLORREF c1, bool showGrid) const
{
    const int srcW = a.Width();
    const int srcH = a.Height();
    if (srcW <= 0 || srcH <= 0) {
        return;
    }

    std::vector<uint32_t> pix(static_cast<size_t>(srcW) * srcH);

    const uint8_t r0 = GetRValue(c0), g0 = GetGValue(c0), b0 = GetBValue(c0);
    const uint8_t r1 = GetRValue(c1), g1 = GetGValue(c1), b1 = GetBValue(c1);

    Utils::ForAllCells(a, [&](int x, int y, uint8_t s) {
        const uint8_t r = s ? r1 : r0;
        const uint8_t g = s ? g1 : g0;
        const uint8_t b = s ? b1 : b0;
        pix[static_cast<size_t>(y) * srcW + x] =
            (uint32_t(b)) | (uint32_t(g) << 8) | (uint32_t(r) << 16) | 0xFF000000u;
    });

    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = srcW;
    bmi.bmiHeader.biHeight = -srcH;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    const int dstW = drawRc.right - drawRc.left;
    const int dstH = drawRc.bottom - drawRc.top;

    SetStretchBltMode(hdc, COLORONCOLOR);
    StretchDIBits(hdc,
                  drawRc.left, drawRc.top, dstW, dstH,
                  0, 0, srcW, srcH,
                  pix.data(), &bmi, DIB_RGB_COLORS, SRCCOPY);

    if (showGrid) {
        HPEN pen = CreatePen(PS_SOLID, 1, Cfg::Render::GRID_LINE_COLOR);
        HGDIOBJ old = SelectObject(hdc, pen);
        for (int x = 1; x < srcW; ++x) {
            int sx = drawRc.left + static_cast<int>(static_cast<long long>(x) * dstW / srcW);
            MoveToEx(hdc, sx, drawRc.top, nullptr);
            LineTo(hdc, sx, drawRc.bottom);
        }
        for (int y = 1; y < srcH; ++y) {
            int sy = drawRc.top + static_cast<int>(static_cast<long long>(y) * dstH / srcH);
            MoveToEx(hdc, drawRc.left, sy, nullptr);
            LineTo(hdc, drawRc.right, sy);
        }
        SelectObject(hdc, old);
        DeleteObject(pen);
    }
}

bool Renderer::SaveGridBmp(const Automaton &a, const wchar_t *path, int scale, COLORREF c0, COLORREF c1) const
{
    if (scale < 1) {
        scale = 1;
    }
    const int outW = a.Width() * scale;
    const int outH = a.Height() * scale;

    const int rowStride =
        ((outW * 3 + (Cfg::Io::BMP_ROW_ALIGN - 1)) / Cfg::Io::BMP_ROW_ALIGN) * Cfg::Io::BMP_ROW_ALIGN;
    const size_t dataSize = static_cast<size_t>(rowStride) * outH;
    std::vector<uint8_t> pixels(dataSize, 0);

    auto B = [](COLORREF c) { return static_cast<uint8_t>((c) & 0xFF); };
    auto G = [](COLORREF c) { return static_cast<uint8_t>(((c) >> 8) & 0xFF); };
    auto R = [](COLORREF c) { return static_cast<uint8_t>(((c) >> 16) & 0xFF); };

    Utils::ForAllPixels(a, scale, [&](int ox, int oyTop, int /*gx*/, int /*gy*/, uint8_t s) {
        const COLORREF c = s ? c1 : c0;
        const int oy = (outH - 1) - oyTop;
        uint8_t *row = pixels.data() + static_cast<size_t>(oy) * rowStride;
        uint8_t *p = row + ox * 3;
        p[0] = B(c);
        p[1] = G(c);
        p[2] = R(c);
    });

    BITMAPFILEHEADER bfh{};
    BITMAPINFOHEADER bih{};
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = outW;
    bih.biHeight = outH;
    bih.biPlanes = 1;
    bih.biBitCount = Cfg::Io::BMP_BPP;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = static_cast<DWORD>(dataSize);

    bfh.bfType = Cfg::Io::BMP_SIG_BM;
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bfh.bfSize = bfh.bfOffBits + static_cast<DWORD>(dataSize);

    HANDLE hFile = CreateFileW(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD written = 0;
    BOOL ok = TRUE;
    ok = ok && WriteFile(hFile, &bfh, sizeof(bfh), &written, nullptr);
    ok = ok && WriteFile(hFile, &bih, sizeof(bih), &written, nullptr);
    ok = ok && WriteFile(hFile, pixels.data(), static_cast<DWORD>(pixels.size()), &written, nullptr);
    CloseHandle(hFile);
    return ok == TRUE;
}
