#include "mplayerdlg.h"
#include "callback.h"
#include "dialog.h"
#include "gbuffer.h"
#include "globals.h"
#include "mouse.h"
#include "msgbox.h"
#include "multimission.h"
#include "surfacemonitor.h"
#include "textbtn.h"

GameEnum Select_Serial_Dialog()
{
#ifdef GAME_DLL
    GameEnum (*func)(void) = reinterpret_cast<GameEnum (*)(void)>(0x0050FD9C);
    return func();
#else
    return false;
#endif
}
BOOL Com_Scenario_Dialog(BOOL a1)
{
#ifdef GAME_DLL
    BOOL (*func)(void) = reinterpret_cast<BOOL (*)(void)>(0x0051289C);
    return func();
#else
    return false;
#endif
}

GameEnum Select_MPlayer_Game()
{
    // buffer for the background
    GraphicBufferClass background(g_visiblePage.Get_Width(), g_visiblePage.Get_Height());

    TextButtonClass modembtn(BUTTON_MP_MODEM, 187, TPF_CENTER | TPF_NOSHADOW | TPF_6PT_GRAD, 240, 208, 160, 18, 0);
    TextButtonClass skrimishbtn(BUTTON_MP_SKIRMISH, 482, TPF_CENTER | TPF_NOSHADOW | TPF_6PT_GRAD, 240, 230, 160, 18, 0);
    TextButtonClass networkbtn(BUTTON_MP_NETWORK, 188, TPF_CENTER | TPF_NOSHADOW | TPF_6PT_GRAD, 240, 252, 160, 18, 0);
    TextButtonClass internetbtn(
        BUTTON_MP_INTERNET, "Internet", TPF_CENTER | TPF_NOSHADOW | TPF_6PT_GRAD, 240, 274, 160, 18, 0);
    TextButtonClass cancelbtn(BUTTON_MP_CANCEL, 19, TPF_CENTER | TPF_NOSHADOW | TPF_6PT_GRAD, 260, 306, 120, 18, 0);

    // set graphic page to draw to
    Set_Logic_Page(g_seenBuff);

    // capture the background for use when redrawing
    g_visiblePage.Blit(background);

    GadgetClass *activegdt = &modembtn;
    skrimishbtn.Add_Tail(*activegdt);
    networkbtn.Add_Tail(*activegdt);
    internetbtn.Add_Tail(*activegdt);
    cancelbtn.Add_Tail(*activegdt);

    TextButtonClass *buttons[5];
    buttons[0] = &modembtn;
    buttons[1] = &skrimishbtn;
    buttons[2] = &networkbtn;
    buttons[3] = &internetbtn;
    buttons[4] = &cancelbtn;
    int current_button = 0;
    int top_button = 0;
    int bottom_button = 4;
    buttons[top_button]->Turn_On();

    BOOL process = true;
    BOOL to_draw = true;
    GameEnum game = GAME_2;
    while (process) {
        Call_Back();
        if (g_allSurfaces.Surfaces_Restored()) {
            g_allSurfaces.Clear_Surfaces_Restored();
            to_draw = true;
        }
        if (to_draw) {
            g_mouse->Hide_Mouse();
            background.Blit(g_visiblePage);
            Dialog_Box(130, 166, 380, 178);
            Draw_Caption(186, 130, 166, 380);

            // draw all linked gadgets
            activegdt->Draw_All(true);
            g_mouse->Show_Mouse();
            to_draw = false;
        }
        // handle keyboard and button input
        KeyNumType input = activegdt->Input();
        if (input != KN_NONE) {
            // handle arrow key input
            switch (input) {
                case KN_UP: {
                    buttons[current_button]->Turn_Off();
                    --current_button;
                    if (current_button < top_button) {
                        current_button = bottom_button;
                    }
                    buttons[current_button]->Turn_On();
                    break;
                }
                case KN_DOWN: {
                    buttons[current_button]->Turn_Off();
                    ++current_button;
                    if (current_button > bottom_button) {
                        current_button = top_button;
                    }
                    buttons[current_button]->Turn_On();
                    break;
                }
                case KN_RETURN: {
                    // enter was pressed so make a KeyNumType to reflect on which button it was
                    input = GADGET_INPUT_RENAME2(BUTTON_MP_MODEM + current_button);
                    break;
                }
                default:
                    break;
            }
            // handle button input
            switch (input) {
                case GADGET_INPUT_RENAME2(BUTTON_MP_MODEM): {
                    // this case should just return the game type and this code should be in Select_Game imo
                    GameEnum serialgame = Select_Serial_Dialog();
                    if (serialgame == GAME_CAMPAIGN) {
                        // We returned back from the modem menu
                        game = GAME_2;
                        to_draw = true;
                    } else {
                        process = false;
                    }
                    break;
                }
                case GADGET_INPUT_RENAME2(BUTTON_MP_SKIRMISH): {
                    // this case should just return the game type and this code should be in Select_Game imo
                    // sort of need to set it for code in Com_Scenario_Dialog, sort of, its a mess...
                    Session.Set_Game_To_Play(GAME_SKIRMISH);
                    game = GAME_SKIRMISH;
                    if (Com_Scenario_Dialog(true)) {
                        // bAftermathMultiplayer = Is_Aftermath_Installed();
                        Session.Set_Send_Scenario_Is_Official(
                            Session.MPlayer_Scenario_List()[Session.MPlayer_Options().LocalID]->Is_Official());
                        process = false;
                    } else {
                        // We returned back from the skirmish menu
                        Session.Set_Game_To_Play(GAME_CAMPAIGN);
                        game = GAME_2;
                        to_draw = true;
                    }
                    break;
                }
                case GADGET_INPUT_RENAME2(BUTTON_MP_NETWORK):
                    // uncomment when/if we have IPXManagerClass
                    /*
                    if (Ipx.Is_IPX()) {
                        MessageBoxClass msg;
                        msg.Process("Winsock failed to initialise!");
                        to_draw = true;
                        break;
                    }
                    */
                    game = GAME_IPX;
                    process = false;
                    break;
                case GADGET_INPUT_RENAME2(BUTTON_MP_INTERNET):
                    game = GAME_INTERNET;
                    process = false;
                    break;
                case KN_ESC:
                case GADGET_INPUT_RENAME2(BUTTON_MP_CANCEL):
                    // yes cancel sets campaign....
                    game = GAME_CAMPAIGN;
                    process = false;
                    break;
                default:
                    break;
            }
        }
    }
    return game;
}
