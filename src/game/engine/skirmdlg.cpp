#include "bigcheck.h"
#include "callback.h"
#include "checkbox.h"
#include "colrlist.h"
#include "controlc.h"
#include "droplist.h"
#include "edit.h"
#include "gadget.h"
#include "gameevent.h"
#include "gameloop.h"
#include "gamemain.h"
#include "gameoptions.h"
#include "gauge.h"
#include "gbuffer.h"
#include "iomap.h"
#include "mouse.h"
#include "msgbox.h"
#include "multimission.h"
#include "queue.h"
#include "rules.h"
#include "session.h"
#include "slider.h"
#include "sndctrl.h"
#include "staticbtn.h"
#include "surfacemonitor.h"
#include "textbtn.h"
#include "vector.h"
#include "special.h"

class ColorButtonClass : public ToggleClass
{
public:
    ColorButtonClass(unsigned id, int x, int y, int w, int h, unsigned char color) :
        m_Color(color), ToggleClass(id, x, y, w, h)
    {
    }
    ColorButtonClass(ColorButtonClass &that) : ToggleClass(that) {}
    virtual ~ColorButtonClass() {}

    virtual BOOL Draw_Me(BOOL redraw) override;
    virtual BOOL Action(unsigned flags, KeyNumType &key) override;

    ColorButtonClass &operator=(ColorButtonClass &that);

private:
    unsigned char m_Color;
};

inline ColorButtonClass &ColorButtonClass::operator=(ColorButtonClass &that)
{
    if (this != &that) {
        ToggleClass::operator=(that);
    }

    return *this;
}

BOOL ColorButtonClass::Draw_Me(BOOL redraw)
{
    if (ControlClass::Draw_Me(redraw)) {
        g_Mouse->Hide_Mouse();
        Draw_Box(m_XPos, m_YPos, m_Width, m_Height, m_ToggleState ? BOX_STYLE_0 : BOX_STYLE_1, false);
        g_LogicPage->Fill_Rect(m_XPos + 1, m_YPos + 1, m_XPos + m_Width - 2, m_YPos + m_Height - 2, m_Color);
        g_Mouse->Show_Mouse();

        return true;
    }

    return false;
}

BOOL ColorButtonClass::Action(unsigned flags, KeyNumType &key)
{
    if (flags & MOUSE_LEFT_RLSE) {
        if (m_ToggleState) {
            Turn_Off();
        } else {
            Turn_On();
        }
    }

    return ToggleClass::Action(flags, key);
}

#define GAUGE 120
#define COLORBOX 130
#define CHECKBOX 140

void Clear_Vector(DynamicVectorClass<NodeNameTag *> *vector)
{
    for (int i = 0; i < vector->Count(); ++i) {
        delete &vector[i];
    }
    vector->Clear();
}

#include "expansion.h"

