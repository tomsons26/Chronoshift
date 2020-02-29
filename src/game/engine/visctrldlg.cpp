#include "visctrldlg.h"
#include "callback.h"
#include "controlc.h"
#include "gadget.h"
#include "gameloop.h"
#include "gamemain.h"
#include "gameoptions.h"
#include "gbuffer.h"
#include "mouse.h"
#include "session.h"
#include "slider.h"
#include "surfacemonitor.h"
#include "textbtn.h"

// original
void VisualControlsClass::Process_Org()
{
    static int _titles[] = { 51, 58, 55, 54 };

    int v53 = 0;
    int width = 432;
    int height = 244;
    int xpos = 104;
    int ypos = 78;
    int txt_btn_y = 160;
    int slider_y_1 = 138;
    int slider_x = 314;
    int slider_y = 138;
    int slider_w = 140;
    int slider_h = 10;
    int slider_y_off = 22;
    int txt_btn_x = 282;

    int v87, input2;

    Set_Logic_Page(g_SeenBuff);

    TextButtonClass optionsbtn(6, TXT_OK, TPF_CENTER | TPF_6PT_GRAD | TPF_NOSHADOW, 0, txt_btn_x, 120, -1, 0);

    TextButtonClass resetbtn(5, TXT_RESET_VALUES, TPF_CENTER | TPF_6PT_GRAD | TPF_NOSHADOW, 0, txt_btn_x, 160, -1, 0);
    optionsbtn.Set_XPos(xpos + width - optionsbtn.Get_Width() - 34);
    resetbtn.Set_XPos(xpos + 34);
    resetbtn.Add_Tail(optionsbtn);

    SliderClass brightness(1, slider_x, slider_y, slider_w, slider_h, 1);
    brightness.Set_Thumb_Size(40);
    brightness.Set_Value(g_Options.Get_Brightness() * 256);
    brightness.Add_Tail(optionsbtn);

    SliderClass color(2, slider_x, slider_y_off + slider_y, slider_w, slider_h, 1);
    color.Set_Thumb_Size(40);
    color.Set_Value(g_Options.Get_Saturation() * 256);
    color.Add_Tail(optionsbtn);

    SliderClass contrast(3, slider_x, slider_y + 2 * slider_y_off, slider_w, slider_h, 1);
    contrast.Set_Thumb_Size(40);
    contrast.Set_Value(g_Options.Get_Contrast() * 256);
    contrast.Add_Tail(optionsbtn);

    SliderClass tint(4, slider_x, slider_y + 3 * slider_y_off, slider_w, slider_h, 1);
    tint.Set_Thumb_Size(40);
    tint.Set_Value(g_Options.Get_Tint() * 256);
    tint.Add_Tail(optionsbtn);

    GadgetClass dialog(xpos, ypos, width, height, 1, 0);
    dialog.Add_Tail(optionsbtn);

    ControlClass background(
        6, 0, 0, g_SeenBuff.Get_Width(), g_SeenBuff.Get_Height(), MOUSE_LEFT_PRESS | MOUSE_RIGHT_PRESS, 0);
    background.Add_Tail(optionsbtn);

    int selection = 0;
    TextButtonClass *buttons[6];
    buttons[0] = 0;
    buttons[1] = 0;
    buttons[2] = 0;
    buttons[3] = 0;
    buttons[4] = &resetbtn;
    buttons[5] = &optionsbtn;

    SliderClass *buttonsliders[6];
    buttonsliders[0] = &brightness;
    buttonsliders[1] = &color;
    buttonsliders[2] = &contrast;
    buttonsliders[3] = &tint;
    buttonsliders[4] = 0;
    buttonsliders[5] = 0;

    int to_draw = 1;
    int process = 1;
    int to_draw_text = 1;
    int v86 = 0;
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
            Dialog_Box(xpos, ypos, width, height);
            Draw_Caption(44, xpos, ypos, width);
            g_Mouse->Show_Mouse();
            to_draw = 0;
            to_draw_text = 1;
        }

        if (to_draw_text) {
            g_Mouse->Hide_Mouse();
            TextPrintType style;
            for (int i = 0; i < 4; ++i) {
                if (selection == i) {
                    style = TPF_USE_BRIGHT;
                } else {
                    style = TPF_6PT_GRAD | TPF_NOSHADOW;
                }
                Fancy_Text_Print(_titles[i],
                    slider_x - 16,
                    slider_y_1 + slider_y_off * i,
                    GadgetClass::Get_Color_Scheme(),
                    0,
                    TPF_6PT_GRAD | TPF_NOSHADOW | TPF_RIGHT | style);
            }
            optionsbtn.Draw_All();
            g_Mouse->Show_Mouse();
            to_draw_text = 0;
        }

        KeyNumType input = optionsbtn.Input();
        input2 = input;
        if (input > 32769) {
            input2 -= 32770;
            switch (input2) {
                case 0:
                    g_Options.Set_Saturation(fixed_t(color.Get_Value(), 256));
                    break;
                case 1:
                    g_Options.Set_Contrast(fixed_t(contrast.Get_Value(), 256));
                    break;
                case 2:
                    g_Options.Set_Tint(fixed_t(tint.Get_Value(), 256));
                    break;
                case 3:
                    v53 = 1;
                    v87 = 5;
                    v86 = 1;
                    break;
                case 4:
                LABEL_29:
                    v53 = 1;
                    v87 = 6;
                    v86 = 1;
                    break;
                default:
                    break;
            }
        } else if (input2 == 32769) {
            g_Options.Set_Brightness(fixed_t(brightness.Get_Value(), 256));
        } else {
            input2 -= 13;
            switch (input2) {
                case 14:
                    goto LABEL_29;
                case 24:
                    if (selection <= 3) {
                        buttonsliders[selection]->Bump(1);
                        switch (selection) {
                            case 0:
                                g_Options.Set_Brightness(fixed_t(brightness.Get_Value(), 256));
                                break;
                            case 1:
                                g_Options.Set_Saturation(fixed_t(color.Get_Value(), 256));
                                break;
                            case 2:
                                g_Options.Set_Contrast(fixed_t(contrast.Get_Value(), 256));
                                break;
                            case 3:
                                g_Options.Set_Tint(fixed_t(tint.Get_Value(), 256));
                                break;
                            default:
                                goto LABEL_73;
                        }
                    } else {
                        buttons[selection]->Turn_Off();
                        buttons[selection]->Flag_To_Redraw();
                        if (--selection < 4) {
                            selection = 5;
                        }
                        buttons[selection]->Turn_On();
                        buttons[selection]->Flag_To_Redraw();
                    }
                    break;
                case 26:
                    if (selection <= 3) {
                        buttonsliders[selection]->Bump(0);
                        switch (selection) {
                            case 0:
                                g_Options.Set_Brightness(fixed_t(brightness.Get_Value(), 256));
                                break;
                            case 1:
                                g_Options.Set_Saturation(fixed_t(color.Get_Value(), 256));
                                break;
                            case 2:
                                g_Options.Set_Contrast(fixed_t(contrast.Get_Value(), 256));
                                break;
                            case 3:
                                g_Options.Set_Tint(fixed_t(tint.Get_Value(), 256));
                                break;
                            default:
                                goto LABEL_73;
                        }
                    } else {
                        buttons[selection]->Turn_Off();
                        buttons[selection]->Flag_To_Redraw();
                        if (++selection > 5) {
                            selection = 4;
                        }
                        buttons[selection]->Turn_On();
                        buttons[selection]->Flag_To_Redraw();
                    }
                    break;
                case 25:
                    if (selection <= 3) {
                        to_draw_text = 1;
                    } else {
                        buttons[selection]->Turn_Off();
                        buttons[selection]->Flag_To_Redraw();
                    }
                    if (--selection == 4) {
                        selection = 3;
                    }
                    if (selection < 0) {
                        selection = 4;
                    }
                    if (selection <= 3) {
                        to_draw_text = 1;
                    } else {
                        buttons[selection]->Turn_On();
                        buttons[selection]->Flag_To_Redraw();
                    }
                    break;
                case 27:
                    if (selection <= 3) {
                        to_draw_text = 1;
                    } else {
                        buttons[selection]->Turn_Off();
                        buttons[selection]->Flag_To_Redraw();
                    }
                    if (++selection > 4) {
                        selection = 0;
                    }
                    if (selection <= 3) {
                        to_draw_text = 1;
                    } else {
                        buttons[selection]->Turn_On();
                        buttons[selection]->Flag_To_Redraw();
                    }
                    break;
                case 0:
                    v53 = 1;
                    v87 = selection + 1;
                    v86 = 1;
                    break;
                default:
                    break;
            }
        }
    LABEL_73:
        if (v86) {
            if (!v53) {
                // sub_0046BCC1("selection");
            }
            if (v87 == 5) {
                brightness.Set_Value(128);
                contrast.Set_Value(128);
                color.Set_Value(128);
                tint.Set_Value(128);
                g_Options.Set_Brightness(fixed_t::_1_2);
                g_Options.Set_Contrast(fixed_t::_1_2);
                g_Options.Set_Saturation(fixed_t::_1_2);
                g_Options.Set_Tint(fixed_t::_1_2);
            } else if (v87 == 6) {
                process = 0;
            }
            v86 = 0;
        }
    }
}

