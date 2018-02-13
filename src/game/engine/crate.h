/**
 * @file
 *
 * @author OmniBlade
 * @author CCHyper
 *
 * @brief Handles bonus crates.
 *
 * @copyright Redalert++ is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef CRATE_H
#define CRATE_H

#include "always.h"
#include "ttimer.h"

enum CrateType
{
    CRATE_NONE = -1,
    CRATE_MONEY,
    CRATE_UNIT,
    CRATE_PARA_BOMB,
    CRATE_HEAL_BASE,
    CRATE_CLOAK,
    CRATE_EXPLOSION,
    CRATE_NAPALM,
    CRATE_SQUAD,
    CRATE_DARKNESS,
    CRATE_REVEAL,
    CRATE_SONAR,
    CRATE_ARMOR,
    CRATE_SPEED,
    CRATE_FIREPOWER,
    CRATE_ICBM,
    CRATE_TIME_QUAKE,
    CRATE_INVULNERABILITY,
    CRATE_CHRONALVORTEX,
    CRATE_COUNT,
};

class CrateClass
{
public:
    CrateClass() : Cell(-1) {}

    BOOL Remove_It();
    BOOL Create_Crate(int16_t cell);
    int16_t Get_Cell() { return Cell; }
    BOOL Timer_Expired() { return CrateTimer.Time() <= 0; }

    static BOOL Put_Crate(int16_t &cell);
    static BOOL Get_Crate(int16_t cell);

private:
    TCountDownTimerClass<FrameTimerClass> CrateTimer;
    int16_t Cell;
};

#endif // CRATE_H