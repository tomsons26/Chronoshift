/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Class for holding RGB color model data.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "rgb.h"
#include "hsv.h"
#include "palette.h"
#include <algorithm>
#include <captainslog.h>

RGBClass const RGBClass::BlackColor(RGB_MIN, RGB_MIN, RGB_MIN);
RGBClass const RGBClass::WhiteColor(RGB_MAX, RGB_MAX, RGB_MAX);

void RGBClass::Adjust(int adjust, const RGBClass &that)
{
    m_Red += adjust * (that.m_Red - m_Red) / 256;
    m_Grn += adjust * (that.m_Grn - m_Grn) / 256;
    m_Blu += adjust * (that.m_Blu - m_Blu) / 256;
}

int const RGBClass::Difference(RGBClass const &that) const
{
    int red = std::abs(m_Red - that.m_Red);
    int grn = std::abs(m_Grn - that.m_Grn);
    int blu = std::abs(m_Blu - that.m_Blu);

    return red * red + grn * grn + blu * blu;
}

RGBClass RGBClass::Average(RGBClass const &that) const
{
    int red = m_Red + that.m_Red;
    int grn = m_Grn + that.m_Grn;
    int blu = m_Blu + that.m_Blu;

    return RGBClass((red / 2), (grn / 2), (blu / 2));
}

void RGBClass::Set(int index)
{
    PaletteClass::CurrentPalette[index] = *this;
}

RGBClass::operator HSVClass()
{
#define HSV_RED (0 * 256) // red         hue = 0
#define HSV_YLW (1 * 256) // yellow      hue = 256
#define HSV_GRN (2 * 256) // green       hue = 512
#define HSV_CYN (3 * 256) // cyan        hue = 768
#define HSV_BLU (4 * 256) // blue        hue = 1024
#define HSV_MAG (5 * 256) // magenta     hue = 1280
#define HSV_RNG (6 * 256) // range / red hue = 1536 / 0

    int red = Get_Red();
    int grn = Get_Green();
    int blu = Get_Blue();

    int hue = 0;

    // Max component value.
    int val = std::max(blu, std::max(grn, red));

    // Min component value.
    int min_val = std::min(blu, std::min(grn, red));

    // Component range.
    int range = val - min_val;

    int sat = 0;

    // Compute saturation.
    if (val > 0) {
        sat = 255 * range / val;
    }

    // Compute hue.
    if (sat > 0) {
        unsigned int h = range;
        unsigned int r = 255 * (val - red) / range;
        unsigned int g = 255 * (val - grn) / range;
        unsigned int b = 255 * (val - blu) / range;

        if (val == red) {
            if (min_val == grn) {
                h = b + HSV_MAG;
            } else {
                h = HSV_YLW - g;
            }
        } else if (val == grn) {
            if (min_val == blu) {
                h = r + HSV_YLW;
            } else {
                h = HSV_CYN - b;
            }
        } else if (min_val == red) {
            h = g + HSV_CYN;
        } else {
            h = HSV_MAG - r;
        }

        hue = h / 6;
    }

    return HSVClass(hue, sat, val);
}
