/**
 * @file
 *
 * @author OmniBlade
 * @author CCHyper
 * @author tomsons26
 *
 * @brief Functions for handling difficulty levels.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "difficulty.h"
#include "callback.h"
#include "gameini.h"
#include "gbuffer.h"
#include "mouse.h"
#include "rules.h"
#include "slider.h"
#include "surfacemonitor.h"
#include "textbtn.h"

void Difficulty_Get(GameINIClass &ini, DifficultyClass &diff, const char *section)
{
    if (ini.Find_Section(section) != nullptr) {
        diff.Firepower = ini.Get_Fixed(section, "FirePower", "1.0");
        diff.Groundspeed = ini.Get_Fixed(section, "Groundspeed", "1.0");
        diff.Airspeed = ini.Get_Fixed(section, "Airspeed", "1.0");
        diff.Armor = ini.Get_Fixed(section, "Armor", "1.0");
        diff.ROF = ini.Get_Fixed(section, "ROF", "1.0");
        diff.Cost = ini.Get_Fixed(section, "Cost", "1.0");
        diff.RepairDelay = ini.Get_Fixed(section, "RepairDelay", "0.02");
        diff.BuildDelay = ini.Get_Fixed(section, "BuildDelay", "0.03");
        diff.BuildTime = ini.Get_Fixed(section, "BuildTime", "1.0");
        diff.BuildSlowdown = ini.Get_Bool(section, "BuildSlowdown", false);
        diff.DestroyWalls = ini.Get_Bool(section, "DestroyWalls", true);
        diff.ContentScan = ini.Get_Bool(section, "ContentScan", false);
    }
}
int Fetch_Difficulty_Dialog(BOOL one_time_mission)
{
    char buffer[512];

    strlcpy(buffer, Text_String(TXT_SET_DIFFICULTY), sizeof(buffer));

    // trucante string at "." if arg is true (is this one off mission?)
    if (one_time_mission) {
        if (buffer[0] != '\0') {
            for (int i = 0; buffer[i + 1] != '\0'; ++i) {
                if (buffer[i] == '.') {
                    buffer[i + 1] = '\0';
                    break;
                }
            }
        }
    }

    // empty call sets the font spacing stuff?
    Fancy_Text_Print(nullptr, 0, 0, nullptr, COLOR_TBLACK, TPF_6PT_GRAD | TPF_SHADOW);

    int width = 0;
    int height = 0;

    Format_Window_String(buffer, 380, width, height);

    TextButtonClass okbtn(BUTTON_OK, TXT_OK, TPF_CENTER | TPF_6PT_GRAD | TPF_NOSHADOW, 470, 244, 60);

    TextButtonClass cancelbtn(BUTTON_CANCEL, TXT_CANCEL, TPF_CENTER | TPF_6PT_GRAD | TPF_NOSHADOW, 110, 244, 60);
    SliderClass diffsli(3, 110, 222, 420, 16, true);

    if (g_Rule.Fine_Diff_Control()) {
        diffsli.Set_Maximum(5);
        diffsli.Set_Value(2);
    } else {
        diffsli.Set_Maximum(3);
        diffsli.Set_Value(1); // set initial value.
    }
    GadgetClass *active_gadget = &okbtn;

    // Button linking
    diffsli.Add(*active_gadget);
    cancelbtn.Add(*active_gadget);

    bool to_draw = true;
    Set_Logic_Page(&g_SeenBuff);

    int diff = DIFF_NONE;
    bool process = true;

    while (process) {
        if (to_draw) {
            to_draw = false;

            g_Mouse->Hide_Mouse();

            Dialog_Box(70, 120, 500, 160);

            Fancy_Text_Print(buffer, 110, 150, GadgetClass::Get_Color_Scheme(), COLOR_TBLACK, TPF_6PT_GRAD | TPF_SHADOW);
            Fancy_Text_Print(TXT_HARD,
                diffsli.Get_Width() + diffsli.Get_XPos(),
                diffsli.Get_YPos() - 18,
                GadgetClass::Get_Color_Scheme(),
                COLOR_TBLACK,
                TPF_6PT_GRAD | TPF_SHADOW | TPF_RIGHT);
            Fancy_Text_Print(TXT_NORMAL,
                diffsli.Get_XPos() + diffsli.Get_Width() / 2,
                diffsli.Get_YPos() - 18,
                GadgetClass::Get_Color_Scheme(),
                COLOR_TBLACK,
                TPF_6PT_GRAD | TPF_SHADOW | TPF_CENTER);
            Fancy_Text_Print(TXT_EASY,
                diffsli.Get_XPos(),
                diffsli.Get_YPos() - 18,
                GadgetClass::Get_Color_Scheme(),
                COLOR_TBLACK,
                TPF_6PT_GRAD | TPF_SHADOW);

            if (diffsli.Get_Value() == 0) {
                Fancy_Text_Print("An easy computer controlled opponent.",
                    110,
                    185,
                    GadgetClass::Get_Color_Scheme(),
                    COLOR_TBLACK,
                    TPF_6PT_GRAD | TPF_SHADOW);
            }

            if (diffsli.Get_Value() == 1) {
                Fancy_Text_Print("An average computer controlled opponent.",
                    110,
                    185,
                    GadgetClass::Get_Color_Scheme(),
                    COLOR_TBLACK,
                    TPF_6PT_GRAD | TPF_SHADOW);
            }

            if (diffsli.Get_Value() == 2) {
                Fancy_Text_Print("An difficult computer controlled opponent.",
                    110,
                    185,
                    GadgetClass::Get_Color_Scheme(),
                    COLOR_TBLACK,
                    TPF_6PT_GRAD | TPF_SHADOW);
            }

            if (diffsli.Get_Value() > 2) {
                Fancy_Text_Print("Diff out of range! Got %d",
                    120,
                    140,
                    GadgetClass::Get_Color_Scheme(),
                    COLOR_TBLACK,
                    TPF_6PT_GRAD | TPF_SHADOW,
                    diffsli.Get_Value());
            }

            if (active_gadget != nullptr) {
                active_gadget->Draw_All(true);
            }

            g_Mouse->Show_Mouse();
        }

        Call_Back();

        if (g_AllSurfaces.Surfaces_Restored()) {
            g_AllSurfaces.Clear_Surfaces_Restored();
            to_draw = true;
        } else {
            KeyNumType input = active_gadget->Input();

            if (input != KN_NONE) {
                switch (input) {
                    case KN_RETURN:
                    case GADGET_INPUT_RENAME2(BUTTON_OK):
                        diff = ((g_Rule.Fine_Diff_Control() == false) + 1) * diffsli.Get_Value();
                        process = false;
                        break;

                    case KN_ESC:
                    case GADGET_INPUT_RENAME2(BUTTON_CANCEL):
                        process = false;
                        break;

                    case GADGET_INPUT_RENAME2(3): // for updating text when slider is moved
                        to_draw = true;
                        break;

                    default:
                        break;
                }
            }
        }
    }

    return diff;
}
