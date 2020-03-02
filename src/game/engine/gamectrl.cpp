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

#include "gamectrl.h"
#include "callback.h"
#include "controlc.h"
#include "gadget.h"
#include "gameevent.h"
#include "gameloop.h"
#include "gamemain.h"
#include "gameoptions.h"
#include "gbuffer.h"
#include "iomap.h"
#include "mouse.h"
#include "msgbox.h"
#include "queue.h"
#include "session.h"
#include "slider.h"
#include "sndctrl.h"
#include "surfacemonitor.h"
#include "textbtn.h"

void GameControlsClass::Process_org()
{
    int v38 = 0;
    int dlg_width = 464;
    int dlg_height = 282;
    int xpos = (g_SeenBuff.Get_Width() - dlg_width) / 2;
    int ypos = (g_SeenBuff.Get_Height() - dlg_height) / 2;
    int v86 = xpos + dlg_width / 2;
    int v85 = 50;
    int slider_y_off = 13;
    int v83 = 10;
    int v82 = 4;
    int slider1_w = dlg_width - 68;
    int slider_height = 12;
    int slider1_x = xpos + 34;
    int slider1_y = ypos + 73;
    int slider2_w = dlg_width - 68;
    int slider2_h = 12;
    int slider2_x = xpos + 34;
    int slider2_y = ypos + 131;
    int button1_w = dlg_width - 80;
    int button1_h = 18;
    int button1_x = xpos + 40;
    int button1_y = ypos + 176;
    int button2_w = dlg_width - 80;
    int button2_h = 18;
    int button2_x = xpos + 40;
    int button2_y = ypos + 204;
    int v65 = 40;
    int v64 = 18;
    int button3_x = xpos + dlg_width / 2 - 20;
    int button3_y = dlg_height + ypos - 36;

    int org_gspeed = g_Options.Get_Game_Speed();
    int org_scrate = g_Options.Get_Scroll_Rate();
    int v57 = 0;
    int selection = 0;

    RemapControlType *remap = GadgetClass::Get_Color_Scheme();

    SliderClass gspeed_btn(100, slider1_x, slider1_y, slider1_w, slider_height, 1);

    SliderClass scrate_btn(101, slider2_x, slider2_y, slider2_w, slider2_h, 1);

    TextButtonClass visual_btn(
        102, TXT_VISUAL_CONTROLS, TPF_CENTER | TPF_6PT_GRAD | TPF_NOSHADOW, button1_x, button1_y, button1_w, button1_h, 0);

    TextButtonClass sound_btn(
        103, TXT_SOUND_CONTROLS, TPF_CENTER | TPF_6PT_GRAD | TPF_NOSHADOW, button2_x, button2_y, button2_w, button2_h, 0);

    TextButtonClass okbtn(104, TXT_OPTIONS_MENU, TPF_CENTER | TPF_6PT_GRAD | TPF_NOSHADOW, button3_x, button3_y, -1, -1, 0);
    okbtn.Set_XPos((g_SeenBuff.Get_Width() - okbtn.Get_Width()) / 2);

    Set_Logic_Page(&g_SeenBuff);

    GadgetClass *activegdgts = &okbtn;
    gspeed_btn.Add_Tail(*activegdgts);
    scrate_btn.Add_Tail(*activegdgts);
    visual_btn.Add_Tail(*activegdgts);
    sound_btn.Add_Tail(*activegdgts);

    gspeed_btn.Set_Maximum(7);
    gspeed_btn.Set_Thumb_Size(1);
    gspeed_btn.Set_Value(6 - org_gspeed);
    scrate_btn.Set_Maximum(7);
    scrate_btn.Set_Thumb_Size(1);
    scrate_btn.Set_Value(6 - org_scrate);

    TextButtonClass *buttons[5];
    buttons[0] = 0;
    buttons[1] = 0;
    buttons[2] = &visual_btn;
    buttons[3] = &sound_btn;
    buttons[4] = &okbtn;

    int v58;

    bool process = true;
    bool to_draw = true;
    bool to_draw_text = true;

    while (process) {
        if (g_Session.Game_To_Play() == GAME_CAMPAIGN || g_Session.Game_To_Play() == GAME_SKIRMISH) {
            Call_Back();
        } else if (Main_Loop()) {
            process = false;
        }

        if (g_AllSurfaces.Surfaces_Restored()) {
            g_AllSurfaces.Clear_Surfaces_Restored();
            to_draw = true;
        }

        if (to_draw) {
            g_Mouse->Hide_Mouse();
            g_Map.Flag_To_Redraw(true);
            g_Map.Render();
            Dialog_Box(xpos, ypos, dlg_width, dlg_height);
            Draw_Caption(TXT_GAME_CONTROLS, xpos, ypos, dlg_width);
            g_Mouse->Show_Mouse();
            to_draw = false;
            to_draw_text = true;
        }
        if (to_draw_text) {
            g_Mouse->Hide_Mouse();

            TextPrintType style = TPF_6PT_GRAD | TPF_NOSHADOW;
            if (selection == 0) {
                style |= TPF_USE_BRIGHT;
            }

            Fancy_Text_Print(56, slider1_x, slider1_y - slider_y_off, remap, 0, style);
            Fancy_Text_Print(173, slider1_x, slider1_y + slider_height + 2, remap, 0, TPF_6PT_GRAD | TPF_NOSHADOW);
            Fancy_Text_Print(172,
                slider1_w + slider1_x,
                slider1_y + slider_height + 2,
                remap,
                0,
                TPF_6PT_GRAD | TPF_NOSHADOW | TPF_RIGHT);

            style = TPF_6PT_GRAD | TPF_NOSHADOW;
            if (selection == 1) {
                style |= TPF_USE_BRIGHT;
            }

            Fancy_Text_Print(57, slider2_x, slider2_y - slider_y_off, remap, 0, style);
            Fancy_Text_Print(173, slider2_x, slider2_y + slider2_h + 2, remap, 0, TPF_6PT_GRAD | TPF_NOSHADOW);
            Fancy_Text_Print(
                172, slider2_w + slider2_x, slider2_y + slider2_h + 2, remap, 0, TPF_6PT_GRAD | TPF_NOSHADOW | TPF_RIGHT);

            activegdgts->Draw_All();
            g_Mouse->Show_Mouse();
            to_draw_text = false;
        }
        int input = activegdgts->Input();
        if (input > 37) {
            if (input <= 0x8064) {
                switch (input) {
                    case 0x8064: // speed
                        selection = 0;
                        to_draw_text = true;
                        break;
                    case KN_UP:
                        if (buttons[selection] != nullptr) {
                            buttons[selection]->Turn_Off();
                            buttons[selection]->Flag_To_Redraw();
                        }
                        --selection;
                        if (selection < 0) {
                            selection = 4;
                        }
                        if (buttons[selection] != nullptr) {
                            buttons[selection]->Turn_On();
                            buttons[selection]->Flag_To_Redraw();
                        }
                        to_draw_text = true;
                        break;
                    case KN_RIGHT:
                        if (selection == 0) {
                            gspeed_btn.Bump(0);
                        } else if (selection == 1) {
                            scrate_btn.Bump(0);
                        }
                        break;
                    case KN_DOWN:
                        if (buttons[selection] != nullptr) {
                            buttons[selection]->Turn_Off();
                            buttons[selection]->Flag_To_Redraw();
                        }
                        ++selection;
                        if (selection > 4) {
                            selection = 0;
                        }
                        if (buttons[selection] != nullptr) {
                            buttons[selection]->Turn_On();
                            buttons[selection]->Flag_To_Redraw();
                        }
                        to_draw_text = true;
                        break;
                }
            } else {
                input -= 0x8065;
                switch (input) {
                    case 0: // rate
                        selection = 1;
                        to_draw_text = true;
                        break;
                    case 1: // visbtn
                        v38 = 1;
                        v58 = 102;
                        v57 = 1;
                        break;
                    case 2: // soundbtn
                        v38 = 1;
                        v58 = 103;
                        v57 = 1;
                        break;
                    case 3: // okbtn
                        v38 = 1;
                        v58 = 104;
                        v57 = 1;
                        break;
                    default:
                        break;
                }
            }
        } else {
            switch (input) {
                case 37:
                    if (selection == 0) {
                        gspeed_btn.Bump(1);
                    } else if (selection == 1) {
                        scrate_btn.Bump(1);
                    }
                    break;
                case 13:
                    v38 = 1;
                    v58 = selection + 100;
                    v57 = 1;
                    break;
                case 27:
                    process = 0;
                    break;
            }
        }
        if (v57) {
            int v26 = gspeed_btn.Get_Value();
            if (org_gspeed != 6 - v26) {
                int v27 = gspeed_btn.Get_Value();
                org_gspeed = 6 - v27;
                // v28 = EventClass::EventClass(&v43, 10, 6 - v27);
                // QueueClass<EventClass,64>::Add(&OutList, v28);
            }
            int v29 = scrate_btn.Get_Value();
            if (org_scrate != 6 - v29) {
                int v30 = scrate_btn.Get_Value();
                org_scrate = 6 - v30;
                // Options.o.ScrollRate = 6 - v30;
            }
            process = 0;
            if (g_Session.Game_To_Play()) {
                // Options.o.GameSpeed = org_gspeed;
                // OptionsClass::Save_Settings(&Options.o);

            } else {
                // v42 = g_Options.GameSpeed();
                // Options.o.GameSpeed = org_gspeed;
                // OptionsClass::Save_Settings(&Options.o);
                // Options.o.GameSpeed = v42;
            }
            if (!v38) {
                //_RTC_UninitUse("selection");
            }
            int v36 = v58;
            if (v58 == 102) {
                // v31 = VisualControlsClass::VisualControlsClass(&v41);
                // VisualControlsClass::Process(v31);
                // process = 1;
                // to_draw = 1;
                // to_draw_text = true;
            } else if (v36 == 103) {
                if (/*SoundType*/ 1) {
                    SoundControlsClass().Process();
                    process = 1;
                    to_draw = 1;
                    to_draw_text = true;
                } else {
                    MessageBoxClass().Process(Text_String(TXT_NO_SOUND_CARD), 23, 0, 0, 0);
                    process = true;
                    to_draw = true;
                    to_draw_text = true;
                }
            }
            v57 = 0;
        }
    }
}

