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
#include "scoreobj.h"
#include "dialog.h"
#include "drawshape.h"
#include "gamefile.h"
#include "gameoptions.h"
#include "gbuffer.h"
#include "language.h"
#include "textprint.h"

#ifdef GAME_DLL
extern ScoreAnimClass **g_ScoreObjs;
#else
ScoreAnimClass *g_ScoreObjs[8];
GraphicBufferClass *g_PseudoSeenBuff = nullptr;
BOOL g_StillUpdating = false;
void *g_Beepy6 = nullptr;
#endif

ScoreAnimClass::ScoreAnimClass(int xpos, int ypos, void *data) :
    m_Timer(), m_XPos(2 * xpos), m_YPos(2 * ypos), m_Data((uint8_t *)data)
{
    m_Timer.Set(0);
    m_Timer.Start();
}
/**
 *
 *
 */
void ScoreAnimClass::Update()
{
    // blank
}

ScoreAnimClass::~ScoreAnimClass()
{
    m_Data = nullptr;
}

ScoreTimeClass::ScoreTimeClass(int xpos, int ypos, void *data, int framecount, int delay) :
    ScoreAnimClass(xpos, ypos, data), m_Frame(0), m_FrameCount(framecount), m_Delay(delay)
{
}

/**
 *
 *
 */
void ScoreTimeClass::Update()
{
    if (m_Timer.Time() == 0) {
        m_Timer.Set(m_Delay);

        m_Frame++;

        if (m_Frame >= m_FrameCount) {
            m_Frame = 0;
        }

        Set_Logic_Page(g_SeenBuff);
        CC_Draw_Shape(m_Data, m_Frame, m_XPos, m_YPos, WINDOW_0, SHAPE_VIEWPORT_REL);
        Set_Logic_Page(g_PseudoSeenBuff);
        CC_Draw_Shape(m_Data, m_Frame, m_XPos, m_YPos, WINDOW_0, SHAPE_VIEWPORT_REL);
        Set_Logic_Page(g_LogicPage);
    }
}

ScoreTimeClass::~ScoreTimeClass() {}

ScoreCredsClass::ScoreCredsClass(int xpos, int ypos, void *data, int framecount, int delay) :
    ScoreAnimClass(xpos, ypos, data), m_Frame(0), m_FrameCount(framecount), m_Delay(delay)
{
    m_ClockSound = GameFileClass::Retrieve("clock1.aud");
    m_CashSound = GameFileClass::Retrieve("cashturn.aud");
}

/**
 *
 *
 */
void ScoreCredsClass::Update()
{
    if (m_Timer.Expired()) {
        m_Timer.Set(m_Delay);

        m_Frame++;

        if (m_Frame >= m_FrameCount) {
            m_Frame = 0;
        }

        Set_Logic_Page(g_SeenBuff);
        // Play_Sample(m_ClockSound, 255, g_Options.Normalize_Volume(130), 0);
        CC_Draw_Shape(m_Data, m_Frame, m_XPos, m_YPos, WINDOW_0, SHAPE_VIEWPORT_REL);
        Set_Logic_Page(g_PseudoSeenBuff);
        CC_Draw_Shape(m_Data, m_Frame, m_XPos, m_YPos, WINDOW_0, SHAPE_VIEWPORT_REL);
        Set_Logic_Page(g_LogicPage);
    }
}

ScoreCredsClass::~ScoreCredsClass()
{
    m_ClockSound = nullptr;
    m_CashSound = nullptr;
}

ScorePrintClass::ScorePrintClass(TextEnum str_id, int xpos, int ypos, uint8_t *palette, int a6) :
    ScoreAnimClass(xpos, ypos, (void *)Text_String((int)str_id)), m_field_19(a6), m_NextEntry(0), m_Palette(palette)
{
}

ScorePrintClass::ScorePrintClass(void *data, int xpos, int ypos, uint8_t *palette, int a6) :
    ScoreAnimClass(xpos, ypos, data), m_field_19(a6), m_NextEntry(0), m_Palette(palette)
{
}

/**
 *
 *
 */
