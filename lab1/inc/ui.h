#pragma once
#include "config.h"
#include <string>
#include <windows.h>

class Ui
{
public:
    Ui();

    void Create(HWND parent, HINSTANCE inst);
    void Layout(HWND parent);
    void SetFont(HFONT f);

    inline std::wstring RuleText() const
    {
        wchar_t buf[128]{0};
        GetWindowTextW(hRuleEdit, buf, 128);
        return buf;
    }

    inline void SetRuleText(const std::wstring &s)
    {
        SetWindowTextW(hRuleEdit, s.c_str());
    }

    inline void SetStartCaption(bool running)
    {
        SetWindowTextW(hStart, running ? L"Pause" : L"Start");
    }

    inline int SpeedMs() const
    {
        int sel = static_cast<int>(SendMessageW(hSpeed, CB_GETCURSEL, 0, 0));
        if (sel < 0) {
            sel = Cfg::Timer::SPEED_DEFAULT_INDEX;
        }
        return Cfg::Timer::SPEED_OPTIONS[sel];
    }

    inline void SetSpeedDefault()
    {
        SendMessageW(hSpeed, CB_SETCURSEL, Cfg::Timer::SPEED_DEFAULT_INDEX, 0);
    }

    inline bool WrapChecked() const
    {
        return SendMessageW(hWrap, BM_GETCHECK, 0, 0) == BST_CHECKED;
    }

    inline void SetWrapChecked(bool v)
    {
        SendMessageW(hWrap, BM_SETCHECK, v ? BST_CHECKED : BST_UNCHECKED, 0);
    }

    inline bool GridChecked() const
    {
        return SendMessageW(hGrid, BM_GETCHECK, 0, 0) == BST_CHECKED;
    }

    inline void SetGridChecked(bool v)
    {
        SendMessageW(hGrid, BM_SETCHECK, v ? BST_CHECKED : BST_UNCHECKED, 0);
    }

    inline HWND RuleEdit() const
    {
        return hRuleEdit;
    }
    inline HWND SpeedBox() const
    {
        return hSpeed;
    }

private:
    int PlaceCtl(HWND hwnd, int x, int y, int w, int h);

private:
    HWND hRuleEdit;
    HWND hRuleApply;
    HWND hStart;
    HWND hStep;
    HWND hReset;
    HWND hSetInit;
    HWND hRandom;
    HWND hClear;
    HWND hSave;
    HWND hSpeed;
    HWND hWrap;
    HWND hGrid;
    HWND hSpeedLabel;
};
