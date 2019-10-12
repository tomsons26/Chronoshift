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

#include "callback.h"
#include "gamefile.h"
#include "gameloop.h"
#include "gbuffer.h"
#include "ini.h"
#include "list.h"
#include "mouse.h"
#include "scenario.h"
#include "surfacemonitor.h"
#include "textbtn.h"
#include <cstdio>
#include <stdio.h>

class EObjectClass
{
public:
    void Add(char *name, char *filename, HousesType side, int scen_index);

public:
    HousesType m_Side;
    int m_Index;
    char m_Name[128];
    char m_Filename[128];
};

void EObjectClass::Add(char *name, char *filename, HousesType side, int scen_index)
{
    strcpy(m_Name, name);
    strcpy(m_Filename, filename);
    m_Side = side;
    m_Index = scen_index;
}

char *CSExpandNames[] = {
    "SCG20EA",
    "SCG21EA",
    "SCG22EA",
    "SCG23EA",
    "SCG24EA",
    "SCG26EA",
    "SCG27EA",
    "SCG28EA",
    "SCU31EA",
    "SCU32EA",
    "SCU33EA",
    "SCU34EA",
    "SCU35EA",
    "SCU36EA",
    "SCU37EA",
    "SCU38EA",
    nullptr,
};

char *AMExpandNames[] = {
    "SCG43EA",
    "SCG41EA",
    "SCG40EA",
    "SCG42EA",
    "SCG47EA",
    "SCG45EA",
    "SCG44EA",
    "SCG48EA",
    "SCG46EA",
    "SCU43EA",
    "SCU40EA",
    "SCU42EA",
    "SCU41EA",
    "SCU45EA",
    "SCU44EA",
    "SCU46EA",
    "SCU47EA",
    "SCU48EA",
    nullptr,
};

class EListClass : public ListClass
{
public:
    EListClass(int id, int x, int y, int w, int h, TextPrintType text_style, void *up_btn_shape, void *down_btn_shape);
    virtual ~EListClass() {}

    // ListClass

    // no clue why these existed, they are the same as listclass...
    // virtual int Add_Item(const char *string) override;
    // virtual int Add_Item(int str_id) override;
    // virtual const char *Current_Item() const override;
    // virtual const char *Get_Item(int string_index) const override;
    virtual void Draw_Entry(int index, int x, int y, int x_max, BOOL redraw) override;

    virtual int Add_Object(EObjectClass *object);
    virtual const EObjectClass *Get_Object(int index) const;
    virtual const EObjectClass *Current_Object() const;
};

EListClass::EListClass(
    int id, int x, int y, int w, int h, TextPrintType text_style, void *up_btn_shape, void *down_btn_shape) :
    ListClass(id, x, y, w, h, text_style, up_btn_shape, down_btn_shape)
{
}

void EListClass::Draw_Entry(int index, int x, int y, int x_max, BOOL redraw)
{
    char str[128];
    bool side = Get_Object(index)->m_Side == HOUSES_GOODGUY;
    sprintf(str, "%s: %s", Fetch_String(side ? TXT_ALLIES : TXT_SOVIET), Get_Object(index)->m_Name);
    int style = TextStyle;
    if (redraw) {
        style |= TPF_USE_BRIGHT;
        g_logicPage->Fill_Rect(x, y, x_max + x - 1, YSpacing + y - 1, GadgetClass::ColorScheme->WindowPalette[0]);//color was 1
    } else {
        if (!(style & TPF_USE_GRAD_PAL)) {
            style |= TPF_USE_MEDIUM;
        }
    }

    Conquer_Clip_Text_Print(str, x + 100, y, GadgetClass::ColorScheme, 0, (TextPrintType)style & ~TPF_CENTER, x_max, Tabs);
}

int EListClass::Add_Object(EObjectClass *obj)
{
    return ListClass::Add_Item(reinterpret_cast<char *>(&obj));
}

const EObjectClass *EListClass::Get_Object(int index) const
{
    return (EObjectClass *)ListClass::Get_Item(index);
}

