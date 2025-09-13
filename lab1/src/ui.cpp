#include "ui.h"
#include <windowsx.h>

Ui::Ui()
    : hRuleEdit(nullptr),
      hRuleApply(nullptr),
      hStart(nullptr),
      hStep(nullptr),
      hReset(nullptr),
      hSetInit(nullptr),
      hRandom(nullptr),
      hClear(nullptr),
      hSave(nullptr),
      hSpeed(nullptr),
      hWrap(nullptr),
      hGrid(nullptr),
      hSpeedLabel(nullptr)
{
}

int Ui::PlaceCtl(HWND hwnd, int x, int y, int w, int h)
{
    SetWindowPos(hwnd, nullptr, x, y, w, h, SWP_NOZORDER);
    return x + w + Cfg::Ui::SPACING_X;
}

void Ui::Create(HWND parent, HINSTANCE inst)
{
    const int y = Cfg::Ui::MARGIN_Y;

    hRuleEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"286", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                Cfg::Ui::MARGIN_X, y, Cfg::Ui::RULE_EDIT_WIDTH, Cfg::Ui::CTL_HEIGHT, parent,
                                (HMENU)(int)CtrlId::RULE_EDIT, inst, nullptr);

    hRuleApply = CreateWindowW(L"BUTTON", L"Apply", WS_CHILD | WS_VISIBLE, 0, y, Cfg::Ui::APPLY_BTN_WIDTH,
                               Cfg::Ui::CTL_HEIGHT, parent, (HMENU)(int)CtrlId::RULE_APPLY, inst, nullptr);

    hStart = CreateWindowW(L"BUTTON", L"Start", WS_CHILD | WS_VISIBLE, 0, y, Cfg::Ui::START_BTN_WIDTH,
                           Cfg::Ui::CTL_HEIGHT, parent, (HMENU)(int)CtrlId::START, inst, nullptr);

    hStep = CreateWindowW(L"BUTTON", L"Step", WS_CHILD | WS_VISIBLE, 0, y, Cfg::Ui::STEP_BTN_WIDTH, Cfg::Ui::CTL_HEIGHT,
                          parent, (HMENU)(int)CtrlId::STEP, inst, nullptr);

    hReset = CreateWindowW(L"BUTTON", L"Reset", WS_CHILD | WS_VISIBLE, 0, y, Cfg::Ui::RESET_BTN_WIDTH,
                           Cfg::Ui::CTL_HEIGHT, parent, (HMENU)(int)CtrlId::RESET, inst, nullptr);

    hSetInit = CreateWindowW(L"BUTTON", L"Set Init", WS_CHILD | WS_VISIBLE, 0, y, Cfg::Ui::SET_INIT_BTN_WIDTH,
                             Cfg::Ui::CTL_HEIGHT, parent, (HMENU)(int)CtrlId::SET_INIT, inst, nullptr);

    hRandom = CreateWindowW(L"BUTTON", L"Random", WS_CHILD | WS_VISIBLE, 0, y, Cfg::Ui::RANDOM_BTN_WIDTH,
                            Cfg::Ui::CTL_HEIGHT, parent, (HMENU)(int)CtrlId::RANDOMIZE, inst, nullptr);

    hClear = CreateWindowW(L"BUTTON", L"Clear", WS_CHILD | WS_VISIBLE, 0, y, Cfg::Ui::CLEAR_BTN_WIDTH,
                           Cfg::Ui::CTL_HEIGHT, parent, (HMENU)(int)CtrlId::CLEAR, inst, nullptr);

    hSave = CreateWindowW(L"BUTTON", L"Save BMP", WS_CHILD | WS_VISIBLE, 0, y, Cfg::Ui::SAVE_BTN_WIDTH,
                          Cfg::Ui::CTL_HEIGHT, parent, (HMENU)(int)CtrlId::SAVE_BMP, inst, nullptr);

    hSpeedLabel = CreateWindowW(L"STATIC", L"Speed:", WS_CHILD | WS_VISIBLE, 0, y + 4, Cfg::Ui::SPEED_LABEL_WIDTH,
                                Cfg::Ui::CTL_HEIGHT, parent, nullptr, inst, nullptr);

    hSpeed = CreateWindowW(L"COMBOBOX", nullptr, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 0, y,
                           Cfg::Ui::SPEED_COMBO_WIDTH, 400, parent, (HMENU)(int)CtrlId::SPEED, inst, nullptr);

    hWrap = CreateWindowW(L"BUTTON", L"Wrap", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 0, y + 4,
                          Cfg::Ui::CHECKBOX_WIDTH, 20, parent, (HMENU)(int)CtrlId::WRAP, inst, nullptr);

    hGrid = CreateWindowW(L"BUTTON", L"Grid", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 0, y + 4,
                          Cfg::Ui::CHECKBOX_WIDTH, 20, parent, (HMENU)(int)CtrlId::GRID, inst, nullptr);

    for (int ms : Cfg::Timer::SPEED_OPTIONS) {
        wchar_t buf[32];
        swprintf(buf, 32, L"%d ms", ms);
        SendMessageW(hSpeed, CB_ADDSTRING, 0, (LPARAM)buf);
    }
    SetSpeedDefault();
}

