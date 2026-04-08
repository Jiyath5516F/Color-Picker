#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "colorsampler.h"
#include "trayicon.h"

#define WM_TRAYICON       (WM_USER + 1)
#define HOTKEY_SHOW_ID    1

class App
{
public:
    explicit App(HINSTANCE hInstance);
    ~App();

    App(const App&)            = delete;
    App& operator=(const App&) = delete;

    bool  Initialise();
    int   Run();

private:
    
    static LRESULT CALLBACK
        WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    LRESULT  HandleMessage(HWND hwnd, UINT msg,
                           WPARAM wp, LPARAM lp);

    
    void  OnPaint      (HWND hwnd);
    void  OnTimer      (HWND hwnd);
    void  OnTrayIcon   (HWND hwnd, WPARAM wp, LPARAM lp);
    void  OnCommand    (HWND hwnd, int id);

    
    void  MinimiseToTray   (HWND hwnd);
    void  RestoreFromTray  (HWND hwnd);

    
    void  DrawColourPreview (HDC hdc, int x, int y, int size);
    void  DrawLabel         (HDC hdc, const RECT& rc,
                             const wchar_t* text,
                             COLORREF colour, HFONT font,
                             UINT fmt = DT_LEFT | DT_VCENTER |
                                        DT_SINGLELINE);

    
    HINSTANCE     m_hInstance;
    HWND          m_hwnd;
    ColorSampler  m_sampler;
    TrayIcon      m_tray;
    ColorInfo     m_currentColour;

    HFONT         m_fontTitle;
    HFONT         m_fontNormal;
    HFONT         m_fontBold;
    HFONT         m_fontSmall;

    bool          m_minimised;

    
    static constexpr int  kWindowW       = 310;
    static constexpr int  kWindowH       = 160;
    static constexpr int  kTimerID       = 1;
    static constexpr int  kTimerMs       = 50;
    static constexpr int  kPad           = 16;
    static constexpr int  kPreviewSize   = 100;
};
