#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00
#define ID_Select_Black 100
#define ID_Select_Red 101
#define ID_Button_Start 102
#define ID_Button_Stop 103
#define ID_Button_Backmost 104
#define ID_Button_Back 105
#define ID_Button_Front 106
#define ID_Button_Frontmost 107
#define ID_Button_Reverse 108
#define ID_Button_Modify 109
#define ID_Button_Show 110
#define ID_Button_Help 111
#define ID_Button_Plus 112
#define ID_Button_Minus 113
#define SPARE_TIME 200

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <cstdlib>
#include "chessboard.h"

float scaleX, scaleY;
ID2D1Factory *pFactory = NULL;
ID2D1DCRenderTarget *pRenderTarget = NULL;
ID2D1SolidColorBrush *pBlackBrush = NULL;
ID2D1SolidColorBrush *pRedBrush = NULL;
ID2D1SolidColorBrush *pWhiteBrush = NULL;
IDWriteFactory *pDWriteFactory = NULL;
IDWriteTextFormat *pPieceTextFormat = NULL;
IDWriteTextFormat *pTextFormat = NULL;
HFONT hDefaultFont;
RECT rc_button_black, rc_button_red, rc_round_text,
    rc_button_start, rc_button_stop, rc_button_reverse,
    rc_button_front, rc_button_back, rc_button_backmost, rc_button_frontmost,
    rc_button_modify, rc_button_show, rc_button_help,
    rc_button_plus, rc_button_minus;

const wchar_t CHARSET_UNICODE[][2] = {L" ",
                                      L"車", L"馬", L"象", L"士", L"將", L"砲", L"卒",
                                      L"俥", L"傌", L"相", L"仕", L"帥", L"炮", L"兵"};
int margin = 20;

struct State_Info
{
    int state, turn, turn_pre, hlt_n, round, game_red_won, game_black_won,
        steps_without_eating, steps_without_eating_max;
    bool machine_red, machine_black, running, isreadyBlack, isreadyRed;
    chessboard brd, brd_pre, brd_ini;
    vector<string> moves;
    int moves_index, display_size;
    position hlt[2] = {-1, -1, -1, -1};
    vector<string> s_set;
    wstring dir_red, dir_black, name_red, name_black;
    ULONGLONG time_red, time_black, time_red_pre, time_black_pre,
        time_start, go_time, time_limit;
    string s_receive, s_send;
    bool sent, over, reviewing, reverse, counting, modifying, showing;
    void Initialize()
    {
        brd = brd_ini = brd_pre = str2brd(ini_state);
        vector<string>().swap(moves);
        moves_index = display_size = 0;
        state = 1;
        turn = turn_pre = 1;
        steps_without_eating = 0;
        steps_without_eating_max = 120;
        round = 0;
        s_set = possible_move(brd, turn);
        hlt_n = 0;
        time_limit = 1000;
        hlt[0] = {-1, -1};
        hlt[1] = {-1, -1};
        running = 0;
        s_send = "ucci";
        sent = 0;
        over = 0;
        reviewing = 0;
        time_red = time_black = 0;
        time_red_pre = time_black_pre = 0;
        time_start = -1;
        counting = 0;
        showing = modifying = 0;
    }
    State_Info()
    {
        go_time = 1000;
        reverse = 0;
        machine_black = machine_red = 0;
        isreadyBlack = isreadyRed = 0;
        dir_red = L"";
        dir_black = L"";
        name_red = L"";
        name_black = L"";
        game_black_won = game_red_won = 0;
        Initialize();
    }

    HWND hWndButtonRed, hWndButtonBlack, hWndButtonStart, hWndButtonReverse;
    HWND hWndButtonModify, hWndButtonShow;
    HANDLE hInReadBlack, hInWriteBlack, hOutReadBlack, hOutWriteBlack;
    HANDLE hInReadRed, hInWriteRed, hOutReadRed, hOutWriteRed;
    PROCESS_INFORMATION piProcInfoBlack, piProcInfoRed;
    HANDLE hThread;
};

LRESULT OnCreate(HWND hWnd)
{
    // Create Factory
    HRESULT hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &pFactory);
    if (!SUCCEEDED(hr))
        return -1;

    // Create board render target.
    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(
            DXGI_FORMAT_B8G8R8A8_UNORM,
            D2D1_ALPHA_MODE_IGNORE),
        0,
        0,
        D2D1_RENDER_TARGET_USAGE_NONE,
        D2D1_FEATURE_LEVEL_DEFAULT);
    hr = pFactory->CreateDCRenderTarget(&props, &pRenderTarget);
    HDC hdc = GetDC(hWnd);
    RECT rc;
    GetClientRect(hWnd, &rc);
    rc.right = rc.left + (int)((rc.bottom - rc.top - margin * 2) * 0.9) + 2 * margin;
    pRenderTarget->BindDC(hdc, &rc);
    if (!SUCCEEDED(hr))
        return -1;
    hr = pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Black), &pBlackBrush);
    if (!SUCCEEDED(hr))
        return -1;
    hr = pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Red), &pRedBrush);
    if (!SUCCEEDED(hr))
        return -1;
    hr = pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::White), &pWhiteBrush);
    if (!SUCCEEDED(hr))
        return -1;
    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown **>(&pDWriteFactory));
    if (!SUCCEEDED(hr))
        return -1;
    hr = pDWriteFactory->CreateTextFormat(
        L"KaiTi", NULL, DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        28.0f * scaleX, L"", &pPieceTextFormat);
    hr = pPieceTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    hr = pPieceTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    if (!SUCCEEDED(hr))
        return -1;
    hr = pDWriteFactory->CreateTextFormat(
        L"Segoe UI", NULL, DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        16.0f * scaleX, L"", &pTextFormat);
    hr = pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    hr = pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    if (!SUCCEEDED(hr))
        return -1;
    return 0;
}

template <class T>
void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

D2D1_POINT_2F operator+(const D2D1_POINT_2F a, const D2D1_POINT_2F b)
{
    D2D1_POINT_2F c = {a.x + b.x, a.y + b.y};
    return c;
}

