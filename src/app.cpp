#include "app.h"
#include "resource.h"

#include <cstdio>
#include <shellapi.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")

namespace ui {
    constexpr COLORREF  kBg           = RGB(248, 249, 250);
    constexpr COLORREF  kTextPrimary  = RGB( 33,  37,  41);
    constexpr COLORREF  kTextSecond   = RGB(108, 117, 125);
    constexpr COLORREF  kTextMuted    = RGB(173, 181, 189);
    constexpr COLORREF  kBorder       = RGB(222, 226, 230);
}

App::App(HINSTANCE hInstance)
    : m_hInstance    (hInstance)
    , m_hwnd         (nullptr)
    , m_currentColour{}
    , m_fontTitle    (nullptr)
    , m_fontNormal   (nullptr)
    , m_fontBold     (nullptr)
    , m_fontSmall    (nullptr)
    , m_minimised    (false)
{
}

App::~App()
{
    if (m_fontTitle)   DeleteObject(m_fontTitle);
    if (m_fontNormal)  DeleteObject(m_fontNormal);
    if (m_fontBold)    DeleteObject(m_fontBold);
    if (m_fontSmall)   DeleteObject(m_fontSmall);
    UnregisterHotKey(m_hwnd, HOTKEY_SHOW_ID);
}

bool App::Initialise()
{
    
    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance      = m_hInstance;
    wc.hIcon         = LoadIconW(m_hInstance,
                                 MAKEINTRESOURCEW(IDI_APP_ICON));
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;          
    wc.lpszClassName = L"ColorPickerWnd";
    wc.hIconSm       = wc.hIcon;

    if (!RegisterClassExW(&wc))
        return false;

    
    constexpr DWORD style   = WS_OVERLAPPED | WS_CAPTION
                            | WS_SYSMENU    | WS_MINIMIZEBOX;
    constexpr DWORD exStyle = WS_EX_TOPMOST;

    RECT rc = { 0, 0, kWindowW, kWindowH };
    AdjustWindowRectEx(&rc, style, FALSE, exStyle);

    const int winW = rc.right  - rc.left;
    const int winH = rc.bottom - rc.top;
    const int posX = (GetSystemMetrics(SM_CXSCREEN) - winW) / 2;
    const int posY = (GetSystemMetrics(SM_CYSCREEN) - winH) / 2;

    
    m_hwnd = CreateWindowExW(
        exStyle, L"ColorPickerWnd", L"Color Picker",
        style,
        posX, posY, winW, winH,
        nullptr, nullptr, m_hInstance,
        this                                
    );
    if (!m_hwnd)
        return false;

    
    auto makeFont = [](int size, int weight) -> HFONT {
        return CreateFontW(
            -size, 0, 0, 0, weight,
            FALSE, FALSE, FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_SWISS,
            L"Segoe UI"
        );
    };
    m_fontTitle  = makeFont(18, FW_BOLD);
    m_fontNormal = makeFont(14, FW_NORMAL);
    m_fontBold   = makeFont(14, FW_BOLD);
    m_fontSmall  = makeFont(11, FW_NORMAL);

    
    HICON trayIcon = LoadIconW(m_hInstance,
                               MAKEINTRESOURCEW(IDI_APP_ICON));
    m_tray.Create(m_hwnd, WM_TRAYICON, trayIcon, L"Color Picker");

    
    RegisterHotKey(m_hwnd, HOTKEY_SHOW_ID,
                   MOD_CONTROL | MOD_SHIFT, 'C');

    
    SetTimer(m_hwnd, kTimerID, kTimerMs, nullptr);

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
    return true;
}

