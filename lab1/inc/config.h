#pragma once
#include <cstdint>
#include <windows.h>

namespace Cfg
{

namespace Window
{
inline constexpr int WIDTH = 1000;
inline constexpr int HEIGHT = 800;
}  // namespace Window

namespace Ui
{
inline constexpr int PANEL_HEIGHT = 44;
inline constexpr int MARGIN_X = 8;
inline constexpr int MARGIN_Y = 8;
inline constexpr int SPACING_X = 8;
inline constexpr int CTL_HEIGHT = 24;

inline constexpr int RULE_EDIT_WIDTH = 80;
inline constexpr int APPLY_BTN_WIDTH = 64;
inline constexpr int START_BTN_WIDTH = 70;
inline constexpr int STEP_BTN_WIDTH = 60;
inline constexpr int RESET_BTN_WIDTH = 64;
inline constexpr int SET_INIT_BTN_WIDTH = 80;
inline constexpr int RANDOM_BTN_WIDTH = 70;
inline constexpr int CLEAR_BTN_WIDTH = 64;
inline constexpr int SAVE_BTN_WIDTH = 86;
inline constexpr int SPEED_LABEL_WIDTH = 48;
inline constexpr int SPEED_COMBO_WIDTH = 90;
inline constexpr int CHECKBOX_WIDTH = 60;
}  // namespace Ui

namespace Automaton
{
inline constexpr int DEFAULT_W = 200;
inline constexpr int DEFAULT_H = 200;

inline constexpr int NEIGHBORS_VON_NEUMANN = 4;

inline constexpr int RULE_BITS_COUNT = 10;
inline constexpr int RULE_ROWS_PER_CURR = 5;
inline constexpr int RULE_TOP_BIT_POS = RULE_BITS_COUNT - 1;

inline constexpr uint16_t DEFAULT_RULE = 286;

inline constexpr unsigned RANDOM_SCALE = 10000u;

inline constexpr int ACTIVE_RESERVE_DIVISOR = 8;

inline constexpr int SPARSE_CANDIDATE_FACTOR = NEIGHBORS_VON_NEUMANN + 1;
}  // namespace Automaton

namespace Render
{
inline constexpr COLORREF COLOR0 = RGB(255, 255, 255);
inline constexpr COLORREF COLOR1 = RGB(0, 0, 0);
inline constexpr COLORREF GRID_LINE_COLOR = RGB(220, 220, 220);
inline constexpr COLORREF PANEL_BG_COLOR = RGB(245, 245, 245);
inline constexpr COLORREF PANEL_LINE_COLOR = RGB(210, 210, 210);
}  // namespace Render

namespace Io
{
inline constexpr int BMP_BPP = 24;
inline constexpr int BMP_ROW_ALIGN = 4;
inline constexpr int BMP_SCALE = 4;
inline constexpr WORD BMP_SIG_BM = 0x4D42;
}  // namespace Io

namespace Timer
{
inline constexpr UINT ID = 1;
inline constexpr UINT DEFAULT_TICK_MS = 120;
inline constexpr int SPEED_OPTIONS[] = {25, 60, 120, 250, 500};
inline constexpr int SPEED_DEFAULT_INDEX = 2;
}  // namespace Timer

}  // namespace Cfg

enum class CtrlId : int {
    RULE_EDIT = 1001,
    RULE_APPLY,
    START,
    STEP,
    RESET,
    SET_INIT,
    RANDOMIZE,
    CLEAR,
    SAVE_BMP,
    SPEED,
    WRAP,
    GRID
};
