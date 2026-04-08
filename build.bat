@echo off
setlocal enabledelayedexpansion

echo.
echo  ========================================
echo   Color Picker  -  Build Script
echo  ========================================
echo.

:: ── Ensure output directory exists ──────────────────────────────────
if not exist build  mkdir build

:: ── Step 1 : Generate the application icon ──────────────────────────
echo  [1/3]  Generating icon ...
python tools\generate_icon.py
if errorlevel 1 (
    echo.
    echo  ERROR  Python failed to generate the icon.
    echo         Make sure Python 3 is in your PATH.
    exit /b 1
)
echo.

:: ── Step 2 : Compile the resource script ────────────────────────────
echo  [2/3]  Compiling resources ...
rc /nologo /I src /fo build\app.res res\app.rc
if errorlevel 1 (
    echo.
    echo  ERROR  Resource compilation failed.
    exit /b 1
)
echo.

:: ── Step 3 : Compile and link the application ──────────────────────
echo  [3/3]  Compiling application ...
cl /nologo /O2 /EHsc /W4 /std:c++17 /DUNICODE /D_UNICODE ^
   /I src                              ^
   src\main.cpp                        ^
   src\app.cpp                         ^
   src\colorsampler.cpp                ^
   src\trayicon.cpp                    ^
   build\app.res                       ^
   /Fe:build\ColorPicker.exe           ^
   /Fo:build\                          ^
   /link /SUBSYSTEM:WINDOWS            ^
   user32.lib gdi32.lib shell32.lib kernel32.lib
if errorlevel 1 (
    echo.
    echo  ERROR  Compilation failed.
    exit /b 1
)

:: ── Clean up intermediate object files ──────────────────────────────
del build\*.obj  2>nul

echo.
echo  ========================================
echo   BUILD SUCCESSFUL
echo   Output:  build\ColorPicker.exe
echo  ========================================
echo.