void ScorePrintClass::Update()
{
    static char _localstr[4];

    if (m_NextEntry > 0 && m_Data[m_NextEntry - 1] == nullptr) {
        for (int i = 0; i < 8; ++i) {
            if (g_ScoreObjs[i] == this) {
                g_ScoreObjs[i] = nullptr;
            }
        }
        delete this;
    } else {
        g_StillUpdating = true;
        if (m_Timer.Expired()) {
            m_Timer.Set(1);
            int anim_ypos = 12 * m_NextEntry + m_XPos;
            if (m_NextEntry > 0) {
                Set_Font_Palette_Range(m_Palette, 0, 15);
                _localstr[0] = (char)m_Data[m_NextEntry - 1];
                g_HidPage.Print(_localstr, anim_ypos - 12, m_YPos, 0, 0);
                g_HidPage.Blit(g_SeenBuff, anim_ypos - 12, m_YPos - 2, anim_ypos - 12, m_YPos - 2, 14, 16);
                g_HidPage.Blit(*g_PseudoSeenBuff, anim_ypos - 12, m_YPos - 2, anim_ypos - 12, m_YPos - 2, 14, 16);
                g_PseudoSeenBuff->Print(_localstr, anim_ypos - 12, m_YPos, 0, 0);
            }
            if (m_Data[m_NextEntry] != nullptr) {
                _localstr[0] = m_Data[m_NextEntry];

                static uint8_t _whitepal[16] = { 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15 };
                Set_Font_Palette_Range(_whitepal, 0, 15);

                g_SeenBuff.Print(_localstr, anim_ypos, m_YPos - 1, 0, 0);
                g_SeenBuff.Print(_localstr, anim_ypos, m_YPos + 1, 0, 0);
                g_SeenBuff.Print(_localstr, anim_ypos + 1, m_YPos, 0, 0);

                g_PseudoSeenBuff->Print(_localstr, anim_ypos, m_YPos - 1, 0, 0);
                g_PseudoSeenBuff->Print(_localstr, anim_ypos, m_YPos + 1, 0, 0);
                g_PseudoSeenBuff->Print(_localstr, anim_ypos + 1, m_YPos, 0, 0);
            }
            ++m_NextEntry;
        }
    }
}

ScorePrintClass::~ScorePrintClass()
{
    m_Palette = nullptr;
}

// ScoreScaleClass
ScoreScaleClass::ScoreScaleClass(void *data, int xpos, int ypos, uint8_t *palette) :
    ScoreAnimClass(xpos, ypos, data), m_DestIndex(0), m_Palette(palette)
{
}

/**
 *
 *
 */
void ScoreScaleClass::Update()
{
    if (m_Timer.Expired()) {
        if (m_DestIndex) {
            Set_Font_Palette_Range(m_Palette, 0, 15);
            g_HidPage.Fill_Rect(0, 0, 14, 14, 0);

            char *string = (char *)m_Data;
            g_HidPage.Print(string, 0, 0, 0, 0);

            // currently unknown what these are
            static int _destx[] = { 0, 80, 107, 134, 180, 228 };
            static int _destw[] = { 6, 20, 30, 40, 60, 80 };

            g_HidPage.Scale(g_SeenBuff,
                0,
                0,
                2 * _destx[m_DestIndex],
                m_YPos,
                10,
                12,
                2 * _destw[m_DestIndex],
                2 * _destw[m_DestIndex],
                1,
                0);

            --m_DestIndex;
        } else {
            Set_Font_Palette_Range(m_Palette, 0, 15);
            for (int i = 0; i < 8; ++i) {
                if (this == g_ScoreObjs[i]) {
                    g_ScoreObjs[i] = nullptr;
                }
            }
            g_HidPage.Print((char *)m_Data, m_XPos, m_YPos, 0, 0);
            g_HidPage.Blit(g_SeenBuff, m_XPos, m_YPos, m_XPos, m_YPos, 12, 12, 0);
            g_HidPage.Blit(*g_PseudoSeenBuff, m_XPos, m_YPos, m_XPos, m_YPos, 12, 12, 0);
            delete this;
        }
    }
}

ScoreScaleClass::~ScoreScaleClass()
{
    m_Palette = nullptr;
}

int Alloc_Object(ScoreAnimClass *obj)
{
    for (int i = 0; i < 8; ++i) {
        if (g_ScoreObjs[i] == nullptr) {
            g_ScoreObjs[i] = obj;
            return i;
        }
    }
    return 0;
}

void Update_Objects()
{
    for (int i = 0; i < 8; ++i) {
        if (g_ScoreObjs[i] != nullptr) {
            g_ScoreObjs[i]->Update();
        }
    }
}

void Dealloc_Objects()
{
    for (int i = 0; i < 8; ++i) {
        if (g_ScoreObjs[i] != nullptr) {
            delete g_ScoreObjs[i];
        }
        g_ScoreObjs[i] = nullptr;
    }
}

void Animate_Score_Objs()
{
    g_StillUpdating = false;
    g_PseudoSeenBuff->Blit(g_SeenBuff);
    Update_Objects();
}

char *Int_Print(int intval)
{
    static char str[12];
    sprintf(str, "%d", intval);
    return str;
}
