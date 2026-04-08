#pragma once

#ifndef _WIN32_IE
#define _WIN32_IE  0x0600
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

class TrayIcon
{
public:
    TrayIcon();
    ~TrayIcon();

    TrayIcon(const TrayIcon&)            = delete;
    TrayIcon& operator=(const TrayIcon&) = delete;

    bool  Create(HWND hwnd, UINT callbackMsg,
                 HICON icon, const wchar_t* tooltip);
    void  Remove();

private:
    NOTIFYICONDATAW  m_nid;
    bool             m_created;
};