void VisualControlsClass::Process_attempt1()
{
    static int _titles[] = { TXT_BRIGHTNESS, TXT_COLOR, TXT_CONTRAST, TXT_TINT };

    int width = 432;
    int height = 244;
    int xpos = 104;
    int ypos = 78;
    int txt_btn_y = 160;
    int slider_y_1 = 138;
    int slider_x = 314;
    int slider_y = 138;
    int slider_w = 140;
    int slider_h = 10;
    int slider_y_off = 22;
    int txt_btn_x = 282;

    Set_Logic_Page(g_SeenBuff);

    TextButtonClass optionsbtn(6, TXT_OK, TPF_CENTER | TPF_6PT_GRAD | TPF_NOSHADOW, 0, txt_btn_x, 120, -1, 0);

    TextButtonClass resetbtn(5, TXT_RESET_VALUES, TPF_CENTER | TPF_6PT_GRAD | TPF_NOSHADOW, 0, txt_btn_x, 160, -1, 0);
    optionsbtn.Set_XPos(xpos + width - optionsbtn.Get_Width() - 34);
    resetbtn.Set_XPos(xpos + 34);
    resetbtn.Add_Tail(optionsbtn);

    SliderClass brightness(1, slider_x, slider_y, slider_w, slider_h, 1);
    brightness.Set_Thumb_Size(40);
    brightness.Set_Value(g_Options.Get_Brightness() * 256);
    brightness.Add_Tail(optionsbtn);

    SliderClass color(2, slider_x, slider_y_off + slider_y, slider_w, slider_h, 1);
    color.Set_Thumb_Size(40);
    color.Set_Value(g_Options.Get_Saturation() * 256);
    color.Add_Tail(optionsbtn);

    SliderClass contrast(3, slider_x, slider_y + 2 * slider_y_off, slider_w, slider_h, 1);
    contrast.Set_Thumb_Size(40);
    contrast.Set_Value(g_Options.Get_Contrast() * 256);
    contrast.Add_Tail(optionsbtn);

    SliderClass tint(4, slider_x, slider_y + 3 * slider_y_off, slider_w, slider_h, 1);
    tint.Set_Thumb_Size(40);
    tint.Set_Value(g_Options.Get_Tint() * 256);
    tint.Add_Tail(optionsbtn);

    GadgetClass dialog(xpos, ypos, width, height, 1, 0);
    dialog.Add_Tail(optionsbtn);

    ControlClass background(
        6, 0, 0, g_SeenBuff.Get_Width(), g_SeenBuff.Get_Height(), MOUSE_LEFT_PRESS | MOUSE_RIGHT_PRESS, 0);
    background.Add_Tail(optionsbtn);

    TextButtonClass *buttons[6];
    buttons[0] = 0;
    buttons[1] = 0;
    buttons[2] = 0;
    buttons[3] = 0;
    buttons[4] = &resetbtn;
    buttons[5] = &optionsbtn;

    SliderClass *buttonsliders[6];
    buttonsliders[0] = &brightness;
    buttonsliders[1] = &color;
    buttonsliders[2] = &contrast;
    buttonsliders[3] = &tint;
    buttonsliders[4] = 0;
    buttonsliders[5] = 0;

    int selection = 0;
    int color_update = 0;
    bool to_draw = true;
    bool process = true;
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
            Dialog_Box(xpos, ypos, width, height);
            Draw_Caption(44, xpos, ypos, width);
            g_Mouse->Show_Mouse();
            to_draw = false;
            to_draw_text = true;
        }

        if (to_draw_text) {
            g_Mouse->Hide_Mouse();
            TextPrintType style;
            for (int i = 0; i < 4; ++i) {
                if (selection == i) {
                    style = TPF_USE_BRIGHT;
                } else {
                    style = TPF_6PT_GRAD | TPF_NOSHADOW;
                }

                Fancy_Text_Print(_titles[i],
                    slider_x - 16,
                    slider_y_1 + slider_y_off * i,
                    GadgetClass::Get_Color_Scheme(),
                    0,
                    TPF_6PT_GRAD | TPF_NOSHADOW | TPF_RIGHT | style);
            }
            optionsbtn.Draw_All();
            g_Mouse->Show_Mouse();
            to_draw_text = false;
        }

        KeyNumType input = optionsbtn.Input();

        if (input != KN_NONE) {
            switch (input) {
                case KN_ESC:
                    process = false;
                    break;

                case GADGET_INPUT_RENAME2(1):
                    color_update = 1;
                    break;

                case GADGET_INPUT_RENAME2(2):
                    color_update = 2;
                    break;

                case GADGET_INPUT_RENAME2(3):
                    color_update = 3;
                    break;

                case GADGET_INPUT_RENAME2(4):
                    color_update = 4;
                    break;

                case GADGET_INPUT_RENAME2(5):
                    color_update = 5;
                    break;

                case GADGET_INPUT_RENAME2(6):
                    process = false;
                    break;

                case KN_DOWN:
                    if (buttons[selection] != nullptr) {
                        buttons[selection]->Turn_Off();
                        buttons[selection]->Flag_To_Redraw();
                    }
                    selection++;
                    if (selection > 5) {
                        selection = 0;
                    }
                    if (buttons[selection] != nullptr) {
                        buttons[selection]->Turn_On();
                        buttons[selection]->Flag_To_Redraw();
                    }

                    if (selection <= 4) {
                        to_draw_text = true;
                    }
                    break;

                case KN_UP:
                    if (buttons[selection] != nullptr) {
                        buttons[selection]->Turn_Off();
                        buttons[selection]->Flag_To_Redraw();
                    }
                    selection--;
                    if (selection < 0) {
                        selection = 5;
                    }
                    if (buttons[selection] != nullptr) {
                        buttons[selection]->Turn_On();
                        buttons[selection]->Flag_To_Redraw();
                    }
                    if (selection >= 0) {
                        to_draw_text = true;
                    }
                    break;

                case KN_LEFT:
                    if (buttonsliders[selection] != nullptr) {
                        buttonsliders[selection]->Bump(1);
                        color_update = selection;
                    }

                    // handle going left from the OK button
                    if (selection == 5) {
                        buttons[selection]->Turn_Off();
                        buttons[selection]->Flag_To_Redraw();
                        selection--;
                        buttons[selection]->Turn_On();
                        buttons[selection]->Flag_To_Redraw();
                    }

                    break;

                case KN_RIGHT:
                    if (buttonsliders[selection] != nullptr) {
                        buttonsliders[selection]->Bump(0);
                        color_update = selection;
                    }

                    // handle going right to the OK button
                    if (selection == 4) {
                        buttons[selection]->Turn_Off();
                        buttons[selection]->Flag_To_Redraw();
                        selection++;
                        buttons[selection]->Turn_On();
                        buttons[selection]->Flag_To_Redraw();
                    }
                    break;

                case KN_RETURN:
                    if (selection == 4) {
                        color_update = 4;
                    }

                    if (selection == 5) {
                        process = false;
                    }
                    break;

                default:
                    break;
            }
        }

        switch (color_update) {
            case 0:
                g_Options.Set_Brightness(fixed_t(brightness.Get_Value(), 256));
                color_update = 0;
                break;

            case 1:
                g_Options.Set_Saturation(fixed_t(color.Get_Value(), 256));
                color_update = 0;
                break;

            case 2:
                g_Options.Set_Contrast(fixed_t(contrast.Get_Value(), 256));
                color_update = 0;
                break;

            case 3:
                g_Options.Set_Tint(fixed_t(tint.Get_Value(), 256));
                color_update = 0;
                break;

            case 4:
                brightness.Set_Value(128);
                contrast.Set_Value(128);
                color.Set_Value(128);
                tint.Set_Value(128);
                g_Options.Set_Brightness(fixed_t::_1_2);
                g_Options.Set_Contrast(fixed_t::_1_2);
                g_Options.Set_Saturation(fixed_t::_1_2);
                g_Options.Set_Tint(fixed_t::_1_2);
                color_update = 0;
                break;
            default:
                break;
        }
    }
}


