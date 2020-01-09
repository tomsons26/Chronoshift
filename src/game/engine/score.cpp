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
#include "score.h"
#include "callback.h"
#include "drawshape.h"
#include "gamefile.h"
#include "gameoptions.h"
#include "globals.h"
#include "house.h"
#include "init.h"
#include "iomap.h"
#include "language.h"
#include "logic.h"
#include "mouse.h"
#include "pal.h"
#include "palette.h"
#include "scoreobj.h"
#include "textprint.h"
#include <algorithm>

char *g_ScreenNames[] = { "alibackh.pcx", "sovbackh.pcx" };

uint8_t _bluepal[] = { 192, 193, 193, 195, 194, 197, 195, 199, 196, 201, 202, 203, 204, 205, 192, 207 };
uint8_t _greenpal[] = { 112, 113, 124, 115, 125, 117, 126, 119, 127, 121, 122, 123, 124, 125, 124, 127 };
uint8_t _redpal[] = { 208, 209, 215, 211, 217, 213, 218, 215, 219, 217, 218, 219, 220, 221, 214, 223 };
uint8_t _yellowpal[] = { 0, 0, 236, 0, 235, 0, 234, 0, 233, 0, 0, 0, 0, 0, 237, 0 };

#pragma pack(push, 1)
struct ScoreStruct
{
    char Name[11];
    int Score;
    int ScenarioIndex;
    int Side;
};
#pragma pack(pop)

/**
 *
 *
 */
