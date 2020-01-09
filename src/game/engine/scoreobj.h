/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Classes for "objects" on the score screen.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef SCOREOBJ_H
#define SCOREOBJ_H

#include "always.h"
#include "palette.h"
#include "gbuffer.h"
#include "ttimer.h"
#include "language.h"
#include "hooker.h"

class ScoreAnimClass
{
public:
    ScoreAnimClass(int xpos, int ypos, void *data);

    virtual void Update();
    virtual ~ScoreAnimClass();

protected:
    int m_XPos;
    int m_YPos;
    TCountDownTimerClass<SystemTimerClass> m_Timer;
    uint8_t *m_Data; // reused in different ways
};

class ScoreTimeClass : public ScoreAnimClass
{
public:
    ScoreTimeClass(int xpos, int ypos, void *data, int framecount, int delay);

    virtual void Update() final;
    virtual ~ScoreTimeClass();

private:
    int m_Frame;
    int m_FrameCount;
    int m_Delay; //delay before executing the Update loop
};

class ScoreCredsClass : public ScoreAnimClass
{
public:
    ScoreCredsClass(int xpos, int ypos, void *data, int framecount, int delay);

    virtual void Update() final;
    virtual ~ScoreCredsClass();

private:
    int m_Frame;
    int m_FrameCount;
    int m_Delay; //delay before executing the Update loop
    void *m_CashSound;
    void *m_ClockSound;
};

class ScorePrintClass : public ScoreAnimClass
{
public:
    ScorePrintClass(TextEnum str_id, int xpos, int ypos, uint8_t *palette, int a6);
    ScorePrintClass(void *data, int xpos, int ypos, uint8_t *palette, int a6);

    virtual void Update() final;
    virtual ~ScorePrintClass();

private:
    int m_field_19;
    int m_NextEntry;
    uint8_t *m_Palette;
};

class ScoreScaleClass : public ScoreAnimClass
{
public:
    ScoreScaleClass(void *data, int xpos, int ypos, uint8_t *palette);

    virtual void Update() final;
    virtual ~ScoreScaleClass();

private:
    int m_DestIndex;
    uint8_t *m_Palette;
};

int Alloc_Object(ScoreAnimClass *obj);
void Update_Objects();
void Clear_Objects();
void Dealloc_Objects();
void Animate_Score_Objs();
char *Int_Print(int intval);

#ifdef GAME_DLL
extern void *&g_Beepy6;
extern ScoreAnimClass **g_ScoreObjs;
extern GraphicBufferClass *&g_PseudoSeenBuff;
extern BOOL &g_StillUpdating;
extern PaletteClass &g_ScorePalette;
#else
extern void *g_Beepy6;
extern ScoreAnimClass *g_ScoreObjs[8];
extern GraphicBufferClass *g_PseudoSeenBuff;
extern BOOL g_StillUpdating;
#endif

#endif //SCOREOBJ_H
