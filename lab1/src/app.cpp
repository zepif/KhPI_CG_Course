#include "app.h"
#include "config.h"
#include "utils.h"

#include <algorithm>
#include <commdlg.h>
#include <cwctype>
#include <string>
#include <windowsx.h>

namespace
{
const wchar_t *kClassName = L"CrystaliWnd";
}

App::App()
    : inst(nullptr),
      hwnd(nullptr),
      uiFont(nullptr),
      drawRc{0, 0, 0, 0},
      running(false),
      showGrid(false),
      color0(Cfg::Render::COLOR0),
      color1(Cfg::Render::COLOR1),
      tickMs(Cfg::Timer::DEFAULT_TICK_MS),
      painting(false),
      paintVal(1),
      lastGx(-1),
      lastGy(-1)
{
}

bool App::Init(HINSTANCE inst, int nCmdShow)
{
    this->inst = inst;

    WNDCLASSEXW wc{sizeof(wc)};
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = &App::StaticWndProc;
    wc.hInstance = this->inst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = kClassName;
    wc.hIconSm = wc.hIcon;
    if (!RegisterClassExW(&wc)) {
        return false;
    }

    hwnd = CreateWindowExW(0, kClassName, L"Crystali", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT,
                           CW_USEDEFAULT, Cfg::Window::WIDTH, Cfg::Window::HEIGHT, nullptr, nullptr, this->inst, this);
    if (!hwnd) {
        return false;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    return true;
}

int App::Run()
{
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}

LRESULT CALLBACK App::StaticWndProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    if (m == WM_NCCREATE) {
        auto cs = reinterpret_cast<CREATESTRUCTW *>(l);
        auto *that = static_cast<App *>(cs->lpCreateParams);
        SetWindowLongPtrW(h, GWLP_USERDATA, (LONG_PTR)that);
        that->hwnd = h;
    }
    auto *that = reinterpret_cast<App *>(GetWindowLongPtrW(h, GWLP_USERDATA));
    if (!that) {
        return DefWindowProcW(h, m, w, l);
    }
    return that->WndProc(h, m, w, l);
}

LRESULT App::WndProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    switch (m) {
        case WM_CREATE:
            OnCreate();
            return 0;
        case WM_SIZE:
            OnSize();
            InvalidateRect(h, nullptr, TRUE);
            return 0;
        case WM_PAINT:
            OnPaint();
            return 0;
        case WM_TIMER:
            OnTimer();
            return 0;
        case WM_COMMAND:
            OnCommand(LOWORD(w), HIWORD(w));
            return 0;
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
            OnMouseDown(m, l);
            return 0;
        case WM_MOUSEMOVE:
            OnMouseMove(l);
            return 0;
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            OnMouseUp();
            return 0;
        case WM_CLOSE:
            KillTimer(h, Cfg::Timer::ID);
            DestroyWindow(h);
            return 0;
        case WM_DESTROY:
            if (uiFont) {
                DeleteObject(uiFont);
            }
            PostQuitMessage(0);
            return 0;
        default:
            break;
    }
    return DefWindowProcW(h, m, w, l);
}

void App::OnCreate()
{
    NONCLIENTMETRICSW ncm{sizeof(ncm)};
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
    uiFont = CreateFontIndirectW(&ncm.lfMessageFont);

    ui.Create(hwnd, inst);
    ui.SetFont(uiFont);
    ui.SetWrapChecked(true);
    ui.SetGridChecked(false);

    OnSize();

    automaton.Resize(Cfg::Automaton::DEFAULT_W, Cfg::Automaton::DEFAULT_H);
    automaton.SetWrap(true);
    automaton.SetRuleBits(Cfg::Automaton::DEFAULT_RULE);

    UpdateTitle();
}

void App::OnSize()
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    ui.Layout(hwnd);
    drawRc.left = 0;
    drawRc.top = Cfg::Ui::PANEL_HEIGHT;
    drawRc.right = rc.right;
    drawRc.bottom = rc.bottom;
}

