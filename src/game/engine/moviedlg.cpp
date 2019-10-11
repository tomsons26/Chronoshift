#include "moviedlg.h"

#include "callback.h"
#include "controlc.h"
#include "gamefile.h"
#include "gameloop.h"
#include "gbuffer.h"
#include "mouse.h"
#include "surfacemonitor.h"
#include "textbtn.h"
#include "tlist.h"

#include <stdio.h>

#define BUTTON_BACK 1
#define BUTTON_PLAY 2

//temp
#pragma pack(push, 1)
struct VQAHeader
{
    int iff[5];
    unsigned short Version;
    unsigned short Flags;
    unsigned short Frames;
    unsigned short ImageWidth;
    unsigned short ImageHeight;
    unsigned char BlockWidth;
    unsigned char BlockHeight;
    unsigned char FPS;
};
#pragma pack(pop)

// temp, to become a DVC
MovieChoiceClass SortedMovieTypes[] = {
    { "PROLOG", "Prologue", MOVIE_PROLOG, 0, false, false },
    { "ALLY1", "Allied Mission 1 - In The Thick Of It", MOVIE_ALLY1, 0, false, false },
    { "ALLY2", "Allied Mission 2 - Five To One", MOVIE_ALLY2, 0, false, false },
    { "SHIPYARD", "SHIPYARD SHOULD NOT SHOW", MOVIE_SHIPYARD, 0, false, false },
    { "SOVIET1", "SOVIET1 SHOULD SHOW", MOVIE_SOVIET1, 0, false, false },
    { 0 },
};

static void MovieChoiceClass::Scan()
{
    for (int i = 0; SortedMovieTypes[i].BaseName != 0; ++i) {
        VQAHeader header;
        char filename[100];
        sprintf(filename, "%s.vqa", SortedMovieTypes[i].BaseName);
        GameFileClass file(filename);
        if (file.Is_Available()) {
            file.Read(&header, sizeof(VQAHeader));
            SortedMovieTypes[i].Length = header.Frames / header.FPS;
            SortedMovieTypes[i].IsAvailable = true;
            file.Close();
        }
    }
}

void MovieChoiceClass::Draw_It(int index, int x, int y, int x_max, int y_max, BOOL selected, TextPrintType style)
{
    static int _tabs[] = { 10, 270 };
    RemapControlType *remapper = GadgetClass::Get_Color_Scheme();
    if (style & TPF_6PT_GRAD) {
        if (selected) {
            style |= TPF_USE_BRIGHT;
            g_logicPage->Fill_Rect(x, y, ((x + x_max) - 1), ((y + y_max) - 1), remapper->WindowPalette[0]);
        } else if (!(style & TPF_USE_GRAD_PAL)) {
            style |= TPF_USE_MEDIUM;
        }
    } else {
        remapper = (selected ? &ColorRemaps[REMAP_10] : &ColorRemaps[REMAP_5]);
    }
    char str[256];
    snprintf(str, sizeof(str), "\t%s\t%02d:%02d", Description, Length / 60, Length % 60);
    Conquer_Clip_Text_Print(str, x, y, remapper, COLOR_TBLACK, style, x_max, _tabs);
}

int MovieControlsClass::Process()
{
    int dialog_x = 106;
    int dialog_y = 80;
    int dialog_w = 428;
    int dialog_h = 240;
    int button_offset = 35;

    TListClass<MovieChoiceClass *> movielist(0,
        dialog_x + 40,
        dialog_y + 40,
        dialog_w - 40 * 2,
        dialog_h - 40 * 2,
        TPF_6PT_GRAD | TPF_NOSHADOW,
        GameFileClass::Retrieve("BTN-UP.SHP"),
        GameFileClass::Retrieve("BTN-DN.SHP"));
    // buttons
    
    TextButtonClass okbtn(BUTTON_BACK,
        "Back",
        TPF_CENTER | TPF_NOSHADOW | TPF_6PT_GRAD,
        dialog_w - button_offset,
        dialog_y + dialog_h - button_offset,
        100);
    TextButtonClass playbtn(BUTTON_PLAY,
        "Play Movie",
        TPF_CENTER | TPF_NOSHADOW | TPF_6PT_GRAD,
        dialog_x + button_offset,
        dialog_y + dialog_h - button_offset,
        100);

    // set graphic page to draw to
    Set_Logic_Page(g_seenBuff);

    // link all the gadgets
    GadgetClass *activegdt = &okbtn;
    movielist.Add_Tail(*activegdt);
    playbtn.Add_Tail(*activegdt);

    // fill up the list entries
    for (int i = 0; SortedMovieTypes[i].BaseName != 0; ++i) {
        if (SortedMovieTypes[i].Is_Allowed()) {
            movielist.Add_Item(&SortedMovieTypes[i]);
        }
    }
    BOOL process = true;
    BOOL to_draw = true;
    RemapControlType *scheme = GadgetClass::Get_Color_Scheme();
    static int _tabs[] = { 10, 270 };
    while (process) {
        Call_Back();
        if (g_allSurfaces.Surfaces_Restored()) {
            g_allSurfaces.Clear_Surfaces_Restored();
            to_draw = true;
        }

        if (to_draw) {
            g_mouse->Hide_Mouse();
            Dialog_Box(dialog_x, dialog_y, dialog_w, dialog_h);
            Draw_Caption("Select Movie", dialog_x, dialog_y, dialog_w);
            Conquer_Clip_Text_Print("Movie\tDuration\t",
                dialog_x + 68,
                dialog_y + 25,
                GadgetClass::Get_Color_Scheme(),
                0,
                TPF_CENTER | TPF_NOSHADOW | TPF_6PT_GRAD,
                700, // not sure what val i need here so it doesn't clip, need to use Conquer_Clip_Text_Print for tabs..
                _tabs);
            // Draw all linked gadgets
            activegdt->Draw_All();
            g_mouse->Show_Mouse();
            to_draw = false;
        }

        // handle keyboard and button input
        KeyNumType input = activegdt->Input();
        if (input != KN_NONE) {
            switch (input) {
                case KN_ESC:
                    process = false;
                    break;
                case KN_SPACE:
                case KN_RETURN:
                case GADGET_INPUT_RENAME2(BUTTON_PLAY):
                    Play_Movie(movielist.Current_Item()->Movie, THEME_NONE, false);
                    to_draw = true;
                    break;
                case GADGET_INPUT_RENAME2(BUTTON_BACK):
                    process = false;
                    break;
                default:
                    break;
            }
        }
    }
    // cleanup
    while (movielist.Count()) {
        MovieChoiceClass *choice = movielist.Get_Item(0);
        movielist.Remove_Item(choice);
    }
    return 0;
}