void PaintBoard(HWND hWnd, chessboard &brd, int n = 0, position upd[] = NULL, int m = 0, position hlt[] = NULL)
{
    State_Info *pState = (State_Info *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    RECT rc;
    GetClientRect(hWnd, &rc);
    float l = ((rc.bottom - rc.top) - 2 * margin) / 10, stroke = 1,
          radius = 0.4f * l, square_ratio = 0.9, line_len = 0.2 * l;
    pRenderTarget->BeginDraw();
    if (n == 0)
    {
        n = 90;
        upd = new position[90];
        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 9; j++)
            {
                upd[i * 9 + j].i = i;
                upd[i * 9 + j].j = j;
            }
        if (upd == NULL)
        {
            MessageBox(hWnd, L"Fail to allocate memory!", L"error", MB_ICONERROR);
            return;
        }
        pRenderTarget->Clear(D2D1::ColorF(1, 1, 1));
        if (pState->showing)
        {
            wchar_t ws[2] = L"9";
            if (pState->reverse)
                ws[0] = L'0';
            for (int i = 0; i < 10; i++)
            {
                D2D1_RECT_F rect_f = {0, i * l + margin, (float)margin, (i + 1) * l + margin};
                pRenderTarget->DrawText(ws, 1, pTextFormat, rect_f, pBlackBrush);
                rect_f = {9 * l + margin, i * l + margin, 9 * l + 2 * margin, (i + 1) * l + margin};
                pRenderTarget->DrawText(ws, 1, pTextFormat, rect_f, pBlackBrush);
                ws[0] = ws[0] + (pState->reverse ? 1 : -1);
            }
            ws[0] = L'a';
            for (int j = 0; j < 9; j++)
            {
                D2D1_RECT_F rect_f = {j * l + margin, 0, (j + 1) * l + margin, (float)margin};
                pRenderTarget->DrawText(ws, 1, pTextFormat, rect_f, pBlackBrush);
                rect_f = {j * l + margin, 10 * l + margin, (j + 1) * l + margin, 10 * l + 2 * margin};
                pRenderTarget->DrawText(ws, 1, pTextFormat, rect_f, pBlackBrush);
                ws[0]++;
            }
        }
    }
    for (int idx = 0; idx < n; idx++)
    {
        int i = upd[idx].i, j = upd[idx].j;
        if (i < 0 || i > 9 || j < 0 || j > 9)
            continue;
        if (pState != NULL && pState->reverse)
            i = 9 - i;
        D2D_POINT_2F top = {(j + 0.5f) * l + margin, i * l + margin},
                     bottom = {(j + 0.5f) * l + margin, (i + 1) * l + margin},
                     left = {j * l + margin, (i + 0.5f) * l + margin},
                     right = {(j + 1) * l + margin, (i + 0.5f) * l + margin},
                     lefttop = {j * l + margin, i * l + margin},
                     leftbottom = {j * l + margin, (i + 1) * l + margin},
                     righttop = {(j + 1) * l + margin, i * l + margin},
                     rightbottom = {(j + 1) * l + margin, (i + 1) * l + margin},
                     mid = {(j + 0.5f) * l + margin, (i + 0.5f) * l + margin};
        pRenderTarget->FillRectangle({left.x, top.y, right.x, bottom.y}, pWhiteBrush);
        if (i == 0 || j != 0 && j != 8 && i == 5)
            pRenderTarget->DrawLine(mid, bottom, pBlackBrush, scaleX * stroke);
        else if (i == 9 || j != 0 && j != 8 && i == 4)
            pRenderTarget->DrawLine(top, mid, pBlackBrush, scaleX * stroke);
        else
            pRenderTarget->DrawLine(top, bottom, pBlackBrush, scaleX * stroke);
        if (j == 0)
            pRenderTarget->DrawLine(mid, right, pBlackBrush, scaleX * stroke);
        else if (j == 8)
            pRenderTarget->DrawLine(left, mid, pBlackBrush, scaleX * stroke);
        else
            pRenderTarget->DrawLine(left, right, pBlackBrush, scaleX * stroke);
        if (i == 0 && j == 3 || i == 1 && j == 4 || i == 7 && j == 3 || i == 8 && j == 4)
            pRenderTarget->DrawLine(mid, rightbottom, pBlackBrush, scaleX * stroke);
        if (i == 2 && j == 5 || i == 1 && j == 4 || i == 9 && j == 5 || i == 8 && j == 4)
            pRenderTarget->DrawLine(mid, lefttop, pBlackBrush, scaleX * stroke);
        if (i == 0 && j == 5 || i == 1 && j == 4 || i == 7 && j == 5 || i == 8 && j == 4)
            pRenderTarget->DrawLine(mid, leftbottom, pBlackBrush, scaleX * stroke);
        if (i == 2 && j == 3 || i == 1 && j == 4 || i == 9 && j == 3 || i == 8 && j == 4)
            pRenderTarget->DrawLine(mid, righttop, pBlackBrush, scaleX * stroke);
        if (pState != NULL && pState->reverse)
            i = 9 - i;
        if (brd[i][j] > 0 && brd[i][j] < 8)
        {
            pRenderTarget->FillEllipse({mid, radius, radius}, pWhiteBrush);
            pRenderTarget->DrawEllipse({mid, radius, radius}, pBlackBrush, scaleX * stroke);
            pRenderTarget->DrawText(
                CHARSET_UNICODE[brd[i][j]], 1, pPieceTextFormat,
                {(left).x, (top).y - 2, (right).x, (bottom).y - 2},
                pBlackBrush);
        }
        if (brd[i][j] > 7 && brd[i][j] < 15)
        {
            pRenderTarget->FillEllipse({mid, radius, radius}, pWhiteBrush);
            pRenderTarget->DrawEllipse({mid, radius, radius}, pRedBrush, scaleX * stroke);
            pRenderTarget->DrawText(
                CHARSET_UNICODE[brd[i][j]], 1, pPieceTextFormat,
                {(left).x, (top).y - 2, (right).x, (bottom).y - 2},
                pRedBrush);
        }
    }
    if (n == 0)
        delete[] upd;
    for (int idx = 0; idx < m; idx++)
    {
        int i = hlt[idx].i, j = hlt[idx].j;
        if (pState != NULL && pState->reverse)
            i = 9 - i;
        D2D_POINT_2F top = {(j + 0.5f) * l + margin, i * l + margin},
                     bottom = {(j + 0.5f) * l + margin, (i + 1) * l + margin},
                     left = {j * l + margin, (i + 0.5f) * l + margin},
                     right = {(j + 1) * l + margin, (i + 0.5f) * l + margin};
        float delta = (1 - square_ratio) / 2.0f * l;
        pRenderTarget->DrawLine({left.x + delta, top.y + delta},
                                {left.x + delta, top.y + delta + line_len},
                                pBlackBrush, scaleX * stroke);
        pRenderTarget->DrawLine({left.x + delta, top.y + delta},
                                {left.x + delta + line_len, top.y + delta},
                                pBlackBrush, scaleX * stroke);
        pRenderTarget->DrawLine({right.x - delta, top.y + delta},
                                {right.x - delta, top.y + delta + line_len},
                                pBlackBrush, scaleX * stroke);
        pRenderTarget->DrawLine({right.x - delta, top.y + delta},
                                {right.x - delta - line_len, top.y + delta},
                                pBlackBrush, scaleX * stroke);
        pRenderTarget->DrawLine({left.x + delta, bottom.y - delta},
                                {left.x + delta, bottom.y - delta - line_len},
                                pBlackBrush, scaleX * stroke);
        pRenderTarget->DrawLine({left.x + delta, bottom.y - delta},
                                {left.x + delta + line_len, bottom.y - delta},
                                pBlackBrush, scaleX * stroke);
        pRenderTarget->DrawLine({right.x - delta, bottom.y - delta},
                                {right.x - delta, bottom.y - delta - line_len},
                                pBlackBrush, scaleX * stroke);
        pRenderTarget->DrawLine({right.x - delta, bottom.y - delta},
                                {right.x - delta - line_len, bottom.y - delta},
                                pBlackBrush, scaleX * stroke);
    }
    HRESULT hr = pRenderTarget->EndDraw();
    if (!SUCCEEDED(hr))
        MessageBox(hWnd, L"Fail to finish paint!", L"Error", MB_ICONERROR);
}