void GameControlsClass::Process()
{
    int v38 = 0;
    int dlg_width = 464;
    int dlg_height = 282;
    int xpos = (g_SeenBuff.Get_Width() - dlg_width) / 2;
    int ypos = (g_SeenBuff.Get_Height() - dlg_height) / 2;

    int v65 = 40;
    int v64 = 18;

    int org_gspeed = g_Options.Get_Game_Speed();
    int org_scrate = g_Options.Get_Scroll_Rate();
    int v57 = 0;
    int selection = 0;
    int slider_y_off = 13;

    RemapControlType *remap = GadgetClass::Get_Color_Scheme();

    int slider1_w = dlg_width - 68;
    int slider_height = 12;
    int slider1_x = xpos + 34;
    int slider1_y = ypos + 73;
    SliderClass gspeed_btn(100, slider1_x, slider1_y, slider1_w, slider_height, 1);

    int slider2_w = dlg_width - 68;
    int slider2_h = 12;
    int slider2_x = xpos + 34;
    int slider2_y = ypos + 131;
    SliderClass scrate_btn(101, slider2_x, slider2_y, slider2_w, slider2_h, 1);

    int button_x = xpos + 40;
    int button_w = dlg_width - 80;
    int button_h = 18;

    int button1_y = ypos + 166;
    TextButtonClass visual_btn(
        102, TXT_VISUAL_CONTROLS, TPF_CENTER | TPF_6PT_GRAD | TPF_NOSHADOW, button_x, button1_y, button_w, button_h, 0);

    int button2_y = ypos + 194;
    TextButtonClass sound_btn(
        103, TXT_SOUND_CONTROLS, TPF_CENTER | TPF_6PT_GRAD | TPF_NOSHADOW, button_x, button2_y, button_w, button_h, 0);

    TextButtonClass keyboard_btn(
        104, "Keyboard Controls", TPF_CENTER | TPF_6PT_GRAD | TPF_NOSHADOW, button_x, button2_y + 28, button_w, button_h, 0);

    int button3_x = xpos + dlg_width / 2 - 20;
    int button3_y = dlg_height + ypos - 36;
    TextButtonClass okbtn(105, TXT_OPTIONS_MENU, TPF_CENTER | TPF_6PT_GRAD | TPF_NOSHADOW, button3_x, button3_y, -1, -1, 0);

    okbtn.Set_XPos((g_SeenBuff.Get_Width() - okbtn.Get_Width()) / 2);

    Set_Logic_Page(&g_SeenBuff);

    GadgetClass *activegdgts = &okbtn;
    gspeed_btn.Add_Tail(*activegdgts);
    scrate_btn.Add_Tail(*activegdgts);
    visual_btn.Add_Tail(*activegdgts);
    sound_btn.Add_Tail(*activegdgts);
    keyboard_btn.Add_Tail(*activegdgts);

    gspeed_btn.Set_Maximum(7);
    gspeed_btn.Set_Thumb_Size(1);
    gspeed_btn.Set_Value(6 - org_gspeed);
    scrate_btn.Set_Maximum(7);
    scrate_btn.Set_Thumb_Size(1);
    scrate_btn.Set_Value(6 - org_scrate);

    TextButtonClass *buttons[6];
    buttons[0] = 0;
    buttons[1] = 0;
    buttons[2] = &visual_btn;
    buttons[3] = &sound_btn;
    buttons[4] = &keyboard_btn;
    buttons[5] = &okbtn;

    bool process = true;
    bool to_draw = true;
    bool to_draw_text = true;
    bool selection_made = false;

    while (process) {
        if (g_Session.Game_To_Play() == GAME_CAMPAIGN || g_Session.Game_To_Play() == GAME_SKIRMISH) {
            Call_Back();
        } else if (Main_Loop()) {
            process = false;
        }

        if (g_AllSurfaces.Surfaces_Restored()) {
            g_AllSurfaces.Clear_Surfaces_Restored();
            to_draw = true;
        }

        if (to_draw) {
            g_Mouse->Hide_Mouse();
            g_Map.Flag_To_Redraw(true);
            g_Map.Render();
            Dialog_Box(xpos, ypos, dlg_width, dlg_height);
            Draw_Caption(TXT_GAME_CONTROLS, xpos, ypos, dlg_width);
            g_Mouse->Show_Mouse();
            to_draw = false;
            to_draw_text = true;
        }
        if (to_draw_text) {
            g_Mouse->Hide_Mouse();

            TextPrintType style = TPF_6PT_GRAD | TPF_NOSHADOW;
            if (selection == 0) {
                style |= TPF_USE_BRIGHT;
            }

            Fancy_Text_Print(TXT_SPEED, slider1_x, slider1_y - slider_y_off, remap, 0, style);
            Fancy_Text_Print(TXT_SLOWER, slider1_x, slider1_y + slider_height + 2, remap, 0, TPF_6PT_GRAD | TPF_NOSHADOW);
            Fancy_Text_Print(TXT_FASTER,
                slider1_w + slider1_x,
                slider1_y + slider_height + 2,
                remap,
                0,
                TPF_6PT_GRAD | TPF_NOSHADOW | TPF_RIGHT);

            style = TPF_6PT_GRAD | TPF_NOSHADOW;
            if (selection == 1) {
                style |= TPF_USE_BRIGHT;
            }

            Fancy_Text_Print(TXT_SCROLLRATE, slider2_x, slider2_y - slider_y_off, remap, 0, style);
            Fancy_Text_Print(TXT_SLOWER, slider2_x, slider2_y + slider2_h + 2, remap, 0, TPF_6PT_GRAD | TPF_NOSHADOW);
            Fancy_Text_Print(
                TXT_FASTER, slider2_w + slider2_x, slider2_y + slider2_h + 2, remap, 0, TPF_6PT_GRAD | TPF_NOSHADOW | TPF_RIGHT);

            activegdgts->Draw_All();
            g_Mouse->Show_Mouse();
            to_draw_text = false;
        }
        KeyNumType input = activegdgts->Input();

        switch (input) {
            case GADGET_INPUT_RENAME2(100):
                selection = 0;
                selection_made = true;
                to_draw_text = true;
                break;

            case GADGET_INPUT_RENAME2(101):
                selection = 1;
                selection_made = true;
                to_draw_text = true;
                break;

            case GADGET_INPUT_RENAME2(102):
                selection = 2;
                selection_made = true;
                break;

            case GADGET_INPUT_RENAME2(103):
                selection = 3;
                selection_made = true;
                break;

            case GADGET_INPUT_RENAME2(104):
                selection = 4;
                selection_made = true;
                break;


            // options menu button/exit out of menu
            case KN_ESC:
            case GADGET_INPUT_RENAME2(105):
                process = false;
                break;

            case KN_RETURN:
                selection_made = true;
                break;

            case KN_UP: {
                if (buttons[selection] != nullptr) {
                    buttons[selection]->Turn_Off();
                    buttons[selection]->Flag_To_Redraw();
                }
                --selection;
                if (selection < 0) {
                    selection = 5;
                }
                if (buttons[selection] != nullptr) {
                    buttons[selection]->Turn_On();
                    buttons[selection]->Flag_To_Redraw();
                }
                to_draw_text = true;
                break;
            }
            case KN_DOWN: {
                if (buttons[selection] != nullptr) {
                    buttons[selection]->Turn_Off();
                    buttons[selection]->Flag_To_Redraw();
                }
                ++selection;
                if (selection > 5) {
                    selection = 0;
                }
                if (buttons[selection] != nullptr) {
                    buttons[selection]->Turn_On();
                    buttons[selection]->Flag_To_Redraw();
                }
                to_draw_text = true;
                break;
            }
            case KN_LEFT:
                if (selection == 0) {
                    gspeed_btn.Bump(1);
                } else if (selection == 1) {
                    scrate_btn.Bump(1);
                }
                break;

            case KN_RIGHT:
                if (selection == 0) {
                    gspeed_btn.Bump(0);
                } else if (selection == 1) {
                    scrate_btn.Bump(0);
                }
                break;

            default:
                break;
        }

        if (selection_made) {
            switch (selection) {
                case 2: {
                    // v31 = VisualControlsClass::VisualControlsClass(&v41);
                    // VisualControlsClass::Process(v31);
                    void (*func)(void *) = reinterpret_cast<void (*)(void *)>(0x0058D5F0);
                    func(nullptr);
                    to_draw = true;
                    to_draw_text = true;
                    break;
                }

                case 3: {
                    if (1) {
                        SoundControlsClass().Process();
                    } else {
                        MessageBoxClass().Process(Text_String(TXT_NO_SOUND_CARD), TXT_OK, 0, 0, 0);
                    }
                    to_draw = true;
                    to_draw_text = true;
                    break;
                }

                case 4:
                    MessageBoxClass().Process("Hello there", "General Kenobi", 0, 0, 0);
                    to_draw = true;
                    to_draw_text = true;
                    break;

                case 5:
                    process = false;
                    break;

                default:
                    break;
            }
            selection_made = false;
        }
    }

    // we exited the loop so save out values that changed
    int gspeed = 6 - gspeed_btn.Get_Value();
    if (org_gspeed != gspeed) {
        g_Options.Set_Game_Speed(gspeed);
        GameEventClass ev(GameEventClass::EVENT_GAMESPEED, (unsigned int)gspeed);
        g_ScheduledEvents.Add(ev);

        if (g_Session.Game_To_Play() == GAME_CAMPAIGN) {
            g_Options.Set_Game_Speed(gspeed);
            g_Options.Save_Settings();
        } else {
            int v38 = g_Options.Get_Game_Speed();
            g_Options.Set_Game_Speed(gspeed);
            g_Options.Save_Settings();
            g_Options.Set_Game_Speed(v38);
        }
    }

    int scrate = 6 - scrate_btn.Get_Value();
    if (org_scrate != scrate) {
        g_Options.Set_Scroll_Rate(scrate);
    }
}