const EObjectClass *EListClass::Current_Object() const
{
    return (EObjectClass *)ListClass::Current_Item();
}

//expansion dialog based on original code but without the crappyness
int Expansion_Dialog(int expansion)
{
    // buttons
    TextButtonClass okbtn(200, TXT_OK, TPF_CENTER | TPF_NOSHADOW | TPF_6PT_GRAD, 80, 316);
    TextButtonClass cancelbtn(201, TXT_CANCEL, TPF_CENTER | TPF_NOSHADOW | TPF_6PT_GRAD, 515, 316);

    // list itself
    EListClass elist(202,
        75,
        64,
        490,
        247,
        TPF_CENTER | TPF_NOSHADOW | TPF_6PT_GRAD,
        GameFileClass::Retrieve("BTN-UP.SHP"),
        GameFileClass::Retrieve("BTN-DN.SHP"));

    // link all the gadgets
    GadgetClass *activegdt = &okbtn;
    cancelbtn.Add_Tail(*activegdt);
    elist.Add_Tail(*activegdt);

    // fill up the list entries
    char name[128];
    // char filename[128]; why did this exist....
    GameFileClass file;
    INIClass ini;

    char **list = nullptr;
    // yes 0 is AM..
    if (expansion == 0) {
        list = AMExpandNames;
    } else if (expansion == 1) {
        list = CSExpandNames;
    } else {
        DEBUG_LOG("Expansion_Dialog got expansion as %d, U wat?\n", expansion);
        return 0;
    }
    static char temp_buffer[64];//was 128 but the name string is capped at 44 sooo
    for (int i = 0; list[i] != nullptr; ++i) {
        strcpy(name, list[i]);
        // strcpy(filename, list[i]);
        // if (filename[strlen(filename) - 1] == '\x0') {
        if (name[sizeof(name) - 1] != '\x0') {
            // name overflowed so we stop
            DEBUG_LOG("WARNING Expansion_Dialog - Filename overflowed!\n");
            break;
        }
        strcat(name, ".INI");
        // strcat(filename, ".INI");

        // new way of getting the index cause original was retarded
        char index[2];
        memcpy(index, &name[3], sizeof(index));
        int scen_idx = atoi(index);

        Scen.Set_Scenario_Name(name);
        Scen.Set_Scenario_Index(scen_idx);

        file.Set_Name(name);
        if (file.Is_Available()) {
            EObjectClass *eobj = new EObjectClass;
            switch (name[2]) {
                case 'G':
                case 'g': {
                    ini.Load(file);
                    ini.Get_String("Basic", "Name", "x", temp_buffer, sizeof(temp_buffer));
                    eobj->Add(temp_buffer, name, HOUSES_GOODGUY, scen_idx);
                    elist.Add_Item(reinterpret_cast<char *>(eobj));
                    ini.Clear();
                    // DEBUG_LOG("Added eobj with house %d, index %d, %s name, %s filename\n",
                    //    eobj->m_Side,
                    //    eobj->m_Index,
                    //    eobj->m_Name,
                    //    eobj->m_Filename);
                    break;
                }
                case 'U':
                case 'u': {
                    ini.Load(file);
                    ini.Get_String("Basic", "Name", "x", temp_buffer, sizeof(temp_buffer));
                    eobj->Add(temp_buffer, name, HOUSES_BADGUY, scen_idx);
                    elist.Add_Item(reinterpret_cast<char *>(eobj));
                    ini.Clear();
                    // DEBUG_LOG("Added eobj with house %d, index %d, %s name, %s filename\n",
                    //    eobj->m_Side,
                    //    eobj->m_Index,
                    //    eobj->m_Name,
                    //    eobj->m_Filename);
                    break;
                }
                default:
                    DEBUG_LOG("WARNING Expansion_Dialog - name[2] was %c\n", name[2]);
                    break;
            }
        }
    }
    // set graphic page to draw to
    Set_Logic_Page(g_seenBuff);

    BOOL process = true;
    BOOL to_draw = true;
    BOOL proceed = false;
    while (process) {
        Call_Back();
        if (g_allSurfaces.Surfaces_Restored()) {
            g_allSurfaces.Clear_Surfaces_Restored();
            to_draw = true;
        }

        if (to_draw) {
            g_mouse->Hide_Mouse();
            Dialog_Box(40, 34, 560, 332);
            Draw_Caption(expansion ? "Counterstrike Missions" : "Aftermath Missions", 40, 34, 560);

            // Draw all linked gadgets
            activegdt->Draw_All();
            g_mouse->Show_Mouse();
            to_draw = false;
        }

        // handle keyboard and button input
        KeyNumType input = activegdt->Input();
        if (input != KN_NONE) {
            switch (input) {
                case KN_RETURN:
                case GADGET_INPUT_RENAME2(200):
                    Whom = elist.Current_Object()->m_Side;
                    Scen.Set_Scenario_Index_And_Name(elist.Current_Object()->m_Index, elist.Current_Object()->m_Filename);
                    process = false;
                    proceed = true;
                    break;
                case KN_ESC:
                case GADGET_INPUT_RENAME2(201):
                    process = false;
                    proceed = false;
                    break;
                default:
                    break;
            }
        }
    }
    // cleanup
    while (elist.Count()) {
        const EObjectClass *item = elist.Get_Object(0);
        elist.Remove_Item((char *)item);
        delete item;
    }
    return proceed;
}