void App::OnPaint()
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    HBRUSH br = CreateSolidBrush(Cfg::Render::PANEL_BG_COLOR);
    RECT rcPanel{0, 0, 0, Cfg::Ui::PANEL_HEIGHT};
    GetClientRect(hwnd, &rcPanel);
    rcPanel.bottom = Cfg::Ui::PANEL_HEIGHT;
    FillRect(hdc, &rcPanel, br);
    DeleteObject(br);

    HPEN pen = CreatePen(PS_SOLID, 1, Cfg::Render::PANEL_LINE_COLOR);
    HGDIOBJ old = SelectObject(hdc, pen);
    MoveToEx(hdc, 0, Cfg::Ui::PANEL_HEIGHT - 1, nullptr);
    LineTo(hdc, rcPanel.right, Cfg::Ui::PANEL_HEIGHT - 1);
    SelectObject(hdc, old);
    DeleteObject(pen);


    renderer.Paint(hdc, drawRc, automaton, color0, color1, showGrid);
    EndPaint(hwnd, &ps);
}

void App::OnTimer()
{
    if (!running) {
        return;
    }
    automaton.Step();
    UpdateTitle();
    InvalidateRect(hwnd, &drawRc, FALSE);
}

void App::OnCommand(WORD id, WORD code)
{
    switch ((CtrlId)id) {
        case CtrlId::RULE_APPLY:
            ApplyRuleFromEdit();
            break;
        case CtrlId::START:
            ToggleRun(!running);
            break;
        case CtrlId::STEP:
            if (!running) {
                automaton.Step();
                UpdateTitle();
                InvalidateRect(hwnd, &drawRc, FALSE);
            }
            break;
        case CtrlId::RESET:
            if (running) {
                ToggleRun(false);
            }
            automaton.ResetToInit();
            UpdateTitle();
            InvalidateRect(hwnd, &drawRc, FALSE);
            break;
        case CtrlId::SET_INIT:
            automaton.SetInitFromCurrent();
            MessageBoxW(hwnd, L"Текущий состояние сохранено как начальное.", L"Set Init", MB_OK | MB_ICONINFORMATION);
            break;
        case CtrlId::RANDOMIZE:
            if (running) {
                ToggleRun(false);
            }
            automaton.Randomize(0.5);
            UpdateTitle();
            InvalidateRect(hwnd, &drawRc, FALSE);
            break;
        case CtrlId::CLEAR:
            if (running) {
                ToggleRun(false);
            }
            automaton.Clear();
            UpdateTitle();
            InvalidateRect(hwnd, &drawRc, FALSE);
            break;
        case CtrlId::SAVE_BMP:
            SaveBmpDialog();
            break;
        case CtrlId::SPEED:
            if (code == CBN_SELCHANGE) {
                tickMs = (UINT)ui.SpeedMs();
                if (running) {
                    KillTimer(hwnd, Cfg::Timer::ID);
                    SetTimer(hwnd, Cfg::Timer::ID, tickMs, nullptr);
                }
            }
            break;
        case CtrlId::WRAP:
            automaton.SetWrap(ui.WrapChecked());
            UpdateTitle();
            break;
        case CtrlId::GRID:
            showGrid = ui.GridChecked();
            UpdateTitle();
            InvalidateRect(hwnd, &drawRc, FALSE);
            break;
        default:
            break;
    }
}

void App::OnMouseDown(UINT msg, LPARAM lParam)
{
    if (running) {
        return;
    }
    SetCapture(hwnd);
    painting = true;
    paintVal = (msg == WM_LBUTTONDOWN) ? 1u : 0u;
    const int sx = GET_X_LPARAM(lParam);
    const int sy = GET_Y_LPARAM(lParam);
    int gx, gy;
    if (ScreenToCell(sx, sy, gx, gy)) {
        if (gx != lastGx || gy != lastGy) {
            automaton.Set(gx, gy, paintVal);
            lastGx = gx;
            lastGy = gy;
            InvalidateRect(hwnd, &drawRc, FALSE);
        }
    }
}

