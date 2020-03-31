/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Class for holding HSV color model data.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "hsv.h"
#include "rgb.h"
#include <algorithm>

HSVClass const HSVClass::s_blackColor(0, 0, 0);
HSVClass const HSVClass::s_whiteColor(0, 0, 100);

/**
 * @brief Carries out an adjustment based on another HSV value.
 *
 * 0x005D3C2C
 */
void HSVClass::Adjust(int adjust, HSVClass const &that)
{
    m_Val += adjust * (that.m_Val - m_Val) / 256;
    m_Sat += adjust * (that.m_Sat - m_Sat) / 256;
    m_Hue += adjust * (that.m_Hue - m_Hue) / 256;
}

/**
 * @brief Carries out an adjustment based on brightness, saturation, tint and contrast.
 */
void HSVClass::Adjust(fixed_t brightness, fixed_t saturation, fixed_t tint, fixed_t contrast)
{
    // int v10 = std::clamp(((brightness * 256) * m_val) / 128, 0, 255);
    // int tmp = ((std::clamp(((brightness * 256) * m_val) / 128, 0, 255) - 128) * (contrast * 256));
    // int v12 = (tmp / 128) + 128;

    int v = std::clamp(
        (((std::clamp(((brightness * 256) * m_Val) / 128, 0, 255) - 128) * (contrast * 256)) / 128) + 128, 0, 255);
    int s = std::clamp(((saturation * 256) * m_Sat) / 128, 0, 255);
    int h = std::clamp(((tint * 256) * m_Hue) / 128, 0, 255);

    m_Hue = h;
    m_Sat = s;
    m_Val = v;
}

/**
 * @brief Calculates a numerical distance between this hsv value and another.
 *
 * 0x005D3CA4
 */
int const HSVClass::Difference(HSVClass const &that) const
{
    int hue = std::abs(m_Hue - that.m_Hue);
    int sat = std::abs(m_Sat - that.m_Sat);
    int val = std::abs(m_Val - that.m_Val);

    return hue * hue + sat * sat + val * val;
}

/**
 * @brief Conversion operator to create an RGB value based on this HSV value.
 *
 * 0x005D3CFC
 */
HSVClass::operator RGBClass() const
{
    if (m_Sat == 0) {
        // Achromatic (grey).
        return RGBClass(m_Val, m_Val, m_Val);
    }

    int sat = m_Sat;
    int val = m_Val;
    int hue = m_Hue * 6; // Scale to full range.
    int q = (hue / 255);
    int rem = hue % 255;

    int values[7];
    values[1] = val;
    values[2] = val;
    values[3] = val * (255 - rem * sat / 255) / 255;
    values[4] = val * (255 - sat) / 255;
    values[5] = values[4];
    values[6] = (255 - sat * (255 - rem) / 255) * val / 255;

    q += (q > 4 ? -4 : 2);
    int red = values[q];

    q += (q > 4 ? -4 : 2);
    int blue = values[q];

    q += (q > 4 ? -4 : 2);
    int green = values[q];

    return RGBClass(red, green, blue);
}

/**
 * @brief Sets the index in the game palette to this HSV value.
 *
 * 0x005D3CFC
 */
void const HSVClass::Set(uint8_t index) const
{
    RGBClass rgb = *this; // should call operator RGBClass;
    rgb.Set(index);
}
