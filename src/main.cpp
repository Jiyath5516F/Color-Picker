#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "app.h"

int WINAPI wWinMain(_In_     HINSTANCE hInstance,
                    _In_opt_ HINSTANCE ,
                    _In_     LPWSTR    ,
                    _In_     int       )
{
    
    SetProcessDPIAware();

    App app(hInstance);

    if (!app.Initialise()) {
        MessageBoxW(nullptr,
                    L"Failed to initialise Color Picker.",
                    L"Error", MB_ICONERROR);
        return 1;
    }

    return app.Run();
}
