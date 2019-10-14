/**
 * @file
 *
 * @author CCHyper
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
#include "mapeditor.h"
#include "globals.h"
#include "control.h"
#include "callback.h"
#include "mouse.h"
#include "iomap.h"
#include "anim.h"

void Setup_Menu(int, char **, unsigned long, int, int);
int Check_Menu(int, char **, char *, long, int);
int Do_Menu(char **, int);

void Setup_Menu(int menu, char **options, unsigned long a3, int a4, int menuskip)
{
#ifndef CHRONOSHIFT_STANDALONE
    DEFINE_CALL(func, 0x0050158C, void, int, char **, unsigned long, int, int);
    func(menu, options, a3, a4, menuskip);
#endif
}

int Check_Menu(int menu, char **options, char *string, long a4, int a5)
{
#ifndef CHRONOSHIFT_STANDALONE
    DEFINE_CALL(func, 0x005016A0, int, int, char **, char *, long, int);
    return func(menu, options, string, a4, a5);
#endif
}

int Do_Menu(char **options, int a2)
{
#ifndef CHRONOSHIFT_STANDALONE
    DEFINE_CALL(func, 0x00501A48, int, char **, int);
    return func(options, a2);
#endif
}

class RightClickMenuClass : public ControlClass
{
public:
    RightClickMenuClass();
    virtual BOOL Action(unsigned flags, KeyNumType &key) override;
};


RightClickMenuClass rightclickmenu;

RightClickMenuClass::RightClickMenuClass() :
    ControlClass(CONTROL_EDITOR_VIEWPORT_INPUT, 0, 0, 0, 0,
        MOUSE_LEFT_PRESS | MOUSE_LEFT_HELD | MOUSE_LEFT_RLSE | MOUSE_LEFT_UP | MOUSE_RIGHT_PRESS, true)
{
}

int &g_UnknownKey = Make_Global<int>(0x00685154);

BOOL RightClickMenuClass::Action(unsigned flags, KeyNumType &key)
{
    DEBUG_LOG("Entered RightClickMenuClass::Action\n");
    if (flags & MOUSE_RIGHT_PRESS) {
        if (Map.Pending_Object() != nullptr && Map.Pending_Object()->Is_Techno()) {
            //PendingObjectPtr = nullptr;
            //PendingObjectTypePtr = nullptr;
            //PendingObjectOwner = HOUSES_NONE;
            Map.Set_Cursor_Shape();
        //} else if (DisplayRepairMode) {
        //    DisplayRepairMode = false;
        //} else if (DisplaySellMode) {
        //    DisplaySellMode = false;
        //} else if (PendingSuper == SPECIAL_NONE) {
        //    Unselect_All();
        //} else {
        //    PendingSuper = SPECIAL_NONE;
        }

        Map.Set_Default_Mouse(MOUSE_POINTER);

        char *menuopts[5];
        // menuopts[0] = "Copy";
        // menuopts[1] = "Paste";
        // menuopts[2] = "Paste Clip";
        // menuopts[3] = "Unselect";
        // menuopts[4] = '\0';
        menuopts[0] = "Fuck Shit up Here";
        // menuopts[1] = "Paste";
        // menuopts[2] = "Paste Clip";
        menuopts[1] = "Close";
        menuopts[2] = '\0';
        bool menuopen = true;

        g_keyboard->Get();

        while (menuopen) {
            Call_Back();
            g_mouse->Hide_Mouse();
            int menuaction = Do_Menu(menuopts, 1);
            g_mouse->Show_Mouse();
            if (g_UnknownKey == (KN_ESC | KEY_VK_BIT) || g_UnknownKey == (KN_LMOUSE | KEY_VK_BIT)
                || g_UnknownKey == (KN_RMOUSE | KEY_VK_BIT)) {
                break;
            }
            if (menuaction <= 3) {
                switch (menuaction) {
                    case 0: {
                        cell_t cell = Map.Click_Cell_Calc(g_wwmouse->Get_Mouse_X(), g_wwmouse->Get_Mouse_Y());
                        AnimClass::Do_Atom_Damage(HOUSES_NONE, cell);
                        menuopen = 0;
                        break;
                    }
                    case 1:
                        menuopen = 0;
                        break;
                    case 2:
                        menuopen = 0;
                        break;
                    case 3:
                        menuopen = 0;
                        break;
                }
            }
        }
    }
    return ControlClass::Action(0, key);
}

MapEditorClass::MapEditorClass() :
    GameMouseClass()
{
}

MapEditorClass::~MapEditorClass()
{
}

void MapEditorClass::One_Time()
{
    DEBUG_LOG("Entered MapEditorClass::One_Time\n");
    GameMouseClass::One_Time();
    rightclickmenu.Set_ID(CONTROL_EDITOR_VIEWPORT_INPUT);
    rightclickmenu.Set_Position(0, 16);
    rightclickmenu.Set_Size(240, 192);
}

void MapEditorClass::Init_IO()
{
    DEBUG_LOG("Entered MapEditorClass::Init_IO\n");
    GameMouseClass::Init_IO();
    rightclickmenu.Zap();
    Add_A_Button(rightclickmenu);
}

void MapEditorClass::AI(KeyNumType &key, int mouse_x, int mouse_y)
{
    GameMouseClass::AI(key, mouse_x, mouse_y);
}

void MapEditorClass::Draw_It(BOOL force_redraw)
{
    GameMouseClass::Draw_It(force_redraw);
}

void MapEditorClass::Detach(ObjectClass *object)
{
    GameMouseClass::Detach(object);
}

void MapEditorClass::Read_INI(GameINIClass &ini)
{
    GameMouseClass::Read_INI(ini);
}

BOOL MapEditorClass::Scroll_Map(DirType dir, int &distance, BOOL redraw)
{
    return GameMouseClass::Scroll_Map(dir, distance, redraw);
}

void MapEditorClass::Write_INI(GameINIClass &ini)
{
    GameMouseClass::Write_INI(ini);
}