void ScoreClass::Presentation()
{
    Disable_Uncompressed_Shapes();
    g_PseudoSeenBuff = new GraphicBufferClass(g_SeenBuff.Get_Width(), g_SeenBuff.Get_Height());
    GameFileClass hall_of_fame("hallfame.dat");

    //
    // initial setup
    //
    int old_yspacing = g_FontXSpacing;
    int side = 1;
    if (g_PlayerPtr->What_Type() != HOUSES_USSR) {
        if (g_PlayerPtr->What_Type() != HOUSES_UKRAINE) {
            side = 0;
        }
    }
    /// sprintf(scorepalname, "scorpal1.pal"); not used?
    // ControlQ = 0;
    g_FontXSpacing = 0;
    g_Map.Override_Mouse_Shape(MOUSE_POINTER);
    g_Theme.Queue_Song(THEME_SCORE);
    g_VisiblePage.Clear();
    g_SysMemPage.Clear();
    g_Mouse->Erase_Mouse(g_HidPage, true);
    g_HiddenPage.Clear();
    Set_Logic_Page(g_SysMemPage);
    g_BlackPalette.Set(0);
    //
    // fetch needed files
    //
    void *country4 = GameFileClass::Retrieve("country4.aud");
    void *sfx4 = GameFileClass::Retrieve("sfx4.aud");
    g_Beepy6 = GameFileClass::Retrieve("beepy6.aud"); // Beepy6 was static dunno why
    int timing = m_field_28 / 3600 + 1;
    void *bluebar = GameFileClass::Retrieve("bar3bhr.shp");
    DEBUG_ASSERT(bluebar != nullptr);
    void *redbar = GameFileClass::Retrieve("bar3rhr.shp");
    DEBUG_ASSERT(redbar != nullptr);
    void *old_font = Set_Font(g_ScoreFontPtr);
    //
    // start actually doing stuff
    //
    Call_Back();
    g_Mouse->Hide_Mouse();
    Load_Title_Screen(g_ScreenNames[side], &g_HidPage, &g_ScorePalette);
    Increase_Palette_Luminance((uint8_t *)&g_ScorePalette[0], 30, 30, 30, 63);
    g_HidPage.Blit(g_SeenBuff);
    g_HidPage.Blit(*g_PseudoSeenBuff);
    g_ScorePalette.Set(7, Call_Back);
    Play_Sample(country4, 255, g_Options.Normalize_Volume(150), 0);
    //
    // create timer and print objects
    //
    g_ScoreObjs[0] = new ScoreTimeClass(238, 2, GameFileClass::Retrieve("timehr.shp"), 30, 4);
    g_ScoreObjs[1] = new ScoreTimeClass(4, 89, GameFileClass::Retrieve("hisc1-hr.shp"), 10, 4);
    g_ScoreObjs[2] = new ScoreTimeClass(4, 180, GameFileClass::Retrieve("hisc2-hr.shp"), 10, 4);
    Set_Logic_Page(g_SeenBuff);
    Alloc_Object(new ScorePrintClass(TXT_SCORE_TIME, 204, 9, _greenpal, 0));
    Alloc_Object(new ScorePrintClass(TXT_SCORE_LEAD, 164, 26, _greenpal, 0));
    Alloc_Object(new ScorePrintClass(TXT_SCORE_EFFI, 164, 38, _greenpal, 0));
    Alloc_Object(new ScorePrintClass(TXT_SCORE_TOTAL, 164, 50, _greenpal, 0));
    Play_Sample(sfx4, 255, g_Options.Normalize_Volume(150), 0);
    Call_Back_Delay(13);
    g_Keyboard->Clear();
    //
    // count houses?
    //
    int someaddition = 0;
    for (int i = 0; i < g_Logic.Count(); ++i) {
        ObjectClass &obj = *g_Logic[i];
        HousesType owner = obj.Owner();
        if (side != 0 && (owner == HOUSES_USSR || owner == HOUSES_BADGUY || owner == HOUSES_UKRAINE)) {
            ++someaddition;
        } else if (side == 0 && owner == HOUSES_GREECE) {
            ++someaddition;
        }
    }
    //
    // tally scores
    //
    int total_score = 0;

    for (HousesType i = HOUSES_FIRST; i <= HOUSES_BADGUY; ++i) {
        HouseClass *hptr = HouseClass::As_Pointer(i);

        if (i == HOUSES_USSR || i == HOUSES_BADGUY || i == HOUSES_UKRAINE) {
            m_BadUnitsLost += hptr->Units_Lost();
            m_BadBuildingsLost += hptr->Buildings_Lost();
        } else {
            m_GoodUnitsLost += hptr->Units_Lost();
            m_GoodBuildingsLost += hptr->Buildings_Lost();
        }

        if (g_PlayerPtr->Is_Ally(i)) {
            total_score += hptr->Get_Score();
        }
    }
    //
    // account for diff
    //
    /*
    int diff = g_PlayerPtr->Get_AI_Difficulty();

    if (diff < 1u) {
        if (!diff) {
            total_score += 500;
        }
    } else if (diff <= 1) {
        total_score += 1500;
    } else if (diff == 2) {
        total_score += 3500;
    }
    */
    // TODO confirm the switch pseudo wasn't fucked as this seems incomplete
    switch (g_PlayerPtr->Get_AI_Difficulty()) {
        case DIFF_EASIEST:
            total_score += 500;
            break;
        case DIFF_EASIER:
            total_score += 1500;
            break;
        case DIFF_NORMAL:
            total_score += 3500;
            break;
    }
    //
    // calculate scores
    //
    if (!someaddition) {
        someaddition = 1;
    }
    int losses = 0;
    if (side) {
        losses = m_BadBuildingsLost + m_BadUnitsLost;
    } else {
        losses = m_GoodBuildingsLost + m_GoodUnitsLost;
    }
    losses = std::min(150, (100 * fixed_t(someaddition, someaddition + losses)));
    // HouseClass::Available_Money(PlayerPtr); unused??!
    int money = g_PlayerPtr->Get_House_Static().Get_Credits() + g_PlayerPtr->Get_Harvested() + 1;
    fixed_t clipped_money(g_PlayerPtr->Get_Stolen() + g_PlayerPtr->Available_Money() + 1, money);
    int clipped_money2 = std::min(100 * clipped_money, 150);
    int score_print_val = clipped_money2 * total_score / 100 + losses * total_score / 100;
    if (score_print_val < -9999) {
        score_print_val = -9999;
    }
    score_print_val = std::min(score_print_val, 99999);
    //
    // print score countups and session time
    //
    g_Keyboard->Clear();
    for (int k = 0; k <= 130; ++k) {
        Set_Font_Palette_Range(_greenpal, 0, 15);
        Count_Up_Print("%3d%%", k * losses / 100, losses, 244, 26);
        if (k >= 30) {
            Count_Up_Print("%3d%%", clipped_money2 * (k - 30) / 100, clipped_money2, 244, 38);
        }
        Print_Minutes(timing);
        Call_Back_Delay(1);
        Play_Sample(g_Beepy6, 255, g_Options.Normalize_Volume(100), 0);
        if (k >= 30 && k >= losses && k - 30 >= clipped_money2) {
            break;
        }
    }
    Count_Up_Print("%3d%%", losses, losses, 244, 26);
    Count_Up_Print("%3d%%", clipped_money2, clipped_money2, 244, 38);
    //
    // draw total score
    //
    char score_str[16];
    sprintf(score_str, "x %5d", total_score);
    Alloc_Object(new ScorePrintClass(score_str, 274, 26, _greenpal, 0));
    Alloc_Object(new ScorePrintClass(score_str, 274, 38, _greenpal, 0));
    //
    // draw total score line
    Call_Back_Delay(8);
    g_SeenBuff.Draw_Line(548, 96, 626, 96, COLOR_WHITE);
    Call_Back_Delay(1);
    g_SeenBuff.Draw_Line(548, 96, 626, 96, COLOR_GREEN);
    //
    // print score val
    //
    sprintf(score_str, "%5d", score_print_val);
    Alloc_Object(new ScorePrintClass(score_str, 286, 50, _greenpal, 0));
    Call_Back_Delay(60);

    // draw credits
    if (side != 0) {
        Show_Credits(side, _greenpal);
    }
    Call_Back_Delay(60);
    Set_Logic_Page(g_SeenBuff);
    Play_Sample(sfx4, 255, g_Options.Normalize_Volume(150), 0);

    static int _casuax[] = { 144, 150 };
    static int _casuay[] = { 78, 78 };
    Alloc_Object(new ScorePrintClass(TXT_SCORE_CASU, _casuax[0], _casuay[0], _greenpal, 0));
    Call_Back_Delay(9);
    ScorePrintClass *printptr = nullptr;

    // draw texts
    static int _gditxy[] = { 90, 90 };
    static int _gditxx[] = { 135, 150 };
    static int _nodtxx[] = { 135, 150 };
    static int _nodtxy[] = { 102, 102 };
    if (side == 0) {
        Alloc_Object(new ScorePrintClass(TXT_ALLIES, _gditxx[0], _gditxy[0], _bluepal, 0));
        printptr = new ScorePrintClass(TXT_SOVIET, _nodtxx[0], _nodtxy[0], _redpal, 0);
    } else {
        Alloc_Object(new ScorePrintClass(TXT_SOVIET, _nodtxx[0], _gditxy[0], _redpal, 0));
        printptr = new ScorePrintClass(TXT_ALLIES, _gditxx[0], _nodtxy[0], _bluepal, 0);
    }
    Alloc_Object(printptr);
    Call_Back_Delay(6);
    //
    // draw more texts
    //
    Set_Font_Palette_Range(_redpal, 0, 15);
    if (side) {
        Do_BadGuy_Casualties_Graph();
    } else {
        Do_Graph(bluebar, redbar, m_SomeUnitsLost + m_GoodUnitsLost, m_BadUnitsLost, 89);
    }
    Set_Logic_Page(g_SeenBuff);
    Play_Sample(sfx4, 255, g_Options.Normalize_Volume(150), 0);
    Alloc_Object(new ScorePrintClass(TXT_SCORE_BUIL, 144, 126, _greenpal, 0));
    Call_Back_Delay(9);
    printptr = nullptr;
    static int _bldggy[] = { 138, 138 };
    static int _bldgny[] = { 150, 150 };
    if (side == 0) {
        Alloc_Object(new ScorePrintClass(TXT_ALLIES, _gditxx[0], _bldggy[0], _bluepal, 0));
        printptr = new ScorePrintClass(TXT_SOVIET, _nodtxx[0], _bldgny[0], _redpal, 0);
    } else {
        Alloc_Object(new ScorePrintClass(TXT_SOVIET, _nodtxx[0], _bldggy[0], _redpal, 0));
        printptr = new ScorePrintClass(TXT_ALLIES, _gditxx[0], _bldgny[0], _bluepal, 0);
    }
    Alloc_Object(printptr);
    Call_Back_Delay(7);
    if (side) {
        Call_Back_Delay(6);
        Set_Font_Palette_Range(_greenpal, 0, 15);
        Do_BadGuy_Buildings_Graph();
    } else {
        Do_Graph(bluebar, redbar, m_SomeBuildingsLost + m_GoodBuildingsLost, m_BadBuildingsLost, 137);
    }

    while (g_StillUpdating) {
        Call_Back_Delay(1);
    }
    // draw badguy credits
    g_Keyboard->Clear();
    if (side == 0) {
        Show_Credits(0, _greenpal);
    }
    //
    // left side top scores stuff
    //
    Play_Sample(sfx4, 255, g_Options.Normalize_Volume(150), 0);
    Alloc_Object(new ScorePrintClass(TXT_SCORE_TOP, 28, 110, _greenpal, 0));
    Call_Back_Delay(9);
    // handle score structs
    ScoreStruct scores[7];
    // write blank hall of fame file if none found
    if (!hall_of_fame.Is_Available()) {
        hall_of_fame.Open(FM_WRITE);
        for (int l = 0; l < 7; ++l) {
            scores[l].ScenarioIndex = 0;
            scores[l].Score = 0;
            scores[l].Name[0] = 0;
            scores[l].Side = 0;
            hall_of_fame.Write(&scores[l], sizeof(ScoreStruct));
        }
        hall_of_fame.Close();
    }
    //
    // read hall of fame file
    hall_of_fame.Open(FM_READ);
    for (int m = 0; m < 7; ++m) {
        hall_of_fame.Read(&scores[m], sizeof(ScoreStruct));
    }
    hall_of_fame.Close();
    //
    // score shuffling code
    if (scores[6].Score >= score_print_val) {
        scores[6].Score = 0;
    }
    int score_entry = 0;
    for (score_entry = 0; score_entry < 7; ++score_entry) {
        if (score_print_val > scores[score_entry].Score) {
            if (score_entry < 6) {
                for (int i = 6; i > score_entry; --i) {
                    memcpy(&scores[i], &scores[i - 1], sizeof(scores[i]));
                }
            }
            scores[score_entry].Score = score_print_val;
            scores[score_entry].ScenarioIndex = g_Scen.Get_Scenario_Index();
            scores[score_entry].Name[0] = 0;
            scores[score_entry].Side = side;
            break;
        }
    }
    Set_Logic_Page(g_SeenBuff);
    //
    // left side score printing code
    char score_val_str[32][7];
    for (int n = 0; n < 7; ++n) {
        uint8_t *pal = nullptr;
        if (scores[score_entry].Side) {
            pal = _redpal;
        } else {
            pal = _bluepal;
        }
        Alloc_Object(new ScorePrintClass(scores[n].Name, 11, 8 * n + 120, pal, 0));
        if (scores[n].Score) {
            char *string = score_val_str[n];
            sprintf(string, "%d", scores[n].Score);
            Alloc_Object(new ScorePrintClass(string, 95, 8 * n + 120, pal, 12));
            if (scores[n].ScenarioIndex < 20) {
                sprintf(string + 16, "%d", scores[n].ScenarioIndex);
            } else {
                strcpy(string + 16, "**");
            }
            Alloc_Object(new ScorePrintClass(string + 16, 77, 8 * n + 120, pal, 12));
            Call_Back_Delay(13);
        }
    }
    //
    // thread wait loop!?
    //
    while (g_StillUpdating) {
        Call_Back_Delay(1);
    }
    g_Keyboard->Clear();
    //
    // write out inputted name to ui and score struct to file
    //
    if (score_entry < 7) {
        uint8_t *pal = nullptr;
        if (scores[score_entry].Side) {
            pal = _redpal;
        } else {
            pal = _bluepal;
        }
        // print inputted name?
        Input_Name(scores[score_entry].Name, 11, 8 * score_entry + 120, pal);

        // write out all scores
        hall_of_fame.Open(FM_WRITE);
        for (int jj = 0; jj < 7; ++jj) {
            hall_of_fame.Write(&scores[jj], sizeof(ScoreStruct));
        }
        hall_of_fame.Close();
        //
    } else {
        // click to continue doesn't show on pc why is this here......
        // this does how up in beta
        Alloc_Object(new ScorePrintClass(TXT_CLICK_CONTINUE, 149, 190, _yellowpal, 0));
        // ControlQ = 0;
        Cycle_Wait_Click(1);
    }
    //
    // prepare for leaving the scorescreen
    //
    g_Keyboard->Clear();
    Dealloc_Objects();
    g_BlackPalette.Set(7);
    g_VisiblePage.Clear();
    g_Mouse->Show_Mouse();
    g_Theme.Queue_Song(THEME_NONE);

    g_BlackPalette.Set(7);
    g_VisiblePage.Clear();

    g_GamePalette.Set(0);

    // cleanup
    Set_Font(old_font);
    g_FontXSpacing = old_yspacing;
    // ControlQ = 0;
    if (g_PseudoSeenBuff != nullptr) {
        delete g_PseudoSeenBuff;
    }
    Enable_Uncompressed_Shapes();
}

