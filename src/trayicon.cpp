#include "trayicon.h"

#include <cstring>

TrayIcon::TrayIcon()
    : m_nid{}
    , m_created(false)
{
}

TrayIcon::~TrayIcon()
{
    Remove();
}

bool TrayIcon::Create(HWND        hwnd,
                      UINT        callbackMsg,
                      HICON       icon,
                      const wchar_t* tooltip)
{
    m_nid.cbSize           = sizeof(NOTIFYICONDATAW);
    m_nid.hWnd             = hwnd;
    m_nid.uID              = 1;
    m_nid.uFlags           = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = callbackMsg;
    m_nid.hIcon            = icon;

    wcscpy_s(m_nid.szTip, tooltip);

    m_created = (Shell_NotifyIconW(NIM_ADD, &m_nid) != FALSE);
    return m_created;
}

void TrayIcon::Remove()
{
    if (m_created) {
        Shell_NotifyIconW(NIM_DELETE, &m_nid);
        m_created = false;
    }
}