int App::Run()
{
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK App::WndProc(HWND hwnd, UINT msg,
                               WPARAM wp, LPARAM lp)
{
    App* self = nullptr;

    if (msg == WM_NCCREATE) {
        auto cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self    = reinterpret_cast<App*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA,
                          reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<App*>(
                   GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->HandleMessage(hwnd, msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT App::HandleMessage(HWND hwnd, UINT msg,
                            WPARAM wp, LPARAM lp)
{
    switch (msg) {

    
    case WM_PAINT:
        OnPaint(hwnd);
        return 0;

    
    case WM_TIMER:
        if (wp == kTimerID) OnTimer(hwnd);
        return 0;

    
    case WM_HOTKEY:
        if (wp == HOTKEY_SHOW_ID) {
            if (m_minimised) RestoreFromTray(hwnd);
            else             SetForegroundWindow(hwnd);
        }
        return 0;

    
    case WM_TRAYICON:
        OnTrayIcon(hwnd, wp, lp);
        return 0;

    
    case WM_COMMAND:
        OnCommand(hwnd, LOWORD(wp));
        return 0;

    
    case WM_SYSCOMMAND:
        if ((wp & 0xFFF0) == SC_MINIMIZE) {
            MinimiseToTray(hwnd);
            return 0;
        }
        break;

    
    case WM_CLOSE:
        MinimiseToTray(hwnd);
        return 0;

    
    case WM_DESTROY:
        KillTimer(hwnd, kTimerID);
        m_tray.Remove();
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wp, lp);
}

void App::OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    RECT client;
    GetClientRect(hwnd, &client);
    const int w = client.right;
    const int h = client.bottom;

    
    HDC     mem    = CreateCompatibleDC(hdc);
    HBITMAP bmp    = CreateCompatibleBitmap(hdc, w, h);
    HBITMAP oldBmp = static_cast<HBITMAP>(SelectObject(mem, bmp));

    
    HBRUSH bgBr = CreateSolidBrush(ui::kBg);
    FillRect(mem, &client, bgBr);
    DeleteObject(bgBr);

    SetBkMode(mem, TRANSPARENT);

    
    DrawColourPreview(mem, kPad, kPad, kPreviewSize);

    
    wchar_t hex[16];
    swprintf_s(hex, L"#%02X%02X%02X",
               m_currentColour.r, m_currentColour.g, m_currentColour.b);

    const int textX = kPad + kPreviewSize + 14;
    RECT rcHex = { textX, kPad + 2, w - kPad, kPad + 26 };
    DrawLabel(mem, rcHex, hex, ui::kTextPrimary, m_fontTitle);

    
    wchar_t rgb[40];
    swprintf_s(rgb, L"rgb(%d, %d, %d)",
               m_currentColour.r, m_currentColour.g, m_currentColour.b);

    RECT rcRgb = { textX, kPad + 32, w - kPad, kPad + 54 };
    DrawLabel(mem, rcRgb, rgb, ui::kTextSecond, m_fontBold);

    
    wchar_t pos[40];
    swprintf_s(pos, L"pos  (%d, %d)",
               m_currentColour.position.x, m_currentColour.position.y);

    RECT rcPos = { textX, kPad + 62, w - kPad, kPad + 80 };
    DrawLabel(mem, rcPos, pos, ui::kTextSecond, m_fontSmall);

    
    RECT rcFoot = { kPad, h - 28, w - kPad, h - 8 };
    DrawLabel(mem, rcFoot,
              L"Ctrl+Shift+C  toggle",
              ui::kTextMuted, m_fontSmall,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    
    BitBlt(hdc, 0, 0, w, h, mem, 0, 0, SRCCOPY);
    SelectObject(mem, oldBmp);
    DeleteObject(bmp);
    DeleteDC(mem);

    EndPaint(hwnd, &ps);
}

void App::DrawColourPreview(HDC hdc, int x, int y, int sz)
{
    HPEN   pen = CreatePen(PS_SOLID, 1, ui::kBorder);
    HBRUSH br  = CreateSolidBrush(RGB(m_currentColour.r,
                                      m_currentColour.g,
                                      m_currentColour.b));
    HPEN   oldP = static_cast<HPEN>  (SelectObject(hdc, pen));
    HBRUSH oldB = static_cast<HBRUSH>(SelectObject(hdc, br));

    RoundRect(hdc, x, y, x + sz, y + sz, 10, 10);

    SelectObject(hdc, oldB);
    SelectObject(hdc, oldP);
    DeleteObject(br);
    DeleteObject(pen);
}

void App::DrawLabel(HDC hdc, const RECT& rc,
                    const wchar_t* text,
                    COLORREF colour, HFONT font, UINT fmt)
{
    HFONT old = static_cast<HFONT>(SelectObject(hdc, font));
    SetTextColor(hdc, colour);
    RECT r = rc;                          
    DrawTextW(hdc, text, -1, &r, fmt);
    SelectObject(hdc, old);
}

void App::OnTimer(HWND hwnd)
{
    ColorInfo c = m_sampler.Sample();

    bool changed = (c.r != m_currentColour.r  ||
                    c.g != m_currentColour.g  ||
                    c.b != m_currentColour.b  ||
                    c.position.x != m_currentColour.position.x ||
                    c.position.y != m_currentColour.position.y);

    if (changed) {
        m_currentColour = c;
        if (!m_minimised)
            InvalidateRect(hwnd, nullptr, FALSE);
    }
}

void App::OnTrayIcon(HWND hwnd, WPARAM , LPARAM lp)
{
    switch (LOWORD(lp)) {

    case WM_LBUTTONDBLCLK:
        RestoreFromTray(hwnd);
        break;

    case WM_RBUTTONUP: {
        POINT pt;
        GetCursorPos(&pt);

        HMENU menu = CreatePopupMenu();
        AppendMenuW(menu, MF_STRING, IDM_TRAY_SHOW, L"Show Color Picker");
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(menu, MF_STRING, IDM_TRAY_EXIT, L"Exit");

        SetForegroundWindow(hwnd);
        TrackPopupMenu(menu, TPM_RIGHTBUTTON,
                       pt.x, pt.y, 0, hwnd, nullptr);
        DestroyMenu(menu);
        break;
    }
    }
}

void App::OnCommand(HWND hwnd, int id)
{
    switch (id) {
    case IDM_TRAY_SHOW:  RestoreFromTray(hwnd);  break;
    case IDM_TRAY_EXIT:  DestroyWindow(hwnd);    break;
    }
}

void App::MinimiseToTray(HWND hwnd)
{
    ShowWindow(hwnd, SW_HIDE);
    m_minimised = true;
}

void App::RestoreFromTray(HWND hwnd)
{
    ShowWindow(hwnd, SW_SHOW);
    ShowWindow(hwnd, SW_RESTORE);
    SetForegroundWindow(hwnd);
    m_minimised = false;
}