int Skirmish_Dialog(int)
{
    // catch the inital state
    bool redeploy = g_Rule.Allow_MCV_Undeploy();


    RemapControlType *remap = GadgetClass::Get_Color_Scheme();

    Is_Counterstrike_Installed();
    Is_Aftermath_Installed();
    ////
    static char namebuf[12];
    memset(namebuf, 0, sizeof(namebuf));
    EditClass name_edit(100, namebuf, 12, TPF_NOSHADOW | TPF_6PT_GRAD, 90, 29, 140, 18, (EditStyleType)7);

    Fancy_Text_Print("", 0, 0, 0, 0, TPF_NOSHADOW | TPF_6PT_GRAD);

    void *up_btn = GameMixFile::Retrieve("btn-up.shp");
    void *dn_btn = GameMixFile::Retrieve("btn-dn.shp");

    static char housetxt[25];
    DropListClass sidedrop(101, housetxt, 25, TPF_NOSHADOW | TPF_6PT_GRAD, 260, 29, 120, 80 + 30, up_btn, dn_btn);

    int colorbox_x = 420;
    int colorbox_y = 29;
    int colorbox_w = 20;
    int colorbox_h = 18;

    ColorButtonClass color1(COLORBOX, colorbox_x, colorbox_y, colorbox_w, colorbox_h, g_ColorRemaps[0].WindowPalette[4]);
    ColorButtonClass color2(
        COLORBOX + 1, colorbox_x + (colorbox_w), colorbox_y, colorbox_w, colorbox_h, g_ColorRemaps[1].WindowPalette[4]);
    ColorButtonClass color3(
        COLORBOX + 2, colorbox_x + (colorbox_w * 2), colorbox_y, colorbox_w, colorbox_h, g_ColorRemaps[2].WindowPalette[4]);
    ColorButtonClass color4(
        COLORBOX + 3, colorbox_x + (colorbox_w * 3), colorbox_y, colorbox_w, colorbox_h, g_ColorRemaps[3].WindowPalette[4]);
    ColorButtonClass color5(
        COLORBOX + 4, colorbox_x + (colorbox_w * 4), colorbox_y, colorbox_w, colorbox_h, g_ColorRemaps[4].WindowPalette[4]);
    ColorButtonClass color6(
        COLORBOX + 5, colorbox_x + (colorbox_w * 5), colorbox_y, colorbox_w, colorbox_h, g_ColorRemaps[5].WindowPalette[4]);
    ColorButtonClass color7(
        COLORBOX + 6, colorbox_x + (colorbox_w * 6), colorbox_y, colorbox_w, colorbox_h, g_ColorRemaps[6].WindowPalette[4]);
    ColorButtonClass color8(
        COLORBOX + 7, colorbox_x + (colorbox_w * 7), colorbox_y, colorbox_w, colorbox_h, g_ColorRemaps[7].WindowPalette[4]);

    ListClass maplist(106, 158, 65, 324, 156, TPF_NOSHADOW | TPF_6PT_GRAD, up_btn, dn_btn);

    int gauge_x = 158;
    int gauge_y = 228;
    int gauge_w = 100;

    // int old_thumb_size = GaugeClass::DefaultThumbSize;
    // GaugeClass::DefaultThumbSize = 10;

    GaugeClass ucountgauge(GAUGE, gauge_x, gauge_y, gauge_w, 13);
    StaticButtonClass ucounttxt(GAUGE + 1, "     ", TPF_NOSHADOW | TPF_6PT_GRAD, gauge_x + gauge_w + 6, gauge_y);
    gauge_y += 16;
    GaugeClass techgauge(GAUGE + 2, gauge_x, gauge_y, gauge_w, 13);
    StaticButtonClass techtxt(GAUGE + 3, "     ", TPF_NOSHADOW | TPF_6PT_GRAD, gauge_x + gauge_w + 6, gauge_y);

    gauge_y += 16;
    GaugeClass creditgauge(GAUGE + 4, gauge_x, gauge_y, gauge_w, 13);
    StaticButtonClass credittxt(GAUGE + 5, "     ", TPF_NOSHADOW | TPF_6PT_GRAD, gauge_x + gauge_w + 6, gauge_y, 45);

    gauge_y += 16;
    GaugeClass aigauge(GAUGE + 6, gauge_x, gauge_y, gauge_w, 13);
    StaticButtonClass aitxt(GAUGE + 7, "     ", TPF_NOSHADOW | TPF_6PT_GRAD, gauge_x + gauge_w + 6, gauge_y);

    int check_x = 310;
    int check_y = 227;
    int check_w = 130;

    BigCheckBoxClass basescheck(CHECKBOX, check_x, check_y, check_w, 13, "Bases", TPF_NOSHADOW | TPF_6PT_GRAD);
    check_y += 16;
    BigCheckBoxClass orecheck(CHECKBOX + 1, check_x, check_y, check_w, 13, "Ore Regenerates", TPF_NOSHADOW | TPF_6PT_GRAD);
    check_y += 16;
    BigCheckBoxClass cratescheck(CHECKBOX + 2, check_x, check_y, check_w, 13, "Crates Appear", TPF_NOSHADOW | TPF_6PT_GRAD);
    check_y += 16;
    BigCheckBoxClass shroudcheck(CHECKBOX + 3, check_x, check_y, check_w, 13, "Shroud Regrows", TPF_NOSHADOW | TPF_6PT_GRAD);

    check_x = 310 + check_w;
    check_y = 227;
    BigCheckBoxClass mcvcheck(CHECKBOX + 4, check_x, check_y, check_w, 13, "Redeployable MCV", TPF_NOSHADOW | TPF_6PT_GRAD);
    check_y += 16;
    BigCheckBoxClass shortcheck(CHECKBOX + 5, check_x, check_y, check_w, 13, "Short Game", TPF_NOSHADOW | TPF_6PT_GRAD);
    check_y += 16;
    BigCheckBoxClass engicheck(CHECKBOX + 6, check_x, check_y, check_w, 13, "Multi Engineer", TPF_NOSHADOW | TPF_6PT_GRAD);
    check_y += 16;
    BigCheckBoxClass supercheck(CHECKBOX + 7, check_x, check_y, check_w, 13, "Superweapons", TPF_NOSHADOW | TPF_6PT_GRAD);

    SliderClass diffslider(112, 90, 315, 460, 16, 1);

    int bottom_button_y = 355;

    TextButtonClass okbtn(109, 23, TPF_CENTER | TPF_NOSHADOW | TPF_6PT_GRAD, 61, bottom_button_y, 90, 18, 0);

    TextButtonClass cancelbtn(111, 19, TPF_CENTER | TPF_NOSHADOW | TPF_6PT_GRAD, 488, bottom_button_y, 90, 18, 0);

    Set_Logic_Page(&g_SeenBuff);

    GadgetClass *activegdgts = &name_edit;

    sidedrop.Add_Tail(*activegdgts);

    color1.Add_Tail(*activegdgts);
    color2.Add_Tail(*activegdgts);
    color3.Add_Tail(*activegdgts);
    color4.Add_Tail(*activegdgts);
    color5.Add_Tail(*activegdgts);
    color6.Add_Tail(*activegdgts);
    color7.Add_Tail(*activegdgts);
    color8.Add_Tail(*activegdgts);

    ColorButtonClass *colorbuttons[8];

    colorbuttons[0] = &color1;
    colorbuttons[1] = &color2;
    colorbuttons[2] = &color3;
    colorbuttons[3] = &color4;
    colorbuttons[4] = &color5;
    colorbuttons[5] = &color6;
    colorbuttons[6] = &color7;
    colorbuttons[7] = &color8;

    colorbuttons[0]->Turn_On();

    // todo decide how to toggle the buttons best
    bool colorstates[8] = { true, false, false, false, false, false, false, false };

    int color = g_Session.m_MPlayerColorIdx;
    if (color >= PLAYER_COLOR_10) {
        color = PLAYER_COLOR_9;
    }

    maplist.Add_Tail(*activegdgts);
    okbtn.Add_Tail(*activegdgts);
    cancelbtn.Add_Tail(*activegdgts);

    ucountgauge.Add_Tail(*activegdgts);
    ucounttxt.Add_Tail(*activegdgts);
    techgauge.Add_Tail(*activegdgts);
    techtxt.Add_Tail(*activegdgts);
    creditgauge.Add_Tail(*activegdgts);
    credittxt.Add_Tail(*activegdgts);
    aigauge.Add_Tail(*activegdgts);
    aitxt.Add_Tail(*activegdgts);

    basescheck.Add_Tail(*activegdgts);
    orecheck.Add_Tail(*activegdgts);
    cratescheck.Add_Tail(*activegdgts);
    shroudcheck.Add_Tail(*activegdgts);
    mcvcheck.Add_Tail(*activegdgts);
    shortcheck.Add_Tail(*activegdgts);
    engicheck.Add_Tail(*activegdgts);
    supercheck.Add_Tail(*activegdgts);

    diffslider.Add_Tail(*activegdgts);

    if (g_Rule.Fine_Diff_Control()) {
        diffslider.Set_Maximum(5);
        diffslider.Set_Value(2);
    } else {
        diffslider.Set_Maximum(3);
        diffslider.Set_Value(1);
    }

    for (HousesType i = HOUSES_FIRST; i < HOUSES_GOODGUY; ++i) {
        sidedrop.Add_Item(HouseTypeClass::As_Reference(i).Full_Name_String());

        // almost same, gives USSR instead of Russia
        // sidedrop.Add_Item(HouseTypeClass::As_Reference(i).Get_Name());
    }
    sidedrop.Set_Selected_Index(g_Session.m_MPlayerHouse);
    sidedrop.Ignore_Input(true);
    static bool _first_time = true;
    if (_first_time) {
        g_Session.m_Options.m_Bases = g_Rule.MPlayer_Bases();
        g_Session.m_Options.m_Credits = g_Rule.MPlayer_Money();
        g_Session.m_Options.m_Ore = g_Rule.MPlayer_OreGrows();
        g_Session.m_Options.m_Goodies = g_Rule.MPlayer_Crates();
        s_Special.Set_Shroud_Regrows(g_Rule.MPlayer_ShadowGrow());
        g_Session.m_Options.m_UnitCount = 6; // (SessionClass::CountMin[*&Rule.MultiplayerBitfield[0] << 30 >> 31] +
                                             // SessionClass::CountMax[*&Rule.MultiplayerBitfield[0] << 30 >> 31]) / 2;
        g_Session.m_Options.m_AIPlayers = 0;
        _first_time = false;
    }

    basescheck.Set_Toggle_State(g_Session.m_Options.m_Bases);
    orecheck.Set_Toggle_State(g_Session.m_Options.m_Ore);
    cratescheck.Set_Toggle_State(g_Session.m_Options.m_Goodies);
    shroudcheck.Set_Toggle_State(s_Special.Shroud_Regrows());
    mcvcheck.Set_Toggle_State(redeploy);

    techgauge.Set_Maximum(9);
    techgauge.Set_Value(g_BuildLevel - 1);

    creditgauge.Set_Maximum(g_Rule.MPlayer_Max_Money());
    creditgauge.Set_Value(g_Session.MPlayer_Options().m_Credits);

    aigauge.Set_Maximum(g_Rule.Max_Players() - 2);
    // i think...
    aigauge.Set_Value(g_Session.MPlayer_Options().m_AIPlayers - 1);

    Clear_Vector(&g_Session.m_Players);

    g_Session.MPlayer_Options().m_LocalID = 0;
    srand(time(0));
    g_Seed = rand();

    // todo maybe, map poplulation code is weird in original, uses a array with strings of all map names
    // investigate if this works well
    DynamicVectorClass<MultiMission *> &multimaps = g_Session.m_MPlayerScenarios;

    for (int i = 0; i < multimaps.Count(); ++i) {
        maplist.Add_Item(multimaps[i]->m_Description);
    }

    strncpy(namebuf, g_Session.m_MPlayerName, sizeof(g_Session.m_MPlayerName));
    name_edit.Set_Text(g_Session.m_MPlayerName, sizeof(g_Session.m_MPlayerName));

    name_edit.Set_Color_Scheme(&g_ColorRemaps[color]);

    bool process = true;

    int to_draw_level = 5;

    bool start_game = false;

    while (process) {
        Call_Back();
        if (g_AllSurfaces.Surfaces_Restored()) {
            g_AllSurfaces.Clear_Surfaces_Restored();
            to_draw_level = 5;
        }

        if (to_draw_level) {
            if (sidedrop.Is_Expanded()) {
                sidedrop.Collapse();
                to_draw_level = 5;
            }
            g_Mouse->Hide_Mouse();

            // total redraw of whole dialog
            if (to_draw_level >= 5) {
                Dialog_Box(0, 0, 640, 400);
                Fancy_Text_Print(0, 0, 0, 0, 0, TPF_CENTER | TPF_NOSHADOW | TPF_6PT_GRAD);

                Fancy_Text_Print(
                    TXT_YOUR_NAME, name_edit.Get_XPos(), name_edit.Get_YPos() - 13, remap, 0, TPF_NOSHADOW | TPF_6PT_GRAD);
                Fancy_Text_Print(
                    TXT_SIDE_COLON, sidedrop.Get_XPos(), sidedrop.Get_YPos() - 13, remap, 0, TPF_NOSHADOW | TPF_6PT_GRAD);
                Fancy_Text_Print(TXT_COLOR_COLON, colorbox_x, colorbox_y - 13, remap, 0, TPF_NOSHADOW | TPF_6PT_GRAD);

                Fancy_Text_Print(TXT_SCENARIOS,
                    maplist.Get_XPos() + maplist.Get_Width() / 2,
                    maplist.Get_YPos() - 13,
                    remap,
                    0,
                    TPF_CENTER | TPF_NOSHADOW | TPF_6PT_GRAD);

                Fancy_Text_Print(
                    TXT_UNIT_COUNT, gauge_x - 3, ucountgauge.Get_YPos(), remap, 0, TPF_RIGHT | TPF_NOSHADOW | TPF_6PT_GRAD);
                Fancy_Text_Print(
                    TXT_TECH_LEVEL, gauge_x - 3, techgauge.Get_YPos(), remap, 0, TPF_RIGHT | TPF_NOSHADOW | TPF_6PT_GRAD);
                Fancy_Text_Print(
                    TXT_CREDITS, gauge_x - 3, creditgauge.Get_YPos(), remap, 0, TPF_RIGHT | TPF_NOSHADOW | TPF_6PT_GRAD);
                Fancy_Text_Print(TXT_AI_PLAYERS_COLON,
                    gauge_x - 3,
                    aigauge.Get_YPos(),
                    remap,
                    0,
                    TPF_RIGHT | TPF_NOSHADOW | TPF_6PT_GRAD);

                Fancy_Text_Print(
                    TXT_EASY, diffslider.Get_XPos(), diffslider.Get_YPos() - 16, remap, 0, TPF_NOSHADOW | TPF_6PT_GRAD);
                Fancy_Text_Print(TXT_NORMAL,
                    diffslider.Get_XPos() + diffslider.Get_Width() / 2,
                    diffslider.Get_YPos() - 16,
                    remap,
                    0,
                    TPF_CENTER | TPF_NOSHADOW | TPF_6PT_GRAD);
                Fancy_Text_Print(TXT_HARD,
                    diffslider.Get_Width() + diffslider.Get_XPos(),
                    diffslider.Get_YPos() - 16,
                    remap,
                    0,
                    TPF_RIGHT | TPF_NOSHADOW | TPF_6PT_GRAD);
            }
            if (to_draw_level >= 1) {
                char buf[16];

                sprintf(buf, "%d", ucountgauge.Get_Value());
                ucounttxt.Set_Text(buf, false);
                ucounttxt.Draw_Me(false);

                sprintf(buf, "%d ", techgauge.Get_Value() + 1);
                techtxt.Set_Text(buf, false);
                techtxt.Draw_Me(false);

                sprintf(buf, "%d", creditgauge.Get_Value());
                credittxt.Set_Text(buf, false);
                credittxt.Draw_Me(false);

                sprintf(buf, "%d", aigauge.Get_Value() + 1);
                aitxt.Set_Text(buf, false);
                aitxt.Draw_Me(false);
            }

            if (to_draw_level >= 4) {
                activegdgts->Flag_List_To_Redraw();
                activegdgts->Draw_All(true);
            }

            g_Mouse->Show_Mouse();
            to_draw_level = 0;
        }

        KeyNumType input = activegdgts->Input();

        // not quite working
        if (input & 0x8000 && sidedrop.Is_Expanded()) {
            sidedrop.Collapse();
            to_draw_level = 5;
        }

        switch (input) {
            case GADGET_INPUT_RENAME2(100):
                break;

            case GADGET_INPUT_RENAME2(101):
                if (sidedrop.Is_Expanded()) {
                    sidedrop.Collapse();
                    to_draw_level = 5;
                }
                break;

            case GADGET_INPUT_RENAME2(GAUGE):
            case GADGET_INPUT_RENAME2(GAUGE + 2):
            case GADGET_INPUT_RENAME2(GAUGE + 4):
            case GADGET_INPUT_RENAME2(GAUGE + 6):
                to_draw_level = 1;
                break;

            case GADGET_INPUT_RENAME2(COLORBOX):
            case GADGET_INPUT_RENAME2(COLORBOX + 1):
            case GADGET_INPUT_RENAME2(COLORBOX + 2):
            case GADGET_INPUT_RENAME2(COLORBOX + 3):
            case GADGET_INPUT_RENAME2(COLORBOX + 4):
            case GADGET_INPUT_RENAME2(COLORBOX + 5):
            case GADGET_INPUT_RENAME2(COLORBOX + 6):
            case GADGET_INPUT_RENAME2(COLORBOX + 7): {
                color = input - GADGET_INPUT_RENAME2(COLORBOX);
                for (int i = 0; i < 8; ++i) {
                    colorbuttons[i]->Turn_Off();
                }
                colorbuttons[color]->Turn_On();

                name_edit.Set_Color_Scheme(&g_ColorRemaps[color]);
                name_edit.Flag_To_Redraw();

                break;
            }

            case GADGET_INPUT_RENAME2(109):
                start_game = true;
                process = false;
                break;

            // exit out of menu
            case KN_ESC:
            case GADGET_INPUT_RENAME2(111):
                process = false;
                break;

            case KN_RETURN:
                start_game = true;
                process = false;
                break;

            default:
                break;
        }
    }

    // todo, we exited the loop so set the values if they changed

    if (start_game) {
        g_Session.m_MPlayerCount = 1;

        // temp

        g_Scen.m_AIDifficulty = (DiffType)2;
        g_Scen.m_HumanDifficulty = (DiffType)0;
        g_Scen.m_ScenarioIndex = maplist.Current_Index();

        g_Session.m_MPlayerHouse = (HousesType)sidedrop.Current_Index();
        MPlayerOptionsStruct &mpopts = g_Session.MPlayer_Options();
        int map_index = maplist.Current_Index();
        // mpopts.m_LocalID = map_index;

        strncpy(g_Scen.m_ScenarioName, multimaps[map_index]->m_Filename, 44);

        strncpy(g_Session.m_MPlayerName, namebuf, sizeof(g_Session.m_MPlayerName));

        g_Session.m_MPlayerColorIdx = ((PlayerColorType)color);

        g_Session.m_Options.m_Credits = 500 * ((creditgauge.Get_Value() + 250) / 500);
        g_Session.m_Options.m_AIPlayers = aigauge.Get_Value() + 1;
        g_Session.m_Options.m_UnitCount = ucountgauge.Get_Value();

        g_Session.m_Options.m_Bases = basescheck.Get_Toggle_State();
        g_Session.m_Options.m_Ore = orecheck.Get_Toggle_State();
        g_Session.m_Options.m_Goodies = cratescheck.Get_Toggle_State();
        s_Special.Set_Shroud_Regrows(shroudcheck.Get_Toggle_State());
        g_Rule.Set_MCV_Undeploy(mcvcheck.Get_Toggle_State());

        int buildlvl = techgauge.Get_Value() + 1;
        if (buildlvl > 10) {
            buildlvl = 10;
        }
        g_BuildLevel = buildlvl;

        g_Session.Write_MultiPlayer_Settings();

        NodeNameTag *nametag = new NodeNameTag;

        memset(nametag, 0, sizeof(NodeNameTag));
        strncpy(nametag->m_Name, g_Session.m_MPlayerName, sizeof(nametag->m_Name));
        nametag->m_House = g_Session.m_MPlayerHouse;
        nametag->m_Color = g_Session.m_MPlayerColorIdx;
        nametag->m_AltHouse = HOUSES_NONE;

        g_Session.m_Players.Add(nametag);
    }

    while (maplist.Count()) {
        maplist.Remove_Item(maplist.Get_Item(0));
    }
    
    // restore inital state
    g_Rule.Set_MCV_Undeploy(redeploy);

    return start_game;
}

int Skirmish_Dialog2(int i)
{
    int retval;
    bool isold = MessageBoxClass().Process("Which Skirmish dialog to show", "new", "old", 0, 0) != 0;
    if (isold == false) {
        retval = Skirmish_Dialog(i);
    } else {
        int (*func)(int) = reinterpret_cast<int (*)(int)>(0x0051289C);
        retval = func(i);
    }

    RawFileClass ses("ses_new.dat");
    if (isold == true) {
        ses.Set_Name("ses_old.dat");
    }
    ses.Open(FM_WRITE);
    ses.Write(&g_Session, sizeof(SessionClass));
    ses.Close();

    RawFileClass scen("scen_new.dat");
    if (isold == true) {
        scen.Set_Name("scen_old.dat");
    }
    scen.Open(FM_WRITE);
    scen.Write(&g_Scen, sizeof(ScenarioClass));
    scen.Close();

    // captainslog_debug("Session Players has %d entries", g_Session.m_Players.Count());

    return retval;
}