/**
 *
 *
 */
void ScoreClass::Do_BadGuy_Buildings_Graph()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x00542104, void, ScoreClass *);
    func(this);
#endif
}

/**
 *
 *
 */
void ScoreClass::Do_Graph(void *bluebar, void *redbar, int good_lost, int bad_lost, int ypos)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x00542AB8, void, ScoreClass *, void *, void *, int, int ,int);
    func(this, bluebar, redbar, good_lost, bad_lost, ypos);
#else
    int side = 1;
    if (g_PlayerPtr->What_Type() != HOUSES_USSR) {
        if (g_PlayerPtr->What_Type() != HOUSES_UKRAINE) {
            side = 0;
        }
    }

    if (side) {
        std::swap(good_lost, bad_lost);
        std::swap(bluebar, redbar);
    }
    int v92 = good_lost;
    int v49 = bad_lost;

    int losses = std::max(good_lost, bad_lost);
    if (!losses) {
        losses = 1;
    }
    int good_frame = 118 * good_lost / losses;
    int bad_frame = 118 * bad_lost / losses;
    if (losses < 20) {
        good_frame = 5 * good_lost;
        bad_frame = 5 * bad_lost;
    }
    int frame_max = std::max(good_frame, bad_frame);
    if (!frame_max) {
        frame_max = 1;
    }
    Set_Logic_Page(g_HidPage);
    g_HidPage.Fill_Rect(0, 0, 248, 18, 0);
    CC_Draw_Shape(redbar, 119, 0, 0, WINDOW_0, SHAPE_VIEWPORT_REL);
    Set_Logic_Page(g_SeenBuff);

    // good guy bars
    uint8_t *pal = nullptr;
    if (side) {
        pal = _redpal;
    } else {
        pal = _bluepal;
    }
    Set_Font_Palette_Range(pal, 0, 15);
    for (int i = 1; i <= good_frame; ++i) {
        if (i == good_frame) {
            g_HidPage.Blit(g_SeenBuff, 0, 0, 2 * 174, 2 * ypos, 2 * good_frame + 6, 16, 0);
            g_HidPage.Blit(*g_PseudoSeenBuff, 0, 0, 2 * 174, 2 * ypos, 2 * good_frame + 6, 16, 0);
        } else {
            Set_Logic_Page(g_PseudoSeenBuff);
            CC_Draw_Shape(bluebar, i, 2 * 174, 2 * ypos, WINDOW_0, SHAPE_VIEWPORT_REL);
            Set_Logic_Page(g_SeenBuff);
            CC_Draw_Shape(bluebar, i, 2 * 174, 2 * ypos, WINDOW_0, SHAPE_VIEWPORT_REL);
        }
        Count_Up_Print("%d", i * good_lost / frame_max, good_lost, 297, ypos + 2);
        Play_Sample(g_Beepy6, 255, g_Options.Normalize_Volume(150), 0);
        Call_Back_Delay(2);
    }
    CC_Draw_Shape(bluebar, good_frame, 2 * 174, 2 * ypos, WINDOW_0, SHAPE_VIEWPORT_REL);
    Set_Logic_Page(g_PseudoSeenBuff);
    CC_Draw_Shape(bluebar, good_frame, 2 * 174, 2 * ypos, WINDOW_0, SHAPE_VIEWPORT_REL);
    Set_Logic_Page(g_SeenBuff);
    Count_Up_Print("%d", good_lost, good_lost, 297, ypos + 2);
    Call_Back_Delay(40);

    // bad guy bars
    if (side) {
        pal = _redpal;
    } else {
        pal = _bluepal;
    }
    Set_Font_Palette_Range(pal, 0, 15);
    for (int j = 1; j <= bad_frame; ++j) {
        if (j == bad_frame) {
            g_HidPage.Blit(g_SeenBuff, 0, 0, 2 * 174, 2 * ypos + 24, 2 * bad_frame + 6, 16, 0);
            g_HidPage.Blit(*g_PseudoSeenBuff, 0, 0, 2 * 174, 2 * ypos + 24, 2 * bad_frame + 6, 16, 0);
        } else {
            Set_Logic_Page(g_PseudoSeenBuff);
            CC_Draw_Shape(redbar, j, 2 * 174, 2 * ypos + 24, WINDOW_0, SHAPE_VIEWPORT_REL);
            Set_Logic_Page(g_SeenBuff);
            CC_Draw_Shape(redbar, j, 2 * 174, 2 * ypos + 24, WINDOW_0, SHAPE_VIEWPORT_REL);
        }
        Count_Up_Print("%d", j * bad_lost / frame_max, bad_lost, 297, ypos + 14);
        Play_Sample(g_Beepy6, 255, g_Options.Normalize_Volume(150), 0);
        Call_Back_Delay(2);
    }
    Set_Logic_Page(g_PseudoSeenBuff);
    CC_Draw_Shape(redbar, bad_frame, 2 * 174, 2 * ypos + 24, WINDOW_0, SHAPE_VIEWPORT_REL);
    Set_Logic_Page(g_SeenBuff);
    CC_Draw_Shape(redbar, bad_frame, 2 * 174, 2 * ypos + 24, WINDOW_0, SHAPE_VIEWPORT_REL);
    Count_Up_Print("%d", bad_lost, bad_lost, 297, ypos + 14);
    Call_Back_Delay(40);
