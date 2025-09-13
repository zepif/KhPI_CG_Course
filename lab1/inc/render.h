#pragma once
#include "automaton.h"
#include <windows.h>

class Renderer
{
public:
    Renderer();

    void Paint(HDC hdc, const RECT &drawRc, const Automaton &a, COLORREF c0, COLORREF c1, bool showGrid) const;

    bool SaveGridBmp(const Automaton &a, const wchar_t *path, int scale, COLORREF c0, COLORREF c1) const;
};