void Fill_Mission_List(EListClass &elist, char **list)
{
    GameFileClass file;
    INIClass ini;
    // fill up the list entries
    char name[128];
    static char temp_buffer[64]; // was 128 but the name string is capped at 44 sooo
    for (int i = 0; list[i] != nullptr; ++i) {
        strcpy(name, list[i]);
        if (name[sizeof(name) - 1] != '\x0') {
            // name overflowed so we stop
            DEBUG_LOG("WARNING Expansion_Dialog - Filename overflowed!\n");
            break;
        }
        strcat(name, ".INI");
        // new way of getting the index cause original was retarded
        char index[2];
        memcpy(index, &name[3], sizeof(index));
        int scen_idx = atoi(index);

        Scen.Set_Scenario_Name(name);
        Scen.Set_Scenario_Index(scen_idx);

        file.Set_Name(name);
        if (file.Is_Available()) {
            EObjectClass *eobj = new EObjectClass;
            switch (name[2]) {
                case 'G':
                case 'g': {
                    ini.Load(file);
                    ini.Get_String("Basic", "Name", "x", temp_buffer, sizeof(temp_buffer));
                    eobj->Add(temp_buffer, name, HOUSES_GOODGUY, scen_idx);
                    elist.Add_Item(reinterpret_cast<char *>(eobj));
                    ini.Clear();
                    break;
                }
                case 'U':
                case 'u': {
                    ini.Load(file);
                    ini.Get_String("Basic", "Name", "x", temp_buffer, sizeof(temp_buffer));
                    eobj->Add(temp_buffer, name, HOUSES_BADGUY, scen_idx);
                    elist.Add_Item(reinterpret_cast<char *>(eobj));
                    ini.Clear();
                    break;
                }
                default:
                    DEBUG_LOG("WARNING Expansion_Dialog - name[2] was %c\n", name[2]);
                    break;
            }
        }
    }
}