#endif
}

/**
 *
 *
 */
void ScoreClass::Do_BadGuy_Casualties_Graph()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0054380C, void, ScoreClass *);
    func(this);
#endif
}

/**
 *
 *
 */
void ScoreClass::Show_Credits(int side, uint8_t *palette)
{
    // ending credits location
    static int _credtx[] = { 182, 182 };
    static int _credty[] = { 167, 62 };
    Alloc_Object(new ScorePrintClass(TXT_SCORE_ENDCRED, _credtx[side], _credty[side], palette, 0));

    Call_Back_Delay(15);

    // credit shape location
    static int _credsx[] = { 276, 276 };
    static int _credsy[] = { 173, 58 };
    void *credshape = GameFileClass::Retrieve(side ? "credsuhr.shp" : "credsahr.shp");
    DEBUG_ASSERT(credshape != nullptr);
    int object_index = Alloc_Object(new ScoreCredsClass(_credsx[side], _credsy[side], credshape, 32, 2));

    // credit text print location
    static int _credpx[] = { 228, 236 };
    static int _credpy[] = { 177, 74 };

    int v18 = g_PlayerPtr->Available_Money() / 100;
    int v8 = -50;

    do {
        int v9 = 5;
        if (g_PlayerPtr->Available_Money() - v8 > 100) {
            v9 = 20;
        }
        if (g_PlayerPtr->Available_Money() - v8 > 500) {
            v9 += 30;
        }
        if (g_PlayerPtr->Available_Money() - v8 > 1000) {
            v9 += g_PlayerPtr->Available_Money() / 40;
        }
        if (v9 < v18) {
            v9 = v18;
        }
        v8 += v9;
        if (v8 < 0) {
            v8 = 0;
        }
        Set_Font_Palette_Range(palette, 0, 15);
        Count_Up_Print("%d", v8, g_PlayerPtr->Available_Money(), _credpx[side], _credpy[side]);
        Call_Back_Delay(2);
    } while (v8 < g_PlayerPtr->Available_Money());

    ScoreAnimClass *obj = g_ScoreObjs[object_index];
    if (obj != nullptr) {
        delete obj;
    }
    g_ScoreObjs[object_index] = nullptr;
}

