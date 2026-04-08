#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct ColorInfo
{
    BYTE      r;
    BYTE      g;
    BYTE      b;
    POINT     position;
    COLORREF  color;
};

class ColorSampler
{
public:
    ColorSampler();
    ~ColorSampler();

    ColorSampler(const ColorSampler&)            = delete;
    ColorSampler& operator=(const ColorSampler&) = delete;

    ColorInfo  Sample() const;

private:
    HDC  m_screenDC;
};