// my own take and design of a expansion dialog that covers both and seperates the expansions into tabs
int Expansion_Dialog3(int expansion)
{
    // buttons
    TextButtonClass okbtn(200, TXT_OK, TPF_CENTER | TPF_NOSHADOW | TPF_6PT_GRAD, 80, 316);
    TextButtonClass cancelbtn(201, TXT_CANCEL, TPF_CENTER | TPF_NOSHADOW | TPF_6PT_GRAD, 515, 316);

    int offset = 30;
    // list itself
    EListClass amelist(202,
        75,
        64 + offset,
        490,
        247 - offset,
        TPF_CENTER | TPF_NOSHADOW | TPF_6PT_GRAD,
        GameFileClass::Retrieve("BTN-UP.SHP"),
        GameFileClass::Retrieve("BTN-DN.SHP"));

    EListClass cselist(203,
        75,
        64 + offset,
        490,
        247 - offset,
        TPF_CENTER | TPF_NOSHADOW | TPF_6PT_GRAD,
        GameFileClass::Retrieve("BTN-UP.SHP"),
        GameFileClass::Retrieve("BTN-DN.SHP"));

    TextButtonClass csbtn(204, "Counterstrike Missions", TPF_CENTER | TPF_NOSHADOW | TPF_6PT_GRAD, 75, 64 + 5, 240, 20);
    TextButtonClass ambtn(
        205, "Aftermath Missions", TPF_CENTER | TPF_NOSHADOW | TPF_6PT_GRAD, 40 + 560 - 240 - 35, 64 + 5, 240, 20);

    // link all the gadgets
    GadgetClass *activegdt = &okbtn;
    cancelbtn.Add_Tail(*activegdt);
    
    csbtn.Add_Tail(*activegdt);
    ambtn.Add_Tail(*activegdt);


    cselist.Add_Tail(*activegdt);
    EListClass *activelist = &cselist;
    csbtn.Set_Toggle_Bool1(true);

    //fill up the lists
    Fill_Mission_List(amelist, AMExpandNames);
    Fill_Mission_List(cselist, CSExpandNames);

    // set graphic page to draw to
    Set_Logic_Page(g_seenBuff);

    BOOL process = true;
    BOOL to_draw = true;
    BOOL proceed = false;
    while (process) {
        Call_Back();
        if (g_allSurfaces.Surfaces_Restored()) {
            g_allSurfaces.Clear_Surfaces_Restored();
            to_draw = true;
        }

        if (to_draw) {
            g_mouse->Hide_Mouse();
            Dialog_Box(40, 34, 560, 332);
            Draw_Caption("Expansion Missions", 40, 34, 560);

            // Draw all linked gadgets
            activegdt->Draw_All();
            g_mouse->Show_Mouse();
            to_draw = false;
        }

        // handle keyboard and button input
        KeyNumType input = activegdt->Input();
        if (input != KN_NONE) {
            switch (input) {
                case KN_RETURN:
                case GADGET_INPUT_RENAME2(200):
                    Whom = activelist->Current_Object()->m_Side;
                    Scen.Set_Scenario_Index_And_Name(
                        activelist->Current_Object()->m_Index, activelist->Current_Object()->m_Filename);
                    process = false;
                    proceed = true;
                    break;
                case KN_ESC:
                case GADGET_INPUT_RENAME2(201):
                    process = false;
                    proceed = false;
                    break;
                case GADGET_INPUT_RENAME2(204):
                    if (activelist != &cselist) {
                    //remove AM list from links and add CS list
                        amelist.Remove();
                        cselist.Add_Tail(*activegdt);
                        activelist = &cselist;
                        //toggle the button states
                    }
                    csbtn.Set_Toggle_Bool1(true);
                    ambtn.Set_Toggle_Bool1(false);
                    to_draw = true;
                    break;
                case GADGET_INPUT_RENAME2(205):
                    if (activelist != &amelist) {
                        // remove CS list from links and add AM list
                        cselist.Remove();
                        amelist.Add_Tail(*activegdt);
                        activelist = &amelist;
                        }
                    // toggle the button states
                    csbtn.Set_Toggle_Bool1(false);
                    ambtn.Set_Toggle_Bool1(true);
                    to_draw = true;
                    break;
                default:
                    break;
            }
        }
    }
    // cleanup
    while (cselist.Count()) {
        const EObjectClass *item = cselist.Get_Object(0);
        cselist.Remove_Item((char *)item);
        delete item;
    }
    while (amelist.Count()) {
        const EObjectClass *item = amelist.Get_Object(0);
        amelist.Remove_Item((char *)item);
        delete item;
    }
    return proceed;
}