void Ui::Layout(HWND parent)
{
    RECT rc;
    GetClientRect(parent, &rc);
    int x = Cfg::Ui::MARGIN_X;
    const int y = Cfg::Ui::MARGIN_Y;

    x = PlaceCtl(hRuleEdit, x, y, Cfg::Ui::RULE_EDIT_WIDTH, Cfg::Ui::CTL_HEIGHT);
    x = PlaceCtl(hRuleApply, x, y, Cfg::Ui::APPLY_BTN_WIDTH, Cfg::Ui::CTL_HEIGHT);
    x = PlaceCtl(hStart, x, y, Cfg::Ui::START_BTN_WIDTH, Cfg::Ui::CTL_HEIGHT);
    x = PlaceCtl(hStep, x, y, Cfg::Ui::STEP_BTN_WIDTH, Cfg::Ui::CTL_HEIGHT);
    x = PlaceCtl(hReset, x, y, Cfg::Ui::RESET_BTN_WIDTH, Cfg::Ui::CTL_HEIGHT);
    x = PlaceCtl(hSetInit, x, y, Cfg::Ui::SET_INIT_BTN_WIDTH, Cfg::Ui::CTL_HEIGHT);
    x = PlaceCtl(hRandom, x, y, Cfg::Ui::RANDOM_BTN_WIDTH, Cfg::Ui::CTL_HEIGHT);
    x = PlaceCtl(hClear, x, y, Cfg::Ui::CLEAR_BTN_WIDTH, Cfg::Ui::CTL_HEIGHT);
    x = PlaceCtl(hSave, x, y, Cfg::Ui::SAVE_BTN_WIDTH, Cfg::Ui::CTL_HEIGHT);
    x = PlaceCtl(hSpeedLabel, x, y + 4, Cfg::Ui::SPEED_LABEL_WIDTH, Cfg::Ui::CTL_HEIGHT);

    RECT r{};
    GetWindowRect(hSpeed, &r);
    const int comboH = r.bottom - r.top;
    x = PlaceCtl(hSpeed, x, y, Cfg::Ui::SPEED_COMBO_WIDTH, comboH);

    x = PlaceCtl(hWrap, x, y + 4, Cfg::Ui::CHECKBOX_WIDTH, 20);
    (void)PlaceCtl(hGrid, x, y + 4, Cfg::Ui::CHECKBOX_WIDTH, 20);
}

void Ui::SetFont(HFONT f)
{
    HWND ctrls[] = {hRuleEdit, hRuleApply, hStart, hStep, hReset, hSetInit,   hRandom,
                    hClear,    hSave,      hSpeed, hWrap, hGrid,  hSpeedLabel};
    for (HWND c : ctrls) {
        if (c) {
            SendMessageW(c, WM_SETFONT, (WPARAM)f, TRUE);
        }
    }
}
