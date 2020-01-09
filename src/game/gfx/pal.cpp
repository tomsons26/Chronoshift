/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Low level palette handling.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "pal.h"
#include "initvideo.h"
#include <algorithm>
#include <cstring>

using std::memcpy;

#ifndef GAME_DLL
uint8_t g_CurrentPalette[768];
#endif

void Set_Palette(void *pal)
{
    memcpy(g_CurrentPalette, pal, 768);
    Set_Video_Palette(pal);
}

void Increase_Palette_Luminance(uint8_t *pal, int red, int green, int blue, int min)
{
  for ( int i = 0; i < 768; i += 3 ) {
    pal[i + 0] = std::min(min, red   * pal[i + 0] / 100 + pal[i + 0]);
    pal[i + 1] = std::min(min, green * pal[i + 1] / 100 + pal[i + 1]);
    pal[i + 2] = std::min(min, blue  * pal[i + 2] / 100 + pal[i + 2]);
  }
}
