#include "app.h"
#include "config.h"
#include <commdlg.h>
#include <string>
#include <windows.h>
#include <windowsx.h>

int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int nCmdShow)
{
    App app;
    if (!app.Init(hInst, nCmdShow)) {
        return 0;
    }
    return app.Run();
}

extern "C" int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR, int nCmdShow)
{
    (void)hPrev;
    return wWinMain(hInst, nullptr, GetCommandLineW(), nCmdShow);
}
