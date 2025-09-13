#pragma once
#include "automaton.h"
#include "config.h"
#include "render.h"
#include "ui.h"
#include <string>
#include <windows.h>

class App
{
public:
    App();
    bool Init(HINSTANCE inst, int nCmdShow);
    int Run();

private:
    static LRESULT CALLBACK StaticWndProc(HWND h, UINT m, WPARAM w, LPARAM l);
    LRESULT WndProc(HWND h, UINT m, WPARAM w, LPARAM l);

    void OnCreate();
    void OnSize();
    void OnPaint();
    void OnTimer();
    void OnCommand(WORD id, WORD code);
    void OnMouseDown(UINT msg, LPARAM lParam);
    void OnMouseMove(LPARAM lParam);
    void OnMouseUp();

    void ToggleRun(bool Run);
    void UpdateTitle() const;
    void ApplyRuleFromEdit();
    void SaveBmpDialog();

    bool ScreenToCell(int sx, int sy, int &gx, int &gy) const;

private:
    HINSTANCE inst;
    HWND hwnd;
    HFONT uiFont;

    RECT drawRc;

    Automaton automaton;
    Renderer renderer;
    Ui ui;

    bool running;
    bool showGrid;

    COLORREF color0;
    COLORREF color1;
    UINT tickMs;

    bool painting;
    uint8_t paintVal;
    int lastGx;
    int lastGy;
};
