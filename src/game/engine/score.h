/**
 * @file
 *
 * @author tomsons26
 *
 * @brief
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef SCORE_H
#define SCORE_H

#include "always.h"
#include "palette.h"
#include "ttimer.h"

class ScoreClass
{
public:
    ScoreClass();
    ~ScoreClass() {}
    void Code_Pointers();
    void Decode_Pointers();
    void Presentation();
    void Do_BadGuy_Buildings_Graph();
    void Do_Graph(void * bluebar, void * redbar, int good_lost, int bad_lost, int ypos);
    void Do_BadGuy_Casualties_Graph();
    void Show_Credits(int side, uint8_t *palette);
    void Print_Minutes(int value);
    void Count_Up_Print(char * format, int value1, int value2, int xpos, int ypos);
    void Input_Name(char *name, int length, int, uint8_t *palette);
    void Init();

private:
    int m_field_0;
    int m_BadUnitsLost;
    int m_GoodUnitsLost;
    int m_SomeUnitsLost;
    int m_BadBuildingsLost;
    int m_GoodBuildingsLost;
    int m_SomeBuildingsLost;
    int m_field_1C;
    int m_field_20;
    int m_field_24;
    int m_field_28;
    TTimerClass<SystemTimerClass> m_Timer;
    int m_field_35;
};

void Cycle_Wait_Click(int);

#endif //SCORE_H