void App::OnMouseMove(LPARAM lParam)
{
    if (!painting) {
        return;
    }
    const int sx = GET_X_LPARAM(lParam);
    const int sy = GET_Y_LPARAM(lParam);
    int gx, gy;
    if (ScreenToCell(sx, sy, gx, gy)) {
        if (gx != lastGx || gy != lastGy) {
            automaton.Set(gx, gy, paintVal);
            lastGx = gx;
            lastGy = gy;
            InvalidateRect(hwnd, &drawRc, FALSE);
        }
    }
}

void App::OnMouseUp()
{
    if (!painting) {
        return;
    }
    painting = false;
    lastGx = lastGy = -1;
    ReleaseCapture();
}

void App::ToggleRun(bool run)
{
    running = run;
    if (running) {
        tickMs = (UINT)ui.SpeedMs();
        SetTimer(hwnd, Cfg::Timer::ID, tickMs, nullptr);
    } else {
        KillTimer(hwnd, Cfg::Timer::ID);
    }
    ui.SetStartCaption(running);
    UpdateTitle();
}

void App::UpdateTitle() const
{
    wchar_t buf[256];
    swprintf(buf, 256, L"Crystali — Rule %u — Iteration %u%s%s", (unsigned)automaton.RuleBits(),
             (unsigned)automaton.Iteration(), automaton.Wrap() ? L" — Wrap" : L"", showGrid ? L" — Grid" : L"");
    SetWindowTextW(hwnd, buf);
}

void App::ApplyRuleFromEdit()
{
    std::wstring s = ui.RuleText();
    s.erase(std::remove_if(s.begin(), s.end(), [](wchar_t ch) { return iswspace(ch); }), s.end());

    bool ok = false;
    uint16_t val = 0;
    if (!s.empty()) {
        const bool bin = std::all_of(s.begin(), s.end(), [](wchar_t ch) { return ch == L'0' || ch == L'1'; });
        if (bin && s.size() == Cfg::Automaton::RULE_BITS_COUNT) {
            val = 0;
            for (wchar_t ch : s) {
                val = static_cast<uint16_t>((val << 1) | (ch == L'1' ? 1 : 0));
            }
            ok = true;
        } else {
            try {
                int dec = std::stoi(s);
                if (dec >= 0 && dec <= ((1 << Cfg::Automaton::RULE_BITS_COUNT) - 1)) {
                    val = static_cast<uint16_t>(dec);
                    ok = true;
                }
            } catch (...) {
            }
        }
    }

    if (ok) {
        automaton.SetRuleBits(val);
        UpdateTitle();
        InvalidateRect(hwnd, &drawRc, FALSE);
    } else {
        MessageBoxW(hwnd, L"Введите 0–1023 или 10-битную строку, например 0100011110.", L"Некорректное правило",
                    MB_ICONWARNING);
    }
}

void App::SaveBmpDialog()
{
    wchar_t file[MAX_PATH] = L"crystali.bmp";
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"BMP Files (*.bmp)\0*.bmp\0All Files\0*.*\0";
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = L"bmp";

    if (GetSaveFileNameW(&ofn)) {
        if (!renderer.SaveGridBmp(automaton, file, Cfg::Io::BMP_SCALE, color0, color1)) {
            MessageBoxW(hwnd, L"Не удалось сохранить BMP.", L"Ошибка", MB_ICONERROR);
        }
    }
}

bool App::ScreenToCell(int sx, int sy, int &gx, int &gy) const
{
    if (sx < drawRc.left || sx >= drawRc.right || sy < drawRc.top || sy >= drawRc.bottom) {
        return false;
    }
    const int drawW = drawRc.right - drawRc.left;
    const int drawH = drawRc.bottom - drawRc.top;
    const int x = sx - drawRc.left;
    const int y = sy - drawRc.top;
    gx = (int)((long long)x * automaton.Width() / std::max(1, drawW));
    gy = (int)((long long)y * automaton.Height() / std::max(1, drawH));
    if (gx < 0) {
        gx = 0;
    }
    if (gx >= automaton.Width()) {
        gx = automaton.Width() - 1;
    }
    if (gy < 0) {
        gy = 0;
    }
    if (gy >= automaton.Height()) {
        gy = automaton.Height() - 1;
    }
    return true;
}