void VisualControlsClass::Process()
{
    static int _titles[] = { TXT_BRIGHTNESS, TXT_COLOR, TXT_CONTRAST, TXT_TINT };

    int width = 432;
    int height = 244;
    int xpos = 104;
    int ypos = 78;
    int txt_btn_y = 160;
    int slider_y_1 = 138;
    int slider_x = 314;
    int slider_y = 138;
    int slider_w = 140;
    int slider_h = 10;
    int slider_y_off = 22;
    int txt_btn_x = 282;

    Set_Logic_Page(g_SeenBuff);

    TextButtonClass optionsbtn(6, TXT_OK, TPF_CENTER | TPF_6PT_GRAD | TPF_NOSHADOW, 0, txt_btn_x, 120, -1, 0);

    TextButtonClass resetbtn(5, TXT_RESET_VALUES, TPF_CENTER | TPF_6PT_GRAD | TPF_NOSHADOW, 0, txt_btn_x, 160, -1, 0);
    optionsbtn.Set_XPos(xpos + width - optionsbtn.Get_Width() - 34);
    resetbtn.Set_XPos(xpos + 34);
    resetbtn.Add_Tail(optionsbtn);

    SliderClass brightness(1, slider_x, slider_y, slider_w, slider_h, 1);
    brightness.Set_Thumb_Size(40);
    brightness.Set_Value(g_Options.Get_Brightness() * 256);
    brightness.Add_Tail(optionsbtn);

    SliderClass color(2, slider_x, slider_y_off + slider_y, slider_w, slider_h, 1);
    color.Set_Thumb_Size(40);
    color.Set_Value(g_Options.Get_Saturation() * 256);
    color.Add_Tail(optionsbtn);

    SliderClass contrast(3, slider_x, slider_y + 2 * slider_y_off, slider_w, slider_h, 1);
    contrast.Set_Thumb_Size(40);
    contrast.Set_Value(g_Options.Get_Contrast() * 256);
    contrast.Add_Tail(optionsbtn);

    SliderClass tint(4, slider_x, slider_y + 3 * slider_y_off, slider_w, slider_h, 1);
    tint.Set_Thumb_Size(40);
    tint.Set_Value(g_Options.Get_Tint() * 256);
    tint.Add_Tail(optionsbtn);

    GadgetClass dialog(xpos, ypos, width, height, 1, 0);
    dialog.Add_Tail(optionsbtn);

    ControlClass background(
        6, 0, 0, g_SeenBuff.Get_Width(), g_SeenBuff.Get_Height(), MOUSE_LEFT_PRESS | MOUSE_RIGHT_PRESS, 0);
    background.Add_Tail(optionsbtn);

    TextButtonClass *buttons[6];
    buttons[0] = 0;
    buttons[1] = 0;
    buttons[2] = 0;
    buttons[3] = 0;
    buttons[4] = &resetbtn;
    buttons[5] = &optionsbtn;

    SliderClass *buttonsliders[6];
    buttonsliders[0] = &brightness;
    buttonsliders[1] = &color;
    buttonsliders[2] = &contrast;
    buttonsliders[3] = &tint;
    buttonsliders[4] = 0;
    buttonsliders[5] = 0;

    int selection = 0;
    bool visual_update = false;
    bool to_draw = true;
    bool process = true;
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
            Dialog_Box(xpos, ypos, width, height);
            Draw_Caption(TXT_VISUAL_CONTROLS, xpos, ypos, width);
            g_Mouse->Show_Mouse();
            to_draw = false;
            to_draw_text = true;
        }

        if (to_draw_text) {
            g_Mouse->Hide_Mouse();
            TextPrintType style;
            for (int i = 0; i < 4; ++i) {
                if (selection == i) {
                    style = TPF_USE_BRIGHT;
                } else {
                    style = TPF_6PT_GRAD | TPF_NOSHADOW;
                }

                Fancy_Text_Print(_titles[i],
                    slider_x - 16,
                    slider_y_1 + slider_y_off * i,
                    GadgetClass::Get_Color_Scheme(),
                    0,
                    TPF_6PT_GRAD | TPF_NOSHADOW | TPF_RIGHT | style);
            }
            optionsbtn.Draw_All();
            g_Mouse->Show_Mouse();
            to_draw_text = false;
        }

        KeyNumType input = optionsbtn.Input();

        if (input != KN_NONE) {
            switch (input) {
                case KN_ESC:
                    process = false;
                    break;

                case GADGET_INPUT_RENAME2(1):
                    selection = 0;
                    visual_update = true;
                    to_draw_text = true;
                    break;

                case GADGET_INPUT_RENAME2(2):
                    selection = 1;
                    visual_update = true;
                    to_draw_text = true;
                    break;

                case GADGET_INPUT_RENAME2(3):
                    selection = 2;
                    visual_update = true;
                    to_draw_text = true;
                    break;

                case GADGET_INPUT_RENAME2(4):
                    selection = 3;
                    visual_update = true;
                    to_draw_text = true;
                    break;

                // Reset button
                case GADGET_INPUT_RENAME2(5):
                    selection = 4;
                    visual_update = true;
                    break;

                // OK button
                case GADGET_INPUT_RENAME2(6):
                    process = false;
                    break;

                case KN_DOWN: {
                    // flag text to redraw if the selection is a slider
                    // turn off the button if we have a button selected
                    if (selection <= 3) {
                        to_draw_text = true;
                    } else {
                        buttons[selection]->Turn_Off();
                        buttons[selection]->Flag_To_Redraw();
                    }

                    ++selection;
                    if (selection > 5) {
                        selection = 0;
                    }

                    // flag text to redraw if the selection is still a slider
                    // turn on the new button if we have a button selected
                    if (selection <= 3) {
                        to_draw_text = true;
                    } else {
                        buttons[selection]->Turn_On();
                        buttons[selection]->Flag_To_Redraw();
                    }
                } break;

                case KN_UP: {
                    // flag text to redraw if the selection is a slider
                    // turn off the button if we have a button selected
                    if (selection <= 3) {
                        to_draw_text = true;
                    } else {
                        buttons[selection]->Turn_Off();
                        buttons[selection]->Flag_To_Redraw();
                    }

                    --selection;
                    if (selection < 0) {
                        selection = 5;
                    }

                    // flag text to redraw if the selection is still a slider
                    // turn on the new button if we have a button selected
                    if (selection <= 3) {
                        to_draw_text = true;
                    } else {
                        buttons[selection]->Turn_On();
                        buttons[selection]->Flag_To_Redraw();
                    }
                } break;

                case KN_LEFT:
                    // handle adjusting slider to a lower value
                    if (selection <= 3) {
                        buttonsliders[selection]->Bump(1);
                        visual_update = true;
                    } else {
                        // handle going left from the OK button
                        if (selection == 5) {
                            buttons[selection]->Turn_Off();
                            buttons[selection]->Flag_To_Redraw();

                            --selection;

                            buttons[selection]->Turn_On();
                            buttons[selection]->Flag_To_Redraw();
                        }
                    }

                    break;

                case KN_RIGHT:
                    // handle adjusting slider to a higher value
                    if (selection <= 3) {
                        buttonsliders[selection]->Bump(0);
                        visual_update = true;
                    } else {
                        // handle going right to the OK button
                        if (selection == 4) {
                            buttons[selection]->Turn_Off();
                            buttons[selection]->Flag_To_Redraw();

                            ++selection;

                            buttons[selection]->Turn_On();
                            buttons[selection]->Flag_To_Redraw();
                        }
                    }

                    break;

                case KN_RETURN:
                    // handle Reset button
                    if (selection == 4) {
                        visual_update = true;
                    }

                    // handle OK button
                    if (selection == 5) {
                        process = false;
                    }
                    break;

                default:
                    break;
            }
        }

        if (visual_update) {
            switch (selection) {
                case 0:
                    g_Options.Set_Brightness(fixed_t(brightness.Get_Value(), 256));
                    break;

                case 1:
                    g_Options.Set_Saturation(fixed_t(color.Get_Value(), 256));
                    break;

                case 2:
                    g_Options.Set_Contrast(fixed_t(contrast.Get_Value(), 256));
                    break;

                case 3:
                    g_Options.Set_Tint(fixed_t(tint.Get_Value(), 256));
                    break;

                case 4:
                    brightness.Set_Value(128);
                    contrast.Set_Value(128);
                    color.Set_Value(128);
                    tint.Set_Value(128);

                    g_Options.Set_Brightness(fixed_t::_1_2);
                    g_Options.Set_Contrast(fixed_t::_1_2);
                    g_Options.Set_Saturation(fixed_t::_1_2);
                    g_Options.Set_Tint(fixed_t::_1_2);

                    buttons[selection]->Turn_Off();
                    selection = 0;
                    to_draw_text = true;
                    break;

                default:
                    break;
            }
            visual_update = false;
        }
    }
}
