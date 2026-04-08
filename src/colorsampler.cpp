#include "colorsampler.h"

ColorSampler::ColorSampler()
    : m_screenDC(GetDC(nullptr))
{
}

ColorSampler::~ColorSampler()
{
    if (m_screenDC)
        ReleaseDC(nullptr, m_screenDC);
}

ColorInfo ColorSampler::Sample() const
{
    ColorInfo info{};

    GetCursorPos(&info.position);

    info.color = GetPixel(m_screenDC,
                          info.position.x,
                          info.position.y);

    info.r = GetRValue(info.color);
    info.g = GetGValue(info.color);
    info.b = GetBValue(info.color);

    return info;
}