void ForceTerminate(HWND hWnd, int turn)
{
    State_Info *pState = (State_Info *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    TerminateThread(pState->hThread, 0);
    pState->hThread = NULL;
    if (turn == 1)
    {
        ShowWindow(pState->hWndButtonRed, SW_SHOW);
        RECT rc_text_red = {rc_button_red.left, rc_button_red.top,
                            rc_button_red.left + (int)(90 * scaleX),
                            rc_button_red.bottom + (int)(90 * scaleX)};
        InvalidateRect(hWnd, &rc_text_red, FALSE);
        TerminateProcess(pState->piProcInfoRed.hProcess, 0);
        CloseHandle(pState->piProcInfoRed.hProcess);
        CloseHandle(pState->piProcInfoRed.hThread);
        CloseHandle(pState->hInReadRed);
        CloseHandle(pState->hInWriteRed);
        CloseHandle(pState->hOutReadRed);
        CloseHandle(pState->hOutWriteRed);
        pState->machine_red = 0;
        pState->dir_red = L"";
        pState->name_red = L"";
        pState->isreadyRed = 0;
    }
    if (turn == 2)
    {
        ShowWindow(pState->hWndButtonBlack, SW_SHOW);
        RECT rc_text_black = {rc_button_black.left, rc_button_black.top,
                              rc_button_black.left + (int)(90 * scaleX),
                              rc_button_black.top + (int)(90 * scaleX)};
        InvalidateRect(hWnd, &rc_text_black, FALSE);
        TerminateProcess(pState->piProcInfoBlack.hProcess, 0);
        CloseHandle(pState->piProcInfoBlack.hProcess);
        CloseHandle(pState->piProcInfoBlack.hThread);
        CloseHandle(pState->hInReadBlack);
        CloseHandle(pState->hInWriteBlack);
        CloseHandle(pState->hOutReadBlack);
        CloseHandle(pState->hOutWriteBlack);
        pState->machine_black = 0;
        pState->dir_black = L"";
        pState->name_black = L"";
        pState->isreadyBlack = 0;
    }
    if (!pState->machine_black && !pState->machine_red)
        SetWindowText(pState->hWndButtonStart, L"▶");
    pState->Initialize();
}

void UpdateFenString(State_Info *pState)
{
    pState->sent = 0;
    if ((pState->turn == 2 && pState->machine_black && !pState->isreadyBlack ||
         pState->turn == 1 && pState->machine_red && !pState->isreadyRed) &&
        pState->running)
    {
        pState->s_send = "ucci";
        return;
    }
    pState->s_send = "position fen ";
    pState->s_send += brd2str(pState->brd_pre);
    pState->s_send += pState->turn_pre == 1 ? " r - - " : " b - - ";
    char tmp[32];
    itoa(pState->steps_without_eating, tmp, 10);
    pState->s_send += tmp;
    pState->s_send += " ";
    itoa(pState->round, tmp, 10);
    pState->s_send += tmp;
    if (!pState->moves.empty())
    {
        pState->s_send += " moves";
        for (int i = pState->moves_index; i < pState->moves.size(); i++)
        {
            pState->s_send += " ";
            pState->s_send += pState->moves[i];
        }
    }
    pState->s_send += "\r\ngo time ";
    itoa(pState->go_time, tmp, 10);
    pState->s_send += tmp;
}

int ImplementMove(HWND hWnd)
{
    State_Info *pState = (State_Info *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    string s = mov2str({1, pState->hlt[0], pState->hlt[1]});
    bool flag = 0;
    for (int i = 0; i < pState->s_set.size(); i++)
        if (s == pState->s_set[i])
        {
            flag = 1;
            break;
        }
    if (!flag)
        return 0;
    pState->steps_without_eating++;
    if (pState->turn == 1)
        pState->round++;
    int moveTo = pState->brd[pState->hlt[1].i][pState->hlt[1].j];
    if (moveTo > 0 && moveTo < 15 &&
        (pState->turn == 1 && moveTo < 8 ||
         pState->turn == 2 && moveTo > 7))
        flag = 1;
    else
        flag = 0;
    pState->moves.push_back(s);
    move(pState->brd, s);
    pState->hlt_n = 2;
    PaintBoard(hWnd, pState->brd, 2, pState->hlt, pState->hlt_n, pState->hlt);
    pState->state = 1;
    pState->turn = 3 - pState->turn;
    pState->s_set = possible_move(pState->brd, pState->turn);
    if (pState->s_set.empty())
    {
        pState->over = 1;
        pState->running = pState->counting = 0;
        wstring s = (pState->turn == 1 ? L"黑" : L"红");
        s += L"方赢!";
        (pState->turn == 1 ? pState->game_black_won : pState->game_red_won)++;
        MessageBox(hWnd, s.c_str(), L"本局结束", MB_OK);
        wstring title = L"UCCI Judge";
        if (pState->game_red_won + pState->game_black_won > 0)
        {
            title += L" (";
            char s_tmp[16];
            wchar_t ws_tmp[16];
            itoa(pState->game_red_won, s_tmp, 10);
            MultiByteToWideChar(CP_ACP, 0, s_tmp, -1, ws_tmp, 16);
            title += ws_tmp;
            itoa(pState->game_black_won, s_tmp, 10);
            MultiByteToWideChar(CP_ACP, 0, s_tmp, -1, ws_tmp, 16);
            title += L" : ";
            title += ws_tmp;
            title += L")";
            SetWindowText(hWnd, title.c_str());
        }
    }
    if (flag)
    {
        pState->moves_index = pState->moves.size();
        pState->brd_pre = pState->brd;
        pState->turn_pre = pState->turn;
        pState->steps_without_eating = 0;
    }
    flag = 0;
    if (pState->moves.size() > 7)
        // bug: if size() -> ULONGLONG then i = 0x7fffffff, if size() -> LONGLONG then i = -1
        // so when process casting, this is quite important
        for (int i = (pState->moves.size() - 2) / 2; i < pState->moves.size() - 2; i++)
            if (pState->moves[i] == pState->moves[pState->moves.size() - 2])
            {
                flag = 1;
                for (int j = 1; j < pState->moves.size() - 2 - i; j++)
                    if (pState->moves[i - j] != pState->moves[pState->moves.size() - 2 - j])
                        flag = 0;
            }
    if (flag)
    {
        pState->over = 1;
        pState->running = pState->counting = 0;
        MessageBox(hWnd, L"不变待判", L"待判", MB_OK);
    }
    int num[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 9; j++)
            num[pState->brd[i][j]] += 1;
    if (num[1] == 0 && num[2] == 0 && num[6] == 0 && num[7] == 0 &&
        num[8] == 0 && num[9] == 0 && num[13] == 0 && num[14] == 0)
    {
        pState->over = 1;
        pState->running = pState->counting = 0;
        MessageBox(hWnd, L"子力不足", L"和棋", MB_OK);
    }
    else if (num[1] == 0 && num[2] == 0 && num[3] == 0 && num[4] == 0 && num[6] < 2 &&
             num[8] == 0 && num[9] == 0 && num[10] == 0 && num[11] == 0 && num[13] < 2 &&
             num[7] == 0 && num[14] == 0)
    {
        pState->over = 1;
        pState->running = pState->counting = 0;
        MessageBox(hWnd, L"子力不足", L"和棋", MB_OK);
    }
    char s_tmp[16];
    wchar_t ws_tmp[16];
    itoa(pState->steps_without_eating_max / 2, s_tmp, 10);
    MultiByteToWideChar(CP_ACP, 0, s_tmp, -1, ws_tmp, 16);
    wstring ws = ws_tmp;
    if (pState->steps_without_eating >= pState->steps_without_eating_max)
    {
        pState->over = 1;
        pState->running = pState->counting = 0;
        MessageBox(hWnd, (ws + L"回合无吃子").c_str(), L"和棋", MB_OK);
    }
    pState->display_size = pState->moves.size();
    return 1;
}

wstring time2wstring(ULONGLONG millisecs)
{
    wstring s_time = L"";
    int secs = millisecs / 1000;
    char tmp[8];
    wchar_t sec_tmp[8], min_tmp[8];
    itoa(secs / 60, tmp, 10);
    MultiByteToWideChar(CP_ACP, 0, tmp, -1, min_tmp, 16);
    if (strlen(tmp) == 1)
        s_time += L"0";
    itoa(secs % 60, tmp, 10);
    MultiByteToWideChar(CP_ACP, 0, tmp, -1, sec_tmp, 16);
    s_time = s_time + min_tmp + L":" + (strlen(tmp) == 1 ? L"0" : L"") + sec_tmp;
    return s_time;
}

vector<string> read_pgn(string filename);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_DESTROY)
    {
        State_Info *pState = (State_Info *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        SafeRelease(&pFactory);
        SafeRelease(&pRenderTarget);
        SafeRelease(&pBlackBrush);
        SafeRelease(&pRedBrush);
        SafeRelease(&pWhiteBrush);
        TerminateProcess(pState->piProcInfoBlack.hProcess, 0);
        TerminateProcess(pState->piProcInfoRed.hProcess, 0);
        PostQuitMessage(0);
    }
    else if (uMsg == WM_CREATE)
    {
        HRESULT hr = OnCreate(hWnd);
        if (!SUCCEEDED(hr))
            MessageBox(hWnd, L"Fail to create rendering objects!", L"Error", MB_ICONERROR);
    }
    else if (uMsg == WM_GETMINMAXINFO)
    {
        MINMAXINFO *pmmi = reinterpret_cast<MINMAXINFO *>(lParam);
        POINT sz_min = {(int)(scaleX * 648), (int)(scaleY * 576)};
        POINT sz_max = {(int)(scaleX * 648), (int)(scaleY * 576)};
        pmmi->ptMinTrackSize = sz_min;
        pmmi->ptMaxTrackSize = sz_max;
    }
    else if (uMsg == WM_PAINT)
    {
        State_Info *pState = (State_Info *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        RECT rc;
        GetClientRect(hWnd, &rc);
        rc.left = rc.left + (int)((rc.bottom - rc.top - margin * 2) * 0.9) + 2 * margin;
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        SelectObject(hdc, hDefaultFont);
        COLORREF color = RGB(0xf0, 0xf0, 0xf0);
        SetBkColor(hdc, color);
        FillRect(hdc, &rc, CreateSolidBrush(color));
        RECT rc_text_black = {rc_button_black.left,
                              rc_button_black.top - (rc_button_black.bottom - rc_button_black.top),
                              rc_button_black.left + (int)(100 * scaleX), rc_button_black.top},
             rc_text_red = {rc_button_red.left,
                            rc_button_red.top - (int)(rc_button_red.bottom - rc_button_red.top),
                            rc_button_red.left + (int)(100 * scaleX), rc_button_red.top};
        wstring ws_text = L"黑方: ";
        ws_text += time2wstring(pState->time_black);
        DrawText(hdc, ws_text.c_str(), -1, &rc_text_black, DT_LEFT);
        ws_text = L"红方: ";
        ws_text += time2wstring(pState->time_red);
        DrawText(hdc, ws_text.c_str(), -1, &rc_text_red, DT_LEFT);
        rc_text_black = {rc_button_black.left, rc_button_black.top,
                         rc_button_black.left + (int)(90 * scaleX),
                         rc_button_black.top + (int)(90 * scaleX)};
        rc_text_red = {rc_button_red.left, rc_button_red.top,
                       rc_button_red.left + (int)(90 * scaleX),
                       rc_button_red.bottom + (int)(90 * scaleX)};
        if (pState->machine_black)
            DrawText(hdc, pState->name_black.c_str(), -1, &rc_text_black,
                     DT_LEFT | DT_EDITCONTROL | DT_WORDBREAK);
        if (pState->machine_red)
            DrawText(hdc, pState->name_red.c_str(), -1, &rc_text_red,
                     DT_LEFT | DT_EDITCONTROL | DT_WORDBREAK);
        ws_text = L"回合: ";
        char tmp[16];
        wchar_t wtmp[16];
        itoa((pState->display_size + 1) / 2, tmp, 10);
        MultiByteToWideChar(CP_ACP, 0, tmp, -1, wtmp, 16);
        ws_text += wtmp;
        rc_round_text = {(int)(510 * scaleX), (int)((rc.bottom - rc.top) / 16.0f - 15 * scaleY),
                         (int)(600 * scaleX), (int)((rc.bottom - rc.top) / 16.0f + 15 * scaleY)};
        if (pState->reviewing)
            DrawText(hdc, ws_text.c_str(), -1, &rc_round_text, DT_LEFT);
        else
            FillRect(hdc, &rc_round_text, CreateSolidBrush(color));
        RECT rc_time_text = {rc_button_black.left, rc_button_minus.top - (int)(30 * scaleY),
                             rc_button_black.left + (int)(90 * scaleX), rc_button_minus.top};
        DrawText(hdc, L"限定时间:", -1, &rc_time_text, DT_LEFT);
        RECT rc_time = {rc_button_black.left, rc_button_minus.top,
                        rc_button_black.left + (int)(60 * scaleX),
                        rc_button_minus.bottom};
        ws_text = L"";
        itoa(pState->go_time / 1000, tmp, 10);
        MultiByteToWideChar(CP_ACP, 0, tmp, -1, wtmp, 16);
        ws_text = ws_text + wtmp + L".";
        itoa(pState->go_time / 100 % 10, tmp, 10);
        MultiByteToWideChar(CP_ACP, 0, tmp, -1, wtmp, 16);
        ws_text = ws_text + wtmp + L"s";
        DrawText(hdc, ws_text.c_str(), -1, &rc_time, DT_LEFT);
        EndPaint(hWnd, &ps);
        if (!pState->reviewing)
            PaintBoard(hWnd, pState->brd, 0, NULL, pState->hlt_n, pState->hlt);
        else
        {
            chessboard brd = pState->brd_ini;
            for (int i = 0; i < pState->display_size; i++)
                move(brd, pState->moves[i]);
            if (!pState->sent)
                PaintBoard(hWnd, brd);
        }
    }
    else if (uMsg == WM_LBUTTONDOWN)
    {
        // handle the click message
        State_Info *pState = (State_Info *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (pState->reviewing || pState->over)
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        RECT rc;
        GetClientRect(hWnd, &rc);
        int xPos = LOWORD(lParam) / scaleX, yPos = HIWORD(lParam) / scaleY;
        float l = (rc.bottom - rc.top - 2 * margin) / 10 / scaleY;
        int i = (yPos - margin / scaleY) / l, j = (xPos - margin / scaleY) / l;
        if (pState->reverse)
            i = 9 - i;
        if (i >= 0 && i <= 9 && j >= 0 && j <= 8)
            if (pState->state == 1)
            {
                if (pState->brd[i][j] > 0 && pState->brd[i][j] < 15 &&
                        (pState->turn == 1 && pState->brd[i][j] > 7 &&
                             !(pState->machine_red && pState->running) ||
                         pState->turn == 2 && pState->brd[i][j] < 8 &&
                             !(pState->machine_black && pState->running)) ||
                    pState->modifying && pState->brd[i][j] > 0 && pState->brd[i][j] < 15)
                {
                    position hlt[1] = {i, j};
                    pState->hlt_n = 1;
                    PaintBoard(hWnd, pState->brd, 2, pState->hlt, pState->hlt_n, hlt);
                    pState->hlt[0] = hlt[0];
                    pState->state = 2;
                    if ((pState->turn == 1 && !pState->machine_red ||
                         pState->turn == 2 && !pState->machine_black) &&
                        !pState->modifying)
                        pState->counting = 1;
                }
            }
            else
            {
                pState->hlt[1].i = i;
                pState->hlt[1].j = j;
                if (pState->modifying)
                {
                    string s_tmp = mov2str({1, pState->hlt[0], pState->hlt[1]});
                    chessboard brd_tmp = pState->brd;
                    move(brd_tmp, s_tmp);
                    if (isvalid(brd_tmp) && ischeck(brd_tmp) == 0)
                    {
                        pState->state = 1;
                        pState->hlt_n = 2;
                        pState->brd_ini = pState->brd_pre = pState->brd = brd_tmp;
                        vector<string>().swap(pState->moves);
                        pState->display_size = 0;
                        UpdateFenString(pState);
                        pState->s_set = possible_move(pState->brd, pState->turn);
                        PaintBoard(hWnd, pState->brd, 2, pState->hlt, pState->hlt_n, pState->hlt);
                    }
                }
                else if (i == pState->hlt[0].i && j == pState->hlt[0].j)
                {
                    pState->state = 1;
                    pState->hlt_n = 0;
                    PaintBoard(hWnd, pState->brd, 1, pState->hlt, pState->hlt_n, NULL);
                }
                else if (pState->brd[i][j] > 0 && pState->brd[i][j] < 15 &&
                         (pState->turn == 1 && pState->brd[i][j] > 7 ||
                          pState->turn == 2 && pState->brd[i][j] < 8))
                {
                    if (pState->turn == 1 && !(pState->machine_red && pState->running) ||
                        pState->turn == 2 && !(pState->machine_black && pState->running))
                    {
                        position hlt[1] = {i, j};
                        pState->hlt_n = 1;
                        PaintBoard(hWnd, pState->brd, 2, pState->hlt, pState->hlt_n, hlt);
                        pState->hlt[0] = hlt[0];
                    }
                }
                else
                {
                    ImplementMove(hWnd);
                    UpdateFenString(pState);
                }
            }
        else
        {
            pState->hlt_n = 0;
            pState->state = 1;
            PaintBoard(hWnd, pState->brd);
        }
    }
    else if (uMsg == WM_KEYDOWN)
    {
        State_Info *pState = (State_Info *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (wParam == VK_LEFT)
            SendMessage(hWnd, WM_COMMAND, ((UINT)BN_CLICKED << 16) + ID_Button_Back, (LPARAM)NULL);
        else if (wParam == VK_RIGHT)
            SendMessage(hWnd, WM_COMMAND, ((UINT)BN_CLICKED << 16) + ID_Button_Front, (LPARAM)NULL);
        else if (wParam == VK_UP)
            SendMessage(hWnd, WM_COMMAND, ((UINT)BN_CLICKED << 16) + ID_Button_Plus, (LPARAM)NULL);
        else if (wParam == VK_DOWN)
            SendMessage(hWnd, WM_COMMAND, ((UINT)BN_CLICKED << 16) + ID_Button_Minus, (LPARAM)NULL);
        else if (wParam == VK_SPACE)
            SendMessage(hWnd, WM_COMMAND, ((UINT)BN_CLICKED << 16) + ID_Button_Reverse, (LPARAM)NULL);
        else if (wParam == VK_F1)
            SendMessage(hWnd, WM_COMMAND, ((UINT)BN_CLICKED << 16) + ID_Button_Help, (LPARAM)NULL);
        else if (wParam == VK_RETURN)
            if (!pState->machine_black)
                SendMessage(hWnd, WM_COMMAND, ((UINT)BN_CLICKED << 16) + ID_Select_Black, (LPARAM)NULL);
            else if (!pState->machine_red)
                SendMessage(hWnd, WM_COMMAND, ((UINT)BN_CLICKED << 16) + ID_Select_Red, (LPARAM)NULL);
            else
                SendMessage(hWnd, WM_COMMAND, ((UINT)BN_CLICKED << 16) + ID_Button_Start, (LPARAM)NULL);
    }
    else if (uMsg == WM_COMMAND)
    {
        int nc = HIWORD(wParam), id = LOWORD(wParam);
        if (nc == BN_CLICKED)
        {
            if (id == ID_Select_Black || id == ID_Select_Red)
            {
                State_Info *pState = (State_Info *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
                pState->running = pState->counting = 0;

                // get file name
                OPENFILENAME ofn;
                wchar_t szFile[1024] = {0};
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hWnd;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFilter = L"可执行文件(.exe)\0*.EXE\0棋谱(.pgn)\0*.PGN\0";
                ofn.nFilterIndex = 1;
                ofn.lpstrFileTitle = NULL;
                ofn.nMaxFileTitle = 0;
                ofn.lpstrInitialDir = NULL;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                WINBOOL suc = GetOpenFileName(&ofn);
                if (suc == FALSE)
                    return DefWindowProc(hWnd, uMsg, wParam, lParam);

                // open pgn
                wcslwr(szFile);
                wstring wstmp(szFile);
                if (wstmp.find(L".pgn") != wstmp.npos)
                {
                    pState->over = 1;
                    char filename[1024] = {0};
                    WideCharToMultiByte(CP_ACP, 0, szFile, -1, filename, 1024, NULL, NULL);
                    pState->moves = read_pgn(filename);
                    pState->display_size = 1;
                    SendMessage(hWnd, WM_COMMAND, ((UINT)BN_CLICKED << 16) + ID_Button_Back, (LPARAM)NULL);
                    SetFocus(hWnd);
                    return DefWindowProc(hWnd, uMsg, wParam, lParam);
                }

                // open engine
                HANDLE hInRead, hInWrite;
                HANDLE hOutRead, hOutWrite;
                STARTUPINFO siStartInfo;
                PROCESS_INFORMATION piProcInfo;
                SECURITY_ATTRIBUTES saAttr;
                saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
                saAttr.bInheritHandle = TRUE;
                saAttr.lpSecurityDescriptor = NULL;
                ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
                HRESULT hr;
                hr = CreatePipe(&hInRead, &hInWrite, &saAttr, 0);
                hr = CreatePipe(&hOutRead, &hOutWrite, &saAttr, 0);
                if (!SUCCEEDED(hr))
                {
                    MessageBox(hWnd, L"Fail to create pipes!", L"error", MB_ICONERROR);
                    return DefWindowProc(hWnd, uMsg, wParam, lParam);
                }
                SetHandleInformation(hOutRead, HANDLE_FLAG_INHERIT, 0);
                siStartInfo.dwFlags |= STARTF_USESHOWWINDOW;
                siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
                siStartInfo.hStdOutput = hOutWrite;
                siStartInfo.hStdInput = hInRead;
                hr = CreateProcess(NULL, szFile, NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo);
                if (!SUCCEEDED(hr))
                {
                    MessageBox(hWnd, L"Fail to open file!", L"error", MB_ICONERROR);
                    return DefWindowProc(hWnd, uMsg, wParam, lParam);
                }

                if (id == ID_Select_Black)
                {
                    pState->piProcInfoBlack = piProcInfo;
                    pState->hInReadBlack = hInRead;
                    pState->hInWriteBlack = hInWrite;
                    pState->hOutReadBlack = hOutRead;
                    pState->hOutWriteBlack = hOutWrite;
                    pState->dir_black = szFile;
                    pState->machine_black = 1;
                    int pos = pState->dir_black.find_last_of(L"\\", pState->dir_black.size() - 1);
                    pState->name_black = pState->dir_black.substr(pos + 1, pState->dir_black.size() - pos - 5);
                    if (pState->name_black.length() > 16)
                        pState->name_black = pState->name_black.substr(0, 16);
                    ShowWindow(pState->hWndButtonBlack, SW_HIDE);
                    RECT rc_text_black = {rc_button_black.left, rc_button_black.top,
                                          rc_button_black.left + (int)(90 * scaleX),
                                          rc_button_black.top + (int)(90 * scaleX)};
                    InvalidateRect(hWnd, &rc_text_black, FALSE);
                }
                else
                {
                    pState->piProcInfoRed = piProcInfo;
                    pState->hInReadRed = hInRead;
                    pState->hInWriteRed = hInWrite;
                    pState->hOutReadRed = hOutRead;
                    pState->hOutWriteRed = hOutWrite;
                    pState->dir_red = szFile;
                    pState->machine_red = 1;
                    int pos = pState->dir_red.find_last_of(L"\\", pState->dir_red.size() - 1);
                    pState->name_red = pState->dir_red.substr(pos + 1, pState->dir_red.size() - pos - 5);
                    if (pState->name_red.length() > 16)
                        pState->name_red = pState->name_red.substr(0, 16);
                    ShowWindow(pState->hWndButtonRed, SW_HIDE);
                    RECT rc_text_red = {rc_button_red.left, rc_button_red.top,
                                        rc_button_red.left + (int)(90 * scaleX),
                                        rc_button_red.bottom + (int)(90 * scaleX)};
                    InvalidateRect(hWnd, &rc_text_red, FALSE);
                }
                UpdateFenString(pState);
            }
            else if (id == ID_Button_Start)
            {
                State_Info *pState = (State_Info *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
                if (pState->modifying)
                    return DefWindowProc(hWnd, uMsg, wParam, lParam);
                wchar_t buttonText[4];
                GetWindowText(pState->hWndButtonStart, buttonText, -1);
                wstring s_tmp = buttonText;
                pState->modifying = 0;
                SetWindowText(pState->hWndButtonModify, L"↔");
                if (pState->machine_black || pState->machine_red)
                    if (s_tmp == (pState->running ? L"❚❚" : L"▶") || s_tmp == L"▷")
                        pState->running = !pState->running;
                if (pState->over)
                {
                    pState->Initialize();
                    PaintBoard(hWnd, pState->brd);
                }
                else if (pState->running)
                {
                    SetWindowText(pState->hWndButtonStart, L"❚❚");
                    UpdateFenString(pState);
                }
                if (pState->reviewing)
                {
                    pState->reviewing = 0;
                    pState->running = 0;
                    SetWindowText(pState->hWndButtonStart, L"▶");
                    PaintBoard(hWnd, pState->brd);
                }
                if (pState->running)
                    pState->counting = 1;
                else if (pState->machine_red && pState->machine_black)
                    pState->counting = 0;
                RECT rc_text = {rc_button_red.left,
                                rc_button_red.top - (int)(rc_button_red.bottom - rc_button_red.top),
                                rc_button_red.left + (int)(100 * scaleX), rc_button_red.top};
                InvalidateRect(hWnd, &rc_text, FALSE);
                rc_text = {rc_button_black.left,
                           rc_button_black.top - (rc_button_black.bottom - rc_button_black.top),
                           rc_button_black.left + (int)(100 * scaleX), rc_button_black.top};
                InvalidateRect(hWnd, &rc_text, FALSE);
                pState->display_size = pState->moves.size();
                InvalidateRect(hWnd, &rc_round_text, FALSE);
            }
            else if (id == ID_Button_Stop)
            {
                State_Info *pState = (State_Info *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
                if (pState->machine_black)
                    ForceTerminate(hWnd, 2);
                if (pState->machine_red)
                    ForceTerminate(hWnd, 1);
                pState->Initialize();
                SetWindowText(pState->hWndButtonStart, pState->running ? L"❚❚" : L"▶");
                SetWindowText(pState->hWndButtonModify, L"↔");
                PaintBoard(hWnd, pState->brd);
                RECT rc_text = {rc_button_red.left,
                                rc_button_red.top - (int)(rc_button_red.bottom - rc_button_red.top),
                                rc_button_red.left + (int)(100 * scaleX), rc_button_red.top};
                InvalidateRect(hWnd, &rc_text, FALSE);
                rc_text = {rc_button_black.left,
                           rc_button_black.top - (rc_button_black.bottom - rc_button_black.top),
                           rc_button_black.left + (int)(100 * scaleX), rc_button_black.top};
                InvalidateRect(hWnd, &rc_text, FALSE);
                pState->game_black_won = pState->game_red_won = 0;
                SetWindowText(hWnd, L"UCCI Judge");
            }
            else if (id == ID_Button_Backmost || id == ID_Button_Back ||
                     id == ID_Button_Front || id == ID_Button_Frontmost)
            {
                State_Info *pState = (State_Info *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
                pState->running = pState->counting = 0;
                pState->reviewing = 1;
                if (!pState->sent)
                    SetWindowText(pState->hWndButtonStart, L"▷");
                if (id == ID_Button_Backmost)
                    pState->display_size = 0;
                else if (id == ID_Button_Back)
                    pState->display_size--;
                else if (id == ID_Button_Front)
                    pState->display_size++;
                else if (id == ID_Button_Frontmost)
                    pState->display_size = pState->moves.size();
                if (pState->display_size > (int)pState->moves.size())
                    pState->display_size = (int)pState->moves.size();
                if (pState->display_size < 0)
                    pState->display_size = 0;
                chessboard brd = pState->brd_ini;
                for (int i = 0; i < pState->display_size; i++)
                    move(brd, pState->moves[i]);
                if (!pState->sent)
                    PaintBoard(hWnd, brd);
                InvalidateRect(hWnd, &rc_round_text, FALSE);
            }
            else if (id == ID_Button_Reverse)
            {
                State_Info *pState = (State_Info *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
                pState->reverse = !pState->reverse;
                PaintBoard(hWnd, pState->brd);
                SetWindowText(pState->hWndButtonReverse, pState->reverse ? L"◒" : L"◓");
                swap(rc_button_black, rc_button_red);
                SetWindowPos(pState->hWndButtonBlack, (HWND)0,
                             rc_button_black.left, rc_button_black.top,
                             rc_button_black.right - rc_button_black.left,
                             rc_button_black.bottom - rc_button_black.top,
                             IsWindowVisible(pState->hWndButtonBlack)
                                 ? SWP_FRAMECHANGED
                                 : SWP_HIDEWINDOW);
                SetWindowPos(pState->hWndButtonRed, (HWND)0,
                             rc_button_red.left, rc_button_red.top,
                             rc_button_red.right - rc_button_red.left,
                             rc_button_red.bottom - rc_button_red.top,
                             IsWindowVisible(pState->hWndButtonRed)
                                 ? SWP_FRAMECHANGED
                                 : SWP_HIDEWINDOW);
                RECT rc_text_black = {rc_button_black.left,
                                      rc_button_black.top - (rc_button_black.bottom - rc_button_black.top),
                                      rc_button_black.left + (int)(100 * scaleX),
                                      rc_button_black.top + (int)(90 * scaleX)},
                     rc_text_red = {rc_button_red.left,
                                    rc_button_red.top - (int)(rc_button_red.bottom - rc_button_red.top),
                                    rc_button_red.left + (int)(100 * scaleX),
                                    rc_button_red.top + (int)(90 * scaleX)};
                InvalidateRect(hWnd, &rc_text_black, FALSE);
                InvalidateRect(hWnd, &rc_text_red, FALSE);
            }
            else if (id == ID_Button_Modify)
            {
                State_Info *pState = (State_Info *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
                if (!pState->running && (brd2str(pState->brd) == ini_state || pState->modifying))
                {
                    pState->modifying = !pState->modifying;
                    pState->counting = 0;
                    pState->hlt_n = 0;
                    pState->state = 1;
                    SetWindowText(pState->hWndButtonModify, pState->modifying ? L"✓" : L"↔");
                    pState->hlt_n = 0;
                    PaintBoard(hWnd, pState->brd);
                }
            }
            else if (id == ID_Button_Show)
            {
                State_Info *pState = (State_Info *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
                pState->showing = !pState->showing;
                SetWindowText(pState->hWndButtonShow, pState->showing ? L"◉" : L"○");
                RECT rc;
                GetClientRect(hWnd, &rc);
                float l = ((rc.bottom - rc.top) - 2 * margin) / 10;
                wchar_t ws[2] = L"9";
                if (pState->reverse)
                    ws[0] = L'0';
                pRenderTarget->BeginDraw();
                for (int i = 0; i < 10; i++)
                {
                    D2D1_RECT_F rect_f = {0, i * l + margin, (float)margin, (i + 1) * l + margin};
                    if (pState->showing)
                        pRenderTarget->DrawText(ws, 1, pTextFormat, rect_f, pBlackBrush);
                    else
                        pRenderTarget->FillRectangle(rect_f, pWhiteBrush);
                    rect_f = {9 * l + margin, i * l + margin, 9 * l + 2 * margin, (i + 1) * l + margin};
                    if (pState->showing)
                        pRenderTarget->DrawText(ws, 1, pTextFormat, rect_f, pBlackBrush);
                    else
                        pRenderTarget->FillRectangle(rect_f, pWhiteBrush);
                    ws[0] = ws[0] + (pState->reverse ? 1 : -1);
                }
                ws[0] = L'a';
                for (int j = 0; j < 9; j++)
                {
                    D2D1_RECT_F rect_f = {j * l + margin, 0, (j + 1) * l + margin, (float)margin};
                    if (pState->showing)
                        pRenderTarget->DrawText(ws, 1, pTextFormat, rect_f, pBlackBrush);
                    else
                        pRenderTarget->FillRectangle(rect_f, pWhiteBrush);
                    rect_f = {j * l + margin, 10 * l + margin, (j + 1) * l + margin, 10 * l + 2 * margin};
                    if (pState->showing)
                        pRenderTarget->DrawText(ws, 1, pTextFormat, rect_f, pBlackBrush);
                    else
                        pRenderTarget->FillRectangle(rect_f, pWhiteBrush);
                    ws[0]++;
                }
                HRESULT hr = pRenderTarget->EndDraw();
                if (!SUCCEEDED(hr))
                    MessageBox(hWnd, L"Fail to end drawing!", L"error", MB_ICONERROR);
            }
            else if (id == ID_Button_Help)
            {
                State_Info *pState = (State_Info *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
                wstring s = L"UCCI运行界面:\n";
                s += L"  点击棋子可移动\n";
                s += L"  选择文件可载入UCCI引擎\n";
                s += L"  点击\"▶ ❚❚\"(回车键)开始/暂停运行\n";
                s += L"  点击\"◒ ◓\"(空格键)反转棋盘\n";
                s += L"  点击\"◼\"初始化界面\n";
                s += L"  点击\"< > << >>\"(左右方向键)复盘\n";
                s += L"  点击\"↔ ✓\"手动修改初始局面\n";
                s += L"  点击\"◉ ○\"显示位置字符串\n";
                s += L"  点击\"↑ ↓\"(上下方向键)调整限定时间";
                if (!pState->running)
                    MessageBox(hWnd, s.c_str(), L"使用说明", MB_OK);
            }
            else if (id == ID_Button_Minus)
            {
                State_Info *pState = (State_Info *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
                if (!pState->running && !pState->sent && pState->go_time > 100)
                    pState->go_time -= 100;
                RECT rc_time = {rc_button_black.left, rc_button_minus.top,
                                rc_button_black.left + (int)(50 * scaleX),
                                rc_button_minus.bottom};
                InvalidateRect(hWnd, &rc_time, FALSE);
            }
            else if (id == ID_Button_Plus)
            {
                State_Info *pState = (State_Info *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
                if (!pState->running && !pState->sent && pState->go_time < 60000)
                    pState->go_time += 100;
                RECT rc_time = {rc_button_black.left, rc_button_minus.top,
                                rc_button_black.left + (int)(50 * scaleX),
                                rc_button_minus.bottom};
                InvalidateRect(hWnd, &rc_time, FALSE);
            }
        }
        SetFocus(hWnd);
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

DWORD WINAPI threadFunction(LPVOID lpParam)
{
    State_Info *pState = (State_Info *)lpParam;
    DWORD dwWritten = 0;
    if ((pState->turn == 1 ? pState->hInWriteRed : pState->hInWriteBlack) == NULL)
        return 0;
    string s_send = pState->s_send + "\r\n";
    HRESULT hr;
    if (!pState->sent)
    {
        hr = WriteFile((pState->turn == 1 ? pState->hInWriteRed : pState->hInWriteBlack),
                       s_send.c_str(), s_send.length(), &dwWritten, NULL);
        if (!SUCCEEDED(hr))
            return -1;
        pState->time_start = GetTickCount64();
        pState->sent = 1;
        pState->s_receive = "";
    }
    char out_buffer[1024] = "";
    DWORD dwRead = 0;
    hr = ReadFile((pState->turn == 1 ? pState->hOutReadRed : pState->hOutReadBlack),
                  out_buffer, 1024, &dwRead, NULL);
    if (!SUCCEEDED(hr))
        return -1;
    pState->s_receive += out_buffer;
    return 0;
}

void Play(HINSTANCE hInstance)
{
    // create window
    HWND hWnd = CreateWindow(
        L"JUDGE", L"UCCI Judge",
        WS_OVERLAPPEDWINDOW & (~WS_MAXIMIZEBOX),
        CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
        NULL, NULL, hInstance, NULL);
    if (hWnd == NULL)
    {
        MessageBox(hWnd, L"Fail to create window!", L"error", MB_ICONERROR);
        return;
    }
    State_Info info;
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)(&info));

    // create buttons
    RECT rc;
    GetClientRect(hWnd, &rc);
    rc_button_black = {(int)(510 * scaleX), (int)((rc.bottom - rc.top) / 4.0f - 15 * scaleY),
                       (int)(600 * scaleX), (int)((rc.bottom - rc.top) / 4.0f + 15 * scaleY)};
    rc_button_red = {(int)(510 * scaleX), (int)((rc.bottom - rc.top) / 4.0f * 3 - 15 * scaleY),
                     (int)(600 * scaleX), (int)((rc.bottom - rc.top) / 4.0f * 3 + 15 * scaleY)};
    rc_button_start = {(int)(510 * scaleX), (int)((rc.bottom - rc.top) / 2.0f - 55 * scaleY),
                       (int)(540 * scaleX), (int)((rc.bottom - rc.top) / 2.0f - 25 * scaleY)};
    rc_button_reverse = {(int)(540 * scaleX), (int)((rc.bottom - rc.top) / 2.0f - 55 * scaleY),
                         (int)(570 * scaleX), (int)((rc.bottom - rc.top) / 2.0f - 25 * scaleY)};
    rc_button_stop = {(int)(570 * scaleX), (int)((rc.bottom - rc.top) / 2.0f - 55 * scaleY),
                      (int)(600 * scaleX), (int)((rc.bottom - rc.top) / 2.0f - 25 * scaleY)};
    rc_button_backmost = {(int)(510 * scaleX), (int)((rc.bottom - rc.top) / 2.0f - 15 * scaleY),
                          (int)(532.5f * scaleX), (int)((rc.bottom - rc.top) / 2.0f + 15 * scaleY)};
    rc_button_back = {(int)(532.5f * scaleX), (int)((rc.bottom - rc.top) / 2.0f - 15 * scaleY),
                      (int)(555 * scaleX), (int)((rc.bottom - rc.top) / 2.0f + 15 * scaleY)};
    rc_button_front = {(int)(555 * scaleX), (int)((rc.bottom - rc.top) / 2.0f - 15 * scaleY),
                       (int)(577.5f * scaleX), (int)((rc.bottom - rc.top) / 2.0f + 15 * scaleY)};
    rc_button_frontmost = {(int)(577.5f * scaleX), (int)((rc.bottom - rc.top) / 2.0f - 15 * scaleY),
                           (int)(600 * scaleX), (int)((rc.bottom - rc.top) / 2.0f + 15 * scaleY)};
    rc_button_modify = {(int)(510 * scaleX), (int)((rc.bottom - rc.top) / 2.0f + 25 * scaleY),
                        (int)(540 * scaleX), (int)((rc.bottom - rc.top) / 2.0f + 55 * scaleY)};
    rc_button_show = {(int)(540 * scaleX), (int)((rc.bottom - rc.top) / 2.0f + 25 * scaleY),
                      (int)(570 * scaleX), (int)((rc.bottom - rc.top) / 2.0f + 55 * scaleY)};
    rc_button_help = {(int)(570 * scaleX), (int)((rc.bottom - rc.top) / 2.0f + 25 * scaleY),
                      (int)(600 * scaleX), (int)((rc.bottom - rc.top) / 2.0f + 55 * scaleY)};
    rc_button_minus = {(int)(560 * scaleX), (int)((rc.bottom - rc.top) / 16.0f * 15.0f - 15 * scaleY),
                       (int)(580 * scaleX), (int)((rc.bottom - rc.top) / 16.0f * 15.0f + 15 * scaleY)};
    rc_button_plus = {(int)(580 * scaleX), (int)((rc.bottom - rc.top) / 16.0f * 15.0f - 15 * scaleY),
                      (int)(600 * scaleX), (int)((rc.bottom - rc.top) / 16.0f * 15.0f + 15 * scaleY)};
    hDefaultFont = CreateFont(20 * scaleY, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
                              CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Segoe UI"));
    if (hDefaultFont == NULL)
    {
        MessageBox(hWnd, L"Fail to create font!", L"error", MB_ICONERROR);
        return;
    }
    info.hWndButtonBlack = CreateWindow(
        L"BUTTON", L"选择文件",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        rc_button_black.left, rc_button_black.top,
        rc_button_black.right - rc_button_black.left,
        rc_button_black.bottom - rc_button_black.top,
        hWnd, (HMENU)ID_Select_Black,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    if (info.hWndButtonBlack == NULL)
    {
        MessageBox(hWnd, L"Fail to create button!", L"error", MB_ICONERROR);
        return;
    }
    SendMessage(info.hWndButtonBlack, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
    info.hWndButtonRed = CreateWindow(
        L"BUTTON", L"选择文件",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        rc_button_red.left, rc_button_red.top,
        rc_button_red.right - rc_button_red.left,
        rc_button_red.bottom - rc_button_red.top,
        hWnd, (HMENU)ID_Select_Red,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    if (info.hWndButtonRed == NULL)
    {
        MessageBox(hWnd, L"Fail to create button!", L"error", MB_ICONERROR);
        return;
    }
    SendMessage(info.hWndButtonRed, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
    info.hWndButtonStart = CreateWindow(
        L"BUTTON", L"▶",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        rc_button_start.left, rc_button_start.top,
        rc_button_start.right - rc_button_start.left,
        rc_button_start.bottom - rc_button_start.top,
        hWnd, (HMENU)ID_Button_Start,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    if (info.hWndButtonStart == NULL)
    {
        MessageBox(hWnd, L"Fail to create button!", L"error", MB_ICONERROR);
        return;
    }
    SendMessage(info.hWndButtonStart, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
    info.hWndButtonReverse = CreateWindow(
        L"BUTTON", L"◓",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        rc_button_reverse.left, rc_button_reverse.top,
        rc_button_reverse.right - rc_button_reverse.left,
        rc_button_reverse.bottom - rc_button_reverse.top,
        hWnd, (HMENU)ID_Button_Reverse,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    if (info.hWndButtonStart == NULL)
    {
        MessageBox(hWnd, L"Fail to create button!", L"error", MB_ICONERROR);
        return;
    }
    SendMessage(info.hWndButtonReverse, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
    HWND hWndButtonStop = CreateWindow(
        L"BUTTON", L"■",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        rc_button_stop.left, rc_button_stop.top,
        rc_button_stop.right - rc_button_stop.left,
        rc_button_stop.bottom - rc_button_stop.top,
        hWnd, (HMENU)ID_Button_Stop,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    if (hWndButtonStop == NULL)
    {
        MessageBox(hWnd, L"Fail to create button!", L"error", MB_ICONERROR);
        return;
    }
    SendMessage(hWndButtonStop, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
    HWND hWndButtonBack = CreateWindow(
        L"BUTTON", L"<",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        rc_button_back.left, rc_button_back.top,
        rc_button_back.right - rc_button_back.left,
        rc_button_back.bottom - rc_button_back.top,
        hWnd, (HMENU)ID_Button_Back,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    if (hWndButtonBack == NULL)
    {
        MessageBox(hWnd, L"Fail to create button!", L"error", MB_ICONERROR);
        return;
    }
    SendMessage(hWndButtonBack, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
    HWND hWndButtonFront = CreateWindow(
        L"BUTTON", L">",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        rc_button_front.left, rc_button_front.top,
        rc_button_front.right - rc_button_front.left,
        rc_button_front.bottom - rc_button_front.top,
        hWnd, (HMENU)ID_Button_Front,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    if (hWndButtonFront == NULL)
    {
        MessageBox(hWnd, L"Fail to create button!", L"error", MB_ICONERROR);
        return;
    }
    SendMessage(hWndButtonFront, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
    HWND hWndButtonBackMost = CreateWindow(
        L"BUTTON", L"<<",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        rc_button_backmost.left, rc_button_backmost.top,
        rc_button_backmost.right - rc_button_backmost.left,
        rc_button_backmost.bottom - rc_button_backmost.top,
        hWnd, (HMENU)ID_Button_Backmost,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    if (hWndButtonBackMost == NULL)
    {
        MessageBox(hWnd, L"Fail to create button!", L"error", MB_ICONERROR);
        return;
    }
    SendMessage(hWndButtonBackMost, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
    HWND hWndButtonFrontMost = CreateWindow(
        L"BUTTON", L">>",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        rc_button_frontmost.left, rc_button_frontmost.top,
        rc_button_frontmost.right - rc_button_frontmost.left,
        rc_button_frontmost.bottom - rc_button_frontmost.top,
        hWnd, (HMENU)ID_Button_Frontmost,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    if (hWndButtonFrontMost == NULL)
    {
        MessageBox(hWnd, L"Fail to create button!", L"error", MB_ICONERROR);
        return;
    }
    SendMessage(hWndButtonFrontMost, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
    info.hWndButtonModify = CreateWindow(
        L"BUTTON", L"↔",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        rc_button_modify.left, rc_button_modify.top,
        rc_button_modify.right - rc_button_modify.left,
        rc_button_modify.bottom - rc_button_modify.top,
        hWnd, (HMENU)ID_Button_Modify,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    if (info.hWndButtonModify == NULL)
    {
        MessageBox(hWnd, L"Fail to create button!", L"error", MB_ICONERROR);
        return;
    }
    SendMessage(info.hWndButtonModify, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
    info.hWndButtonShow = CreateWindow(
        L"BUTTON", L"○",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        rc_button_show.left, rc_button_show.top,
        rc_button_show.right - rc_button_show.left,
        rc_button_show.bottom - rc_button_show.top,
        hWnd, (HMENU)ID_Button_Show,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    if (info.hWndButtonShow == NULL)
    {
        MessageBox(hWnd, L"Fail to create button!", L"error", MB_ICONERROR);
        return;
    }
    SendMessage(info.hWndButtonShow, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
    HWND hWndButtonHelp = CreateWindow(
        L"BUTTON", L"?",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        rc_button_help.left, rc_button_help.top,
        rc_button_help.right - rc_button_help.left,
        rc_button_help.bottom - rc_button_help.top,
        hWnd, (HMENU)ID_Button_Help,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    if (hWndButtonHelp == NULL)
    {
        MessageBox(hWnd, L"Fail to create button!", L"error", MB_ICONERROR);
        return;
    }
    SendMessage(hWndButtonHelp, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
    HWND hWndButtonMinus = CreateWindow(
        L"BUTTON", L"↓",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        rc_button_minus.left, rc_button_minus.top,
        rc_button_minus.right - rc_button_minus.left,
        rc_button_minus.bottom - rc_button_minus.top,
        hWnd, (HMENU)ID_Button_Minus,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    if (hWndButtonMinus == NULL)
    {
        MessageBox(hWnd, L"Fail to create button!", L"error", MB_ICONERROR);
        return;
    }
    SendMessage(hWndButtonMinus, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
    HWND hWndButtonPlus = CreateWindow(
        L"BUTTON", L"↑",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        rc_button_plus.left, rc_button_plus.top,
        rc_button_plus.right - rc_button_plus.left,
        rc_button_plus.bottom - rc_button_plus.top,
        hWnd, (HMENU)ID_Button_Plus,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
    if (hWndButtonPlus == NULL)
    {
        MessageBox(hWnd, L"Fail to create button!", L"error", MB_ICONERROR);
        return;
    }
    SendMessage(hWndButtonPlus, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);

    ShowWindow(hWnd, SW_SHOW);

    MSG msg = {};
    info.hThread = NULL;
    DWORD dwThreadID;
    ULONGLONG t, t_pre = GetTickCount64();
    bool invalid = 0;
    while (TRUE)
    {
        t = GetTickCount64();
        if ((info.counting || info.sent) && t != t_pre)
        {
            if (info.turn == 1)
                info.time_red += t - t_pre;
            if (info.turn == 2)
                info.time_black += t - t_pre;
            if (info.time_red / 1000 != info.time_red_pre / 1000)
            {
                RECT rc_text = {rc_button_red.left,
                                rc_button_red.top - (int)(rc_button_red.bottom - rc_button_red.top),
                                rc_button_red.left + (int)(100 * scaleX), rc_button_red.top};
                InvalidateRect(hWnd, &rc_text, FALSE);
                info.time_red_pre = info.time_red;
            }
            if (info.time_black / 1000 != info.time_black_pre / 1000)
            {
                RECT rc_text = {rc_button_black.left,
                                rc_button_black.top - (rc_button_black.bottom - rc_button_black.top),
                                rc_button_black.left + (int)(100 * scaleX), rc_button_black.top};
                InvalidateRect(hWnd, &rc_text, FALSE);
                info.time_black_pre = info.time_black;
            }
        }
        t_pre = t;
        if (!info.running && !info.sent)
        {
            wchar_t buttonText[4];
            GetWindowText(info.hWndButtonStart, buttonText, -1);
            wstring s_tmp = buttonText;
            if (info.over || info.reviewing)
            {
                if (s_tmp != L"▷")
                    SetWindowText(info.hWndButtonStart, L"▷");
            }
            else
            {
                if (s_tmp != L"▶")
                    SetWindowText(info.hWndButtonStart, L"▶");
            }
            TerminateThread(info.hThread, 0);
            info.hThread = NULL;
            info.sent = 0;
            invalid = 0;
        }
        else if ((info.turn == 2 && info.machine_black || info.turn == 1 && info.machine_red))
        {
            if (info.hThread == NULL)
            {
                invalid = 0;
                info.hThread = CreateThread(NULL, 0, threadFunction, &info, 0, &dwThreadID);
                if (info.hThread == NULL)
                {
                    MessageBox(hWnd, L"Fail to create thread!", L"error", MB_ICONERROR);
                    continue;
                }
            }
            if (WaitForSingleObject(info.hThread, 1) == WAIT_OBJECT_0)
            {
                if (info.turn == 1 && !info.isreadyRed || info.turn == 2 && !info.isreadyBlack)
                {
                    if (info.s_send == "ucci")
                    {
                        if (info.s_receive.find("ucciok") != info.s_receive.npos)
                        {
                            info.s_send = "isready";
                            info.sent = 0;
                        }
                    }
                    else if (info.s_send == "isready")
                    {
                        if (info.s_receive.find("readyok") != info.s_receive.npos)
                        {
                            info.sent = 0;
                            if (info.turn == 1)
                                info.isreadyRed = 1;
                            else
                                info.isreadyBlack = 1;
                            UpdateFenString(&info);
                        }
                    }
                }
                else
                {
                    int suc;
                    char tmp[32], tmp_move[32];
                    size_t pos = info.s_receive.find("bestmove");
                    if (pos == info.s_receive.npos)
                        suc = 0;
                    else
                    {
                        string s_tmp = info.s_receive.substr(pos);
                        sscanf(s_tmp.c_str(), "%s%s", tmp, tmp_move);
                        PaintBoard(hWnd, info.brd, 2, info.hlt, 0, NULL);
                        movement mov = str2mov(tmp_move);
                        info.hlt[0] = mov.b;
                        info.hlt[1] = mov.e;
                        suc = mov.valid ? ImplementMove(hWnd) : 0;
                    }
                    if (suc)
                        UpdateFenString(&info);
                    else if (info.s_receive.find("nobestmove") != info.s_receive.npos)
                    {
                        info.over = 1;
                        info.running = info.counting = info.sent = 0;
                        wstring s = (info.turn == 1 ? L"黑" : L"红");
                        s = s + L"方赢! " + L"(对方认输)";
                        (info.turn == 1 ? info.game_black_won : info.game_red_won)++;
                        MessageBox(hWnd, s.c_str(), L"本局结束", MB_OK);
                        if (info.game_red_won + info.game_black_won > 0)
                        {
                            wstring title = L"UCCI Judge";
                            title += L" (";
                            char s_tmp[16];
                            wchar_t ws_tmp[16];
                            itoa(info.game_red_won, s_tmp, 10);
                            MultiByteToWideChar(CP_ACP, 0, s_tmp, -1, ws_tmp, 16);
                            title += ws_tmp;
                            itoa(info.game_black_won, s_tmp, 10);
                            MultiByteToWideChar(CP_ACP, 0, s_tmp, -1, ws_tmp, 16);
                            title += L" : ";
                            title += ws_tmp;
                            title += L")";
                            SetWindowText(hWnd, title.c_str());
                        }
                    }
                    else
                        // input not valid for now
                        // if still not valid it will end the game
                        invalid = 1;
                }
                info.hThread = NULL;
                CloseHandle(info.hThread);
            }
            else if (info.running || info.sent)
            {
                ULONGLONG time_gone = GetTickCount64() - info.time_start;
                if (info.s_send == "ucci" && time_gone > info.time_limit && info.sent)
                {
                    MessageBox(hWnd, L"请载入正确的UCCI引擎", L"引擎错误", MB_ICONEXCLAMATION);
                    info.running = info.counting = info.sent = 0;
                }
                else if (info.s_send == "isready" && time_gone > info.time_limit && info.sent)
                {
                    MessageBox(hWnd, L"准备引擎无响应", L"引擎错误", MB_ICONEXCLAMATION);
                    info.running = info.counting = info.sent = 0;
                }
                else if ((LONGLONG)time_gone > (LONGLONG)(info.go_time + SPARE_TIME) && info.sent)
                {
                    info.over = 1;
                    info.running = info.counting = info.sent = 0;
                    wstring s = (info.turn == 1 ? L"黑" : L"红");
                    s = s + L"方赢! " + (invalid ? L"(对方响应错误)" : L"(对方超时)");
                    MessageBox(hWnd, s.c_str(), L"本局结束", MB_OK);
                    (info.turn == 1 ? info.game_black_won : info.game_red_won)++;
                    wstring title = L"UCCI Judge";
                    if (info.game_red_won + info.game_black_won > 0)
                    {
                        title += L" (";
                        char s_tmp[16];
                        wchar_t ws_tmp[16];
                        itoa(info.game_red_won, s_tmp, 10);
                        MultiByteToWideChar(CP_ACP, 0, s_tmp, -1, ws_tmp, 16);
                        title += ws_tmp;
                        itoa(info.game_black_won, s_tmp, 10);
                        MultiByteToWideChar(CP_ACP, 0, s_tmp, -1, ws_tmp, 16);
                        title += L" : ";
                        title += ws_tmp;
                        title += L")";
                        SetWindowText(hWnd, title.c_str());
                    }
                    info.time_start = GetTickCount64();
                }
            }
        }
        WINBOOL ava = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
        if (msg.message == WM_QUIT)
            break;
        else if (ava)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    TerminateProcess(info.piProcInfoBlack.hProcess, 0);
    TerminateProcess(info.piProcInfoRed.hProcess, 0);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, PWSTR pCmdLine, int nCmdShow)
{
    HDC hdc = GetDC(0);
    scaleX = (double)GetDeviceCaps(hdc, DESKTOPHORZRES) / GetDeviceCaps(hdc, HORZRES);
    scaleY = (double)GetDeviceCaps(hdc, DESKTOPVERTRES) / GetDeviceCaps(hdc, VERTRES);
    SetProcessDPIAware();
    margin *= scaleX;

    // register window class
    WNDCLASS wc = {};
    wc.lpszClassName = L"JUDGE";
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    CoInitialize(NULL);

    Play(hInstance);

    return 0;
}