/**
 *
 *
 */
void ScoreClass::Print_Minutes(int value)
{
    char string[20];

    if (value >= 60) {
        if (value / 60 > 9) {
            value = 599;
        }
        sprintf(string, Text_String(TXT_SCORE_TIMEFORMAT1), value / 60, value % 60);
    } else {
        sprintf(string, Text_String(TXT_SCORE_TIMEFORMAT2), value);
    }

    g_SeenBuff.Print(string, 550, 18, 0, 0);
    g_PseudoSeenBuff->Print(string, 550, 18, 0, 0);
}

/**
 *
 *
 */
void ScoreClass::Count_Up_Print(char *format, int value1, int value2, int xpos, int ypos)
{
    int val_to_use;

    if (value1 <= value2) {
        val_to_use = value1;
    } else {
        val_to_use = value2;
    }

    char string[64];
    sprintf(string, format, val_to_use);
    g_SeenBuff.Print(string, 2 * xpos, 2 * ypos, 0, 12);
    g_PseudoSeenBuff->Print(string, 2 * xpos, 2 * ypos, 0, 12);
}

/**
 *
 *
 */
void ScoreClass::Input_Name(char *name, int length, int a3, uint8_t *palette)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x005444AC, void, ScoreClass *, char *, int, int, uint8_t *);
    func(this, name, length, a3, palette);
#endif
}

void Cycle_Wait_Click(int a1)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x00541EA0, void, int);
    func(a1);
#endif
}
