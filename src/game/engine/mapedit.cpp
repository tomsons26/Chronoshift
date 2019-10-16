/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Part of IOMap stack handling the map editor interface.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "mapedit.h"
#include "building.h"
#include "callback.h"
#include "cell.h"
#include "edit.h"
#include "gamefile.h"
#include "gameini.h"
#include "gameloop.h"
#include "globals.h"
#include "house.h"
#include "infantry.h"
#include "iomap.h"
#include "mouse.h"
#include "msgbox.h"
#include "object.h"
#include "palette.h"
#include "drawshape.h"
#include "session.h"
#include "techno.h"
#include "terrain.h"
#include "tileset.h"
#include "unit.h"
#include "vessel.h"
#include "gameptr.h"
#include "triggertype.h"
#include "tlist.h"
#include <stdio.h>

int &UnknownKey = Make_Global<int>(0x00685154);

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

// temp
void Disect_Scenario_Name(
    const char *scen_name, int &a2, ScenarioPlayerEnum &player, ScenarioDirEnum &direction, ScenarioVarEnum &variation)
{
    DEFINE_CALL(func, 0x0053EF14, void, const char *, int &, ScenarioPlayerEnum &, ScenarioDirEnum &, ScenarioVarEnum &);
    func(scen_name, a2, player, direction, variation);
}

void Clear_Scenario()
{
    DEFINE_CALL(func, 0x0053AA94, void);
    func();
}

void Fill_In_Data()
{
    DEFINE_CALL(func, 0x0053A5C8, void);
    func();
}

int Read_Scenario_INI(const char *scen_name, int a2)
{
    DEFINE_CALL(func, 0x0053D06C, int, const char *, int);
    return func(scen_name, a2);
}

void Write_Scenario_INI(const char *scen_name)
{
    GameINIClass ini;

    // if available load existing scenario, this is for keeping custom entries you already had
    GameFileClass file(scen_name);
    if (file.Is_Available()) {
        ini.Load(file, true);
    }
    ini.Clear("Basic");
    ini.Put_String("Basic", "Name", Scen.ScenarioDescription);

    ini.Put_MovieType("Basic", "Intro", Scen.IntroMovie);
    ini.Put_MovieType("Basic", "Brief", Scen.BriefMovie);
    ini.Put_MovieType("Basic", "Win", Scen.WinMovie);
    ini.Put_MovieType("Basic", "Lose", Scen.LoseMovie);
    ini.Put_MovieType("Basic", "Action", Scen.ActionMovie);

    ini.Put_HousesType("Basic", "Player", g_PlayerPtr->What_Type());

    ini.Put_ThemeType("Basic", "Theme", Scen.TransitTheme);

    ini.Put_Fixed("Basic", "CarryOverMoney", Scen.CarryOverMoney);

    ini.Put_Bool("Basic", "ToCarryOver", Scen.ToCarryOver);
    ini.Put_Bool("Basic", "ToInherit", Scen.ToInherit);
    ini.Put_Bool("Basic", "TimerInherit", Scen.TimerInherit);
    ini.Put_Bool("Basic", "CivEvac", Scen.CivEvac);

    ini.Put_Int("Basic", "NewINIFormat", 3);

    ini.Put_Int("Basic", "CarryOverCap", Scen.CarryOverCap / 100);

    ini.Put_Bool("Basic", "EndOfGame", Scen.EndOfGame);
    ini.Put_Bool("Basic", "NoSpyPlane", Scen.NoSpyPlane);
    ini.Put_Bool("Basic", "SkipScore", Scen.SkipScore);
    ini.Put_Bool("Basic", "OneTimeOnly", Scen.OneTimeOnly);
    ini.Put_Bool("Basic", "SkipMapSelect", Scen.SkipMapSelect);
    ini.Put_Bool("Basic", "Official", true);
    ini.Put_Bool("Basic", "FillSilos", Scen.FillSilos);
    ini.Put_Bool("Basic", "TruckCrate", Scen.TruckCrate);

    HouseClass::Write_INI(ini);
    TeamTypeClass::Write_INI(ini);
    TriggerTypeClass::Write_INI(ini);
    Map.Write_INI(ini);
    TerrainClass::Write_INI(ini);
    UnitClass::Write_INI(ini);
    VesselClass::Write_INI(ini);
    InfantryClass::Write_INI(ini);
    BuildingClass::Write_INI(ini);

    DEFINE_CALL(BaseClass_Write_INI, 0x00426944, void, void *, GameINIClass &);
    DEFINE_CALL(OverlayClass_Write_INI, 0x0052736C, void, GameINIClass &);
    DEFINE_CALL(SmudgeClass_Write_INI, 0x005501E4, void, GameINIClass &);

    BaseClass_Write_INI((void *)0x0067F28C, ini);
    OverlayClass_Write_INI(ini);
    SmudgeClass_Write_INI(ini);

    // if briefing text exists write it out
    if (strlen(Scen.BriefingText) != 0) {
        ini.Put_TextBlock("Briefing", Scen.BriefingText);
    }
    RawFileClass rfile(scen_name);
    ini.Save(rfile, true);
}

// test function that replaces special dialog so this can be invoked
void MapEditClass::Spec(int)
{
    Map.Save_Scenario();
}

MapEditClass::Struct MapEditClass::c;
char MapEditClass::HealthBuf[20];

MissionType MapEditClass::MapEditMissions[9] = {
    MISSION_GUARD,
    MISSION_STICKY,
    MISSION_HARMLESS,
    MISSION_HARVEST,
    MISSION_AREA_GUARD,
    MISSION_RETURN,
    MISSION_AMBUSH,
    MISSION_HUNT,
    MISSION_SLEEP,
};

/**
 * @brief
 *
 * @address 0x0051D7E0 (beta)
 */
MapEditClass::MapEditClass() : GameMouseClass()
{
    c.m_TotalObjectCount = 0;
    c.m_CurrentEntry = 0;
    c.m_CurrentOwner = HOUSES_GOODGUY;
    c.m_GrabbedObject = nullptr;

    for (int i = 0; i < 9; ++i) {
        c.m_CategoryCounts[i] = 0;
        c.m_CurrentIndex[i] = 0;
    }

    c.m_TeamType = nullptr;
    c.m_TriggerType = nullptr;
    c.m_BasePercent = 100;
    Scen.Set_Waypoint(WAYPOINT_HOME, 0);
    CellClass::CurrentSelectedCell = 0;
    c.m_UnsavedChanges = false;
    c.m_Bit2 = false;
    c.m_BaseBuilding = false;
}

MapEditClass::~MapEditClass() {}

/**
 * @brief
 *
 * @address 0x0051D880 (beta)
 */
void MapEditClass::One_Time()
{
    // WIP
    GameMouseClass::One_Time();
    c.m_InputHandler = new ControlClass(515, 0, 8, 632, 392, MOUSE_LEFT_PRESS | MOUSE_LEFT_RLSE);
    c.m_HouseTypeList = new ListClass(508,
        10,
        100,
        60,
        90,
        TPF_NOSHADOW | TPF_EDITOR,
        GameFileClass::Retrieve("ebtn-up.shp"),
        GameFileClass::Retrieve("ebtn-dn.shp"));

    for (HousesType i = HOUSES_FIRST; i < HOUSES_COUNT; ++i) {
        c.m_HouseTypeList->Add_Item(HouseTypeClass::As_Reference(i).Get_Name());
    }

    c.m_MissionList = new ListClass(511,
        70,
        150,
        80,
        40,
        TPF_NOSHADOW | TPF_EDITOR,
        GameFileClass::Retrieve("ebtn-up.shp"),
        GameFileClass::Retrieve("ebtn-dn.shp"));

    for (int i = 0; i < 9; ++i) {
        c.m_MissionList->Add_Item(MissionClass::Name_From(MapEditMissions[i]));
    }
    c.m_HealthBarGauge = new TriColorGaugeClass(512, 200, 170, 50, 10);
    c.m_HealthBarGauge->Use_Thumb(true);
    c.m_HealthBarGauge->Set_Maximum(256);
    c.m_HealthBarGauge->Set_Red_Limit(62);
    c.m_HealthBarGauge->Set_Yellow_Limit(126);
    HealthBuf[0] = '\0';
    c.m_HealthLabel =
        new TextLabelClass(HealthBuf, 255, 181, GadgetClass::Get_Color_Scheme(), TPF_CENTER | TPF_OUTLINE | TPF_EDITOR);
    c.m_SellableButton = new TextButtonClass(509, "Sellable", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 255, 175, 60);
    c.m_RebuildButton = new TextButtonClass(510, "Rebuild", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 255, 185, 60);
    c.m_FacingDial = new Dial8Class(513, 160, 160, 30, 30, DIR_NORTH);
    c.m_BaseGauge = new GaugeClass(514, 250, 0, 50, 8);
    c.m_BaseLabel =
        new TextLabelClass("Base:", 247, 0, GadgetClass::Get_Color_Scheme(), TPF_RIGHT | TPF_NOSHADOW | TPF_EDITOR);
    c.m_BaseGauge->Set_Maximum(100);
    c.m_BaseGauge->Set_Value(c.m_BasePercent);
}

/**
 * @brief
 *
 * @address 0x0051DB7C (beta)
 */
void MapEditClass::Init_IO()
{
    if (g_InMapEditor) {
        GameScreenClass::Buttons = nullptr;
        Add_A_Button(*c.m_BaseGauge);
        Add_A_Button(*c.m_BaseLabel);
        Add_A_Button(*c.m_InputHandler);
    } else {
        GameMouseClass::Init_IO();
    }
}

/**
 * @brief
 *
 * @address 0x (beta)
 */
void MapEditClass::AI(KeyNumType &key, int mouse_x, int mouse_y)
{
    // all this is test code
    if (key == (KEY_CTRL_BIT | KN_F2)) {
        ScenarioInit = 0;
        if (g_InMapEditor && c.m_UnsavedChanges) {
            MessageBoxClass msg;
            int state = msg.Process("Save Changes?", 0xD, 0xE, 0, 0);
            g_hidPage.Clear(0);
            Flag_To_Redraw(true);
            Render();
            if (state == 0) {
                if (!Save_Scenario()) {
                    c.m_UnsavedChanges = false;
                    Toggle_Editor_Mode(false);
                    g_InMapEditor = false;
                } else {
                    key = KN_NONE;
                }
            }
        } else if (!g_InMapEditor) {
            c.m_UnsavedChanges = false;
        }
        Toggle_Editor_Mode(!g_InMapEditor);
        key = KN_NONE;
    }

    if (g_InMapEditor) {
        ++g_GameFrame;

        int mousexpos = g_mouse->Get_Mouse_X();
        int mouseypos = g_mouse->Get_Mouse_Y();
        int xoff = TacOffsetX + (CELL_PIXELS * DisplayWidth + 128) / 256;
        int yoff = TacOffsetX + (CELL_PIXELS * DisplayWidth + 128) / 256;

        // these ifs are probably horribly wrong....
        if (mousexpos > TacOffsetX && mousexpos < xoff && mouseypos > TacOffsetY) {
            if (mouseypos < yoff) {
                Override_Mouse_Shape((c.m_TriggerType != nullptr ? MOUSE_MOVE : MOUSE_POINTER));
            }
        }

        // these ifs are probably horribly wrong....
        if (mousexpos >= TacOffsetX && mousexpos <= xoff && mouseypos >= TacOffsetY && mouseypos <= yoff) {
            cell_t click = Click_Cell_Calc(mousexpos, mouseypos);
            if (click != -1) {
                Set_Cursor_Pos(click);
                if (PendingObjectTypePtr != nullptr) {
                    Flag_To_Redraw(true);
                }
            }
        }

        if (c.m_Bit2 && Mouse_Moved()) {
            if (PendingObjectTypePtr != nullptr) {
                Flag_To_Redraw(true);
                if (!Place_Object()) {
                    c.m_UnsavedChanges = true;
                    Start_Placement();
                }
            } else {
                if (c.m_GrabbedObject != nullptr) {
                    c.m_GrabbedObject->Mark(MARK_REDRAW);
                    if (!Move_Grabbed_Object()) {
                        c.m_UnsavedChanges = true;
                    }
                }
            }
        }

        TechnoClass *tptr = nullptr;

        if (CurrentObjects.Count()) {
            tptr = (TechnoClass *)CurrentObjects.Fetch_Head();
        }

        switch (key) {
            //
            // handle mouse and keyboard inputs
            //
            // handle mouse rightclick
            case KN_RMOUSE:
                // we cancelled base building or object placement
                if (PendingObjectTypePtr != nullptr) {
                    if (c.m_BaseBuilding) {
                        Cancel_Base_Building();
                    } else {
                        Cancel_Placement();
                    }
                }
                // we cancelled trigger placement
                if (c.m_TriggerType != nullptr) {
                    Stop_Trigger_Placement();
                }
                // remove the current popup controls
                if (CurrentObjects.Count()) {
                    tptr->Unselect();
                    Popup_Controls();
                }
                // because rightclick was made show the rightclick menu
                Main_Menu();
                key = KN_NONE;
                break;
            // handle tabbing to next object
            case KN_TAB:
                Select_Next();
                key = KN_NONE;
                break;
            // handle placing new objects via insert key
            case KN_E_INSERT:
                if (PendingObjectTypePtr != nullptr) {
                    // remove the current popup controls
                    if (CurrentObjects.Count()) {
                        tptr->Unselect();
                        Popup_Controls();
                    }
                    Start_Placement();
                }
                key = KN_NONE;
                break;
            // handle passability debug toggle
            case KN_F6:
                g_Debug_Passable = g_Debug_Passable == 0;
                g_hidPage.Clear(0);
                Flag_To_Redraw(true);
                key = KN_NONE;
                break;
            // place home waypoint
            case (KN_SHIFT_BIT | KN_HOME):
                if (CellClass::CurrentSelectedCell) {
                    if (Scen.Get_Waypoint(WAYPOINT_HOME) != -1) {
                        bool found = false;
                        for (int i = 0; i < WAYPOINT_COUNT; ++i) {
                            if (i != WAYPOINT_HOME && Scen.Get_Waypoint(WAYPOINT_HOME) == Scen.Get_Waypoint(i)) {
                                found = true;
                            }
                        }
                        if (!found) {
                            Array[Scen.Get_Waypoint(WAYPOINT_HOME)].Set_Bit16(false);
                            Flag_Cell(Scen.Get_Waypoint(WAYPOINT_HOME));
                        }
                    }
                    Scen.Set_Waypoint(WAYPOINT_HOME, CellClass::CurrentSelectedCell);
                    Array[CellClass::CurrentSelectedCell].Set_Bit16(true);
                    Flag_Cell(CellClass::CurrentSelectedCell);
                    c.m_UnsavedChanges = true;
                    key = KN_NONE;
                }
                break;
            // place reinforcement waypoint
            case (KN_SHIFT_BIT | KN_R):
                if (CellClass::CurrentSelectedCell && CellClass::CurrentSelectedCell != Scen.Get_Waypoint(WAYPOINT_HOME)) {
                    if (Scen.Get_Waypoint(WAYPOINT_REINF) != -1) {
                        bool found = false;
                        for (int i = 0; i < 101; ++i) {
                            if (i != WAYPOINT_REINF && Scen.Get_Waypoint(WAYPOINT_REINF) == Scen.Get_Waypoint(i)) {
                                found = true;
                            }
                        }
                        if (!found) {
                            Array[Scen.Get_Waypoint(WAYPOINT_REINF)].Set_Bit16(false);
                            Flag_Cell(Scen.Get_Waypoint(WAYPOINT_REINF));
                        }
                    }
                    Scen.Set_Waypoint(WAYPOINT_REINF, CellClass::CurrentSelectedCell);
                    Array[CellClass::CurrentSelectedCell].Set_Bit16(true);
                    Flag_Cell(CellClass::CurrentSelectedCell);
                    c.m_UnsavedChanges = true;
                    key = KN_NONE;
                }
                break;
            // handle removing flags and waypoints
            case (KN_ALT_BIT | KN_SPACE):
                if (CellClass::CurrentSelectedCell) {
                    for (int i = 0; i < WAYPOINT_HOME; ++i) {
                        if (Scen.Get_Waypoint(i) == CellClass::CurrentSelectedCell) {
                            Scen.Set_Waypoint(i, -1);
                        }
                    }
                    /* TODO makes no sense
                    for (i = 0; i < 8; ++i) {
                        v43 = v94 >> 0x18;
                        HouseClass::As_Pointer(i + 12);
                        if (HouseClass::As_Pointer(i + 12)
                            && CellClass::CurrentSelectedCell == HouseClass::As_Pointer(v43)->FlagLocation) {
                            v44 = As_Target(CellClass::CurrentSelectedCell);
                            v45 = HouseClass::As_Pointer(v43);
                            HouseClass::Flag_Remove(As_Target(CellClass::CurrentSelectedCell), 1);
                        }
                    }
                    */
                    if (CellClass::CurrentSelectedCell != Scen.Get_Waypoint(WAYPOINT_HOME)
                        && CellClass::CurrentSelectedCell != Scen.Get_Waypoint(WAYPOINT_REINF)) {
                        Array[CellClass::CurrentSelectedCell].Set_Bit16(false);
                    }
                    c.m_UnsavedChanges = true;
                    Flag_Cell(CellClass::CurrentSelectedCell);
                }
                key = KN_NONE;
                break;
            // handle start location flag placement
            case (KN_ALT_BIT | KN_1):
            case (KN_ALT_BIT | KN_2):
            case (KN_ALT_BIT | KN_3):
            case (KN_ALT_BIT | KN_4):
            case (KN_ALT_BIT | KN_5):
            case (KN_ALT_BIT | KN_6):
            case (KN_ALT_BIT | KN_7):
            case (KN_ALT_BIT | KN_8):
                if (CellClass::CurrentSelectedCell) {
                    HousesType house = (HousesType)g_keyboard->To_ASCII(key & 0xFF) - '1' + 12;
                    HouseClass *hptr = HouseClass::As_Pointer(house);
                    if (hptr != nullptr) {
                        hptr->Flag_Attach(CellClass::CurrentSelectedCell, 1);
                    }
                } else if (tptr != nullptr) {
                    HousesType house = (HousesType)g_keyboard->To_ASCII(key & 0xFF) - '1' + 12;
                    HouseClass *hptr = HouseClass::As_Pointer(house);
                    if (hptr != nullptr && tptr->What_Am_I() == RTTI_UNIT) {
                        hptr->Flag_Attach((UnitClass *)tptr, 1);
                    }
                }
                key = KN_NONE;
                break;
            case KN_LEFT:
                if (PendingObjectTypePtr != nullptr) {
                    Place_Prev();
                }
                key = KN_NONE;
                break;
            case KN_RIGHT:
                if (PendingObjectTypePtr != nullptr) {
                    Place_Next();
                }
                key = KN_NONE;
                break;
            case KN_PGUP:
                if (PendingObjectTypePtr != nullptr) {
                    Place_Prev_Category();
                }
                key = KN_NONE;
                break;
            case KN_PGDN:
                if (PendingObjectTypePtr != nullptr) {
                    Place_Next_Category();
                }
                key = KN_NONE;
                break;
            //
            // handle gadget inputs
            //
            // handle house listbox
            case GADGET_INPUT_RENAME2(508): {
                ListClass *list = (ListClass *)Buttons->Extract_Gadget(508);
                if (list->Current_Index() != tptr->Owner() && Change_House((HousesType)list->Current_Index())) {
                    c.m_UnsavedChanges = true;
                }
                g_hidPage.Clear(0);
                Buttons->Flag_List_To_Redraw();
                Flag_To_Redraw(true);
                key = KN_NONE;
                break;
            }

            // handle sellable button
            case GADGET_INPUT_RENAME2(509): {
                if (tptr->What_Am_I() == RTTI_BUILDING) {
                    BuildingClass *bptr = (BuildingClass *)tptr;
                    if (bptr->Class_Of().Get_TechLevel() != -1) {
                        bptr->Set_Sellable(!bptr->Sellable());
                        bptr->Mark(MARK_REDRAW);
                    }
                    bptr->Sellable() ? c.m_SellableButton->Turn_On() : c.m_SellableButton->Turn_Off();
                }
                key = KN_NONE;
                break;
            }

            // handle rebuild button
            case GADGET_INPUT_RENAME2(510): {
                if (tptr->What_Am_I() == RTTI_BUILDING) {
                    BuildingClass *bptr = (BuildingClass *)tptr;
                    if (bptr->Class_Of().Get_TechLevel() != -1) {
                        bptr->Set_Rebuild(!bptr->Rebuild());
                        bptr->Mark(MARK_REDRAW);
                    }
                    bptr->Rebuild() ? c.m_RebuildButton->Turn_On() : c.m_RebuildButton->Turn_Off();
                }

                key = KN_NONE;
                break;
            }

            // handle mission list
            case GADGET_INPUT_RENAME2(511): {
                MissionType mission = MapEditMissions[c.m_MissionList->Current_Index()];
                if (tptr->Is_Techno() && mission != tptr->Get_Mission()) {
                    tptr->Assign_Mission(mission);
                    Buttons->Flag_List_To_Redraw();
                    Flag_To_Redraw(true);
                    c.m_UnsavedChanges = true;
                }
                key = KN_NONE;
            }

            // handle health gauge
            case GADGET_INPUT_RENAME2(512): {
                if (tptr->Is_Techno()) {
                    // TODO
                    /*
                    v104 = (*(this->HealthBarGauge->c.g.l.vtable + 0x80))(
                        this->HealthBarGauge, this->HealthBarGauge->c.g.l.vtable, 256);
                    v105 = fixed::fixed(&v138, v104, 256);
                    v106 = (*(*(*CurrentObject.v.Vector + 0x11) + 0x34))(*CurrentObject.v.Vector, v105);
                    */
                    int health = 0; //(*v107 * *(v106 + 0x12A) + 0x80) >> 8;
                    if (health <= 0) {
                        health = 1;
                    }
                    if (health != tptr->Get_Health()) {
                        tptr->Set_Health(health);
                        g_hidPage.Clear(0);
                        Flag_To_Redraw(true);
                        c.m_UnsavedChanges = true;
                    }
                    sprintf(MapEditClass::HealthBuf, "%d", health);
                }
                key = KN_NONE;
            }
            // handle facing
            case GADGET_INPUT_RENAME2(513): {
                if (tptr->Is_Techno() && c.m_FacingDial->Get_Direction() != tptr->Get_Facing().Get_Current()) {
                    DirType dir = c.m_FacingDial->Get_Direction();
                    // set unit facing
                    tptr->Get_Facing().Set_Current(dir);
                    tptr->Get_Facing().Set_Desired(dir);
                    // set turret facing
                    if (tptr->What_Am_I() == RTTI_UNIT) {
                        UnitClass *uptr = (UnitClass *)tptr;
                        uptr->Get_Turret_Facing().Set_Current(dir);
                        uptr->Get_Turret_Facing().Set_Desired(dir);
                    }
                    g_hidPage.Clear(0);
                    Flag_To_Redraw(true);
                    c.m_UnsavedChanges = true;
                }
                key = KN_NONE;
            }

            // base slider
            case GADGET_INPUT_RENAME2(514): {
                if (c.m_BaseGauge->Get_Value() != c.m_BasePercent) {
                    c.m_BasePercent = c.m_BaseGauge->Get_Value();
                    Build_Base_To(c.m_BasePercent);
                    g_hidPage.Clear(0);
                    Flag_To_Redraw(true);
                }
                key = KN_NONE;
            }

            // editor input handler
            case GADGET_INPUT_RENAME2(515): {
                DEBUG_LOG("GADGET INPUT\n");
                if (g_keyboard->Down(VK_LBUTTON)) {
                    if (PendingObjectTypePtr != nullptr) {
                        if (!Place_Object()) {
                            c.m_UnsavedChanges = true;
                            Start_Placement();
                        }
                    } else if (c.m_TriggerType != nullptr) {
                        Place_Trigger();
                        c.m_UnsavedChanges = true;
                    } else {
                        if (!CurrentObjects.Count()) {
                            goto LABEL_444;
                        }
                        /*
                        v71 = TickCount.Accumulated;
                        if (TickCount.Started != 0xFFFFFFFF) {
                            if (WindowsTimer) {
                                v72 = WinTimerClass::Get_System_Tick_Count(WindowsTimer);
                            } else {
                                v72 = 0;
                            }
                            v71 += v72 - TickCount.Started;
                        }
                        if ((v71 - c.m_UpdateRate) >= 15) {
                        */
                        // complete guess
                        if (TickCountTimer.Time() - c.m_UpdateRate >= 15) {
                        LABEL_444:
                            if (Select_Object()) {
                                CellClass::CurrentSelectedCell =
                                    Click_Cell_Calc(g_keyboard->Get_MouseQX(), g_keyboard->Get_MouseQY());
                                g_hidPage.Clear();
                                Flag_To_Redraw(true);
                                Render();
                            } else {
                                CellClass::CurrentSelectedCell = 0;
                                Grab_Object();
                            }
                        }
                    }
                    /*
                    v74 = TickCount.Accumulated;
                    if (TickCount.Started != 0xFFFFFFFF) {
                        if (WindowsTimer) {
                            v75 = WinTimerClass::Get_System_Tick_Count(WindowsTimer);
                        } else {
                            v75 = 0;
                        }
                        v74 += v75 - TickCount.Started;
                    }
                    */
                    // complete guess
                    c.m_UpdateRate = TickCountTimer.Time();
                } else {
                    c.m_Bit2 = false;
                    c.m_GrabbedObject = nullptr;
                }
                key = KN_NONE;
                break;
            }
            default:
                break;
        }

        // this might had been in a switch case...
        if (CellClass::CurrentSelectedCell) {
            // Update_Waypoint((*flags & 0xEBFF) - 97);//wat
        }
        key = KN_NONE;
    }
    GameMouseClass::AI(key, mouse_x, mouse_y);
}

/**
 * @brief
 *
 * @address 0x0051F7BC (beta)
 */
void MapEditClass::Draw_It(BOOL force_redraw)
{
    GameMouseClass::Draw_It(force_redraw);
    if (g_InMapEditor) {
        Fancy_Text_Print(
            "Ore Total=%ld   ", 0, 0, GadgetClass::Get_Color_Scheme(), 12, TPF_NOSHADOW | TPF_EDITOR, TotalValue);
        if (GameScreenClass::Buttons != nullptr && force_redraw && CurrentObjects.Count() > 0) {
            char namebuf[56]; // 56 in beta, large enough for retail?
            CurrentObjects.Fetch_Head()->Full_Name();
            const char *name = Fetch_String(CurrentObjects.Fetch_Head()->Full_Name());
            sprintf(namebuf, "%s (%d)", name, CurrentObjects.Fetch_Head()->As_Target());
            Fancy_Text_Print(namebuf, 160, 0, &ColorRemaps[REMAP_7], 0, TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR);
        }
    }
}

/**
 * @brief
 *
 * @address 0x005200DC (beta)
 */
void MapEditClass::Detach(ObjectClass *object)
{
    if (c.m_GrabbedObject == object) {
        c.m_GrabbedObject = nullptr;
    }
}

/**
 * @brief
 *
 * @address 0x00520514 (beta)
 */
void MapEditClass::Read_INI(GameINIClass &ini)
{
    GameMouseClass::Read_INI(ini);
    c.m_BasePercent = Scen.Get_CarryOver_Percent();
    c.m_BaseGauge->Set_Value(c.m_BasePercent);
}

/**
 * @brief
 *
 * @address 0x005200A4 (beta)
 */
BOOL MapEditClass::Scroll_Map(DirType dir, int &distance, BOOL redraw)
{
    if (g_InMapEditor && redraw) {
        Flag_To_Redraw(true);
    }
    return GameMouseClass::Scroll_Map(dir, distance, redraw);
}

/**
 * @brief
 *
 * @address 0x00520544 (beta)
 */
void MapEditClass::Write_INI(GameINIClass &ini)
{
    GameMouseClass::Write_INI(ini);
    ini.Put_Int("Basic", "Percent", c.m_BasePercent);
}

/**
 * @brief
 *
 * @address 0x0051DC1C (beta)
 */
BOOL MapEditClass::Add_To_List(ObjectTypeClass *objecttype)
{
    if (objecttype != nullptr && c.m_TotalObjectCount < 601) {
        c.m_ObjectTypeList[c.m_TotalObjectCount] = objecttype;
        ++c.m_TotalObjectCount;
        switch (objecttype->What_Am_I()) {
            case RTTI_TEMPLATETYPE:
                ++c.m_CategoryCounts[0];
                break;
            case RTTI_OVERLAYTYPE:
                ++c.m_CategoryCounts[1];
                break;
            case RTTI_SMUDGETYPE:
                ++c.m_CategoryCounts[2];
                break;
            case RTTI_TERRAINTYPE:
                ++c.m_CategoryCounts[3];
                break;
            case RTTI_UNITTYPE:
                ++c.m_CategoryCounts[4];
                break;
            case RTTI_INFANTRYTYPE:
                ++c.m_CategoryCounts[5];
                break;
            case RTTI_VESSELTYPE:
                ++c.m_CategoryCounts[6];
                break;
            case RTTI_BUILDINGTYPE:
                ++c.m_CategoryCounts[7];
                break;
            case RTTI_AIRCRAFTTYPE:
                ++c.m_CategoryCounts[8];
                break;
            default:
                DEBUG_LOG("MapEditClass::Add_To_List added a unsupported object!\n");
                break;
        }
        return true;
    }
    return false;
}

/**
 * @brief Toggles between game and editor mode.
 *
 * @address 0x004BBC94 (beta)
 */
void MapEditClass::Toggle_Editor_Mode(BOOL editor_mode)
{
    if (editor_mode) {
        g_InMapEditor = true;
        g_Debug_Unshroud = true;
        Unselect_All();
        Map.Activate(0);
        Map.Init_IO();
        g_hiddenPage.Clear();
    } else {
        g_InMapEditor = false;
        g_Debug_Unshroud = false;
        Unselect_All();
        Map.Init_IO();
        g_hidPage.Clear();
    }
    Map.Flag_To_Redraw(true);
    Map.Render();
}

/**
 * @brief
 *
 * @address 0x00516FB8 (beta)
 */
BOOL MapEditClass::New_Scenario()
{
    int a2;
    ScenarioPlayerEnum player;
    ScenarioDirEnum direction;
    ScenarioVarEnum variation;
    Disect_Scenario_Name(Scen.Scenario_Name(), a2, player, direction, variation);
    if (g_PlayerPtr != nullptr) {
        switch (g_PlayerPtr->What_Type()) {
            case HOUSES_SPAIN:
                player = SCEN_PLAYER_SPAIN;
                break;
            case HOUSES_GREECE:
                player = SCEN_PLAYER_GREECE;
                break;
            case HOUSES_USSR:
                player = SCEN_PLAYER_USSR;
                break;
            default:
                DEBUG_LOG("MapEditClass::New_Scenario Unhandled House Type\n");
                break;
        }
    }
    if (Pick_Scenario("New Scenario", a2, player, direction, variation)) {
        return -1;
    }
    ++ScenarioInit;
    Clear_Scenario();
    Scen.Set_Scenario_Name(a2, player, direction, variation);

    for (HousesType i = HOUSES_FIRST; i < HOUSES_COUNT; ++i) {
        new HouseClass(i);
    }
    HouseClass *hptr;
    switch (player) {
        case SCEN_PLAYER_SPAIN:
            hptr = HouseClass::As_Pointer(HOUSES_SPAIN);
            hptr->Set_Human(true);
            g_PlayerPtr = hptr;
            // Base.Player = 2;
            c.m_CurrentOwner = HOUSES_GOODGUY;
            break;

        case SCEN_PLAYER_GREECE:
            hptr = HouseClass::As_Pointer(HOUSES_GREECE);
            hptr->Set_Human(true);
            g_PlayerPtr = hptr;
            // Base.Player = 2;
            c.m_CurrentOwner = HOUSES_GOODGUY;
            break;

        case SCEN_PLAYER_USSR:
            hptr = HouseClass::As_Pointer(HOUSES_USSR);
            hptr->Set_Human(true);
            g_PlayerPtr = hptr;
            // Base.Player = 0;
            c.m_CurrentOwner = HOUSES_GOODGUY; // shouldn't this be BADGUY?...
            break;

        case SCEN_PLAYER_SPECIAL:
        case 4:
            break;

        case 5:
            hptr = HouseClass::As_Pointer(HOUSES_MULTI_FIRST);
            hptr->Set_Human(true);
            g_PlayerPtr = hptr;
            c.m_CurrentOwner = HOUSES_MULTI_FIRST;
            break;

        default:
            DEBUG_LOG("MapEditClass::New_Scenario Unhandled Player value!\n");
            break;
    }

    Fill_In_Data();
    // TODO
    MapEditClass::Size_Map(-1, -1, 30, 30);
    // Scen.Waypoints[WAYPOINT_REINF]
    // Scen.Waypoints[WAYPOINT_HOME]
    // Flag_Cell
    // Set_Tactical_Position
    --ScenarioInit;
    return 0;
}

/**
 * @brief
 *
 * @address 0x005172A0 (beta)
 */
BOOL MapEditClass::Load_Scenario()
{
    int a2;
    ScenarioDirEnum direction;
    ScenarioVarEnum variation;
    ScenarioPlayerEnum player;

    Disect_Scenario_Name(Scen.Scenario_Name(), a2, player, direction, variation);
    if (Pick_Scenario("Load Scenario", a2, player, direction, variation)) {
        return -1;
    }

    Scen.Set_Scenario_Name(a2, player, direction, variation);
    /*
    if ( player == 5 )
    {
        Clear_Vector(&Session.Players, v4, v5);
        v6 = operator new(0x1Du);
        if ( v6 )
        {
            v6 = &IPXAddressClass::IPXAddressClass((v6 + 0xC))[0xFFFFFFFE].byte8;
        }
        v7 = Session.MPlayerName;
        v8 = v6;
        v14 = v6;
        do
        {
            v9 = *v7;
            *v8 = *v7;
            if ( !v9 )
            {
                break;
            }
            v10 = v7[1];
            v7 += 2;
            v8[1] = v10;
            v8 += 2;
        }
        while ( v10 );
        *(v14 + 0x16) = Session.MPlayerHouse;
        *(v14 + 0x17) = Session.MPlayerColorIdx;
        DynamicVectorClass<NodeNameTag *>::Add(&Session.Players, &v14);
        Session.MPlayerCount = 1;
        a1->CurrentOwner = 12;
    } else */
    {
        c.m_CurrentOwner = HOUSES_GOODGUY;
    }
    Clear_Scenario();
    if (Read_Scenario_INI(Scen.Scenario_Name(), 1)) {
        Fill_In_Data();
        GamePalette.Set(0, nullptr);
    } else {
        MessageBoxClass msg(TXT_EDITOR_UNABLE_TO_READ);
        static char msgbuf[256];
        sprintf(msgbuf, "Unable to read scenario %s!\n", Scen.Scenario_Name());
        msg.Process(msgbuf, 0x17, 0, 0, 0);
        g_hidPage.Clear(0);
        Flag_To_Redraw(true);
        Render();
    }

    return 0;
}

/**
 * @brief
 *
 * @address 0x005174B8 (beta)
 */
int MapEditClass::Save_Scenario()
{
    int a2;
    ScenarioDirEnum direction;
    ScenarioVarEnum variation;
    ScenarioPlayerEnum player;

    Disect_Scenario_Name(Scen.Scenario_Name(), a2, player, direction, variation);
    if (Pick_Scenario("Save Scenario", a2, player, direction, variation)) {
        return -1;
    }
    Scen.Set_Scenario_Name(a2, player, direction, variation);

    HouseClass *hptr;
    switch (player) {
        case SCEN_PLAYER_SPAIN:
            hptr = HouseClass::As_Pointer(HOUSES_SPAIN);
            hptr->Set_Human(true);
            g_PlayerPtr = hptr;
            c.m_CurrentOwner = HOUSES_GOODGUY;
            break;
        case SCEN_PLAYER_GREECE:
            hptr = HouseClass::As_Pointer(HOUSES_GREECE);
            hptr->Set_Human(true);
            g_PlayerPtr = hptr;
            c.m_CurrentOwner = HOUSES_GOODGUY;
            break;
        case SCEN_PLAYER_USSR:
            hptr = HouseClass::As_Pointer(HOUSES_USSR);
            hptr->Set_Human(true);
            g_PlayerPtr = hptr;
            c.m_CurrentOwner = HOUSES_GOODGUY; // shouldn't this be BADGUY?...
            break;
        case SCEN_PLAYER_SPECIAL:
            DEBUG_LOG("Unhandled Player value in Save_Scenario SCEN_PLAYER_SPECIAL\n");
            break;
        default:
            DEBUG_LOG("Unhandled Player value in Save_Scenario!\n");
            break;
    }
    Write_Scenario_INI(Scen.Scenario_Name());
    return 0;
}

/**
 * @brief
 *
 * @address 0x (beta)
 */
int MapEditClass::Scenario_Dialog()
{
    return 0;
}

/**
 * @brief
 *
 * @address 0x005175D8 (beta)
 */
int MapEditClass::Pick_Scenario(
    char *dialog_title, int &a2, ScenarioPlayerEnum &player, ScenarioDirEnum &direction, ScenarioVarEnum &variation)
{
    TextButtonClass northbtn(0x64, "North (Spain)", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 0x73, 0x5B, 0x5A, 9);
    TextButtonClass southbtn(0x65, "South (Greece)", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 0x73, 0x64, 0x5A, 9);
    TextButtonClass housebtn(
        0x66, HouseTypeClass::As_Reference(HOUSES_USSR).Get_Name(), TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 115, 109, 90, 9);
    TextButtonClass multibtn(0x67, "Multiplayer", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 0x73, 0x76, 0x5A, 9);

    TextButtonClass eastbtn(0x68, "East", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 0x69, 0x4B, 0x32, 9);
    TextButtonClass westbtn(0x69, "West", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 0xA5, 0x4B, 0x32, 9);

    TextButtonClass okbtn(0x6A, 23, TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 0x6E, 0x97, 0x2D, 9);
    TextButtonClass cancelbtn(0x6B, 0x13, TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 0xA5, 0x97, 0x2D, 9);

    static char scen_buf[12];
    EditClass edit(0x6C, scen_buf, 5, TPF_NOSHADOW | TPF_EDITOR, 165, 43, 45, 9, EDIT_NUMS);

    TextButtonClass abtn(0x6D, "A", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 128, 59, 13, 9);
    TextButtonClass bbtn(0x6E, "B", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 141, 59, 13, 9);
    TextButtonClass cbtn(0x6F, "C", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 154, 59, 13, 9);
    TextButtonClass dbtn(0x70, "D", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 167, 59, 13, 9);

    Set_Logic_Page(g_seenBuff);
    sprintf(scen_buf, "%d", a2);
    edit.Set_Text(scen_buf, 5);
    abtn.Turn_Off();
    bbtn.Turn_Off();
    cbtn.Turn_Off();
    dbtn.Turn_Off();

    switch (variation) {
        case SCEN_VAR_A:
            abtn.Turn_On();
            break;
        case SCEN_VAR_B:
            bbtn.Turn_On();
            break;
        case SCEN_VAR_C:
            cbtn.Turn_On();
            break;
        case SCEN_VAR_D:
            dbtn.Turn_On();
            break;
        default:
            break;
    }

    GadgetClass *activegdt = &edit;
    abtn.Add_Tail(*activegdt);
    bbtn.Add_Tail(*activegdt);
    cbtn.Add_Tail(*activegdt);
    dbtn.Add_Tail(*activegdt);
    northbtn.Add_Tail(*activegdt);
    southbtn.Add_Tail(*activegdt);
    housebtn.Add_Tail(*activegdt);
    multibtn.Add_Tail(*activegdt);
    eastbtn.Add_Tail(*activegdt);
    westbtn.Add_Tail(*activegdt);
    okbtn.Add_Tail(*activegdt);
    cancelbtn.Add_Tail(*activegdt);

    northbtn.Turn_Off();
    southbtn.Turn_Off();
    housebtn.Turn_Off();
    multibtn.Turn_Off();

    if (player == 5) {
        multibtn.Turn_On();
    } else if (g_PlayerPtr != nullptr) {
        switch (g_PlayerPtr->What_Type()) {
            case HOUSES_SPAIN:
                northbtn.Turn_On();
                break;
            case HOUSES_GREECE:
                southbtn.Turn_On();
                break;
            case HOUSES_USSR:
                housebtn.Turn_On();
                break;
            default:
                DEBUG_LOG("MapEditClass::Pick_Scenario Unhandled House Type\n");
                break;
        }
    }
    eastbtn.Turn_Off();
    westbtn.Turn_Off();
    ToggleClass *dirbtn;
    if (direction) {
        dirbtn = &westbtn;
    } else {
        dirbtn = &eastbtn;
    }
    dirbtn->Turn_On();
    bool process = true;
    bool to_draw = true;
    bool cancelled = false;
    while (process) {
        Call_Back();
        if (to_draw) {
            g_mouse->Hide_Mouse();
            Dialog_Box(60, 18, 200, 164);
            Draw_Caption(dialog_title, 60, 18, 200);
            Fancy_Text_Print("Scenario", 155, 43, GadgetClass::Get_Color_Scheme(), 0, TPF_RIGHT | TPF_NOSHADOW | TPF_EDITOR);
            activegdt->Draw_All(true);
            g_mouse->Show_Mouse();
            to_draw = false;
        }
        KeyNumType input = activegdt->Input();
        if (input != KN_NONE) {
            switch (input) {
                case KN_ESC:
                    process = false;
                    cancelled = true;
                    break;
                case KA_RETURN:
                case GADGET_INPUT_RENAME2(0x6A):
                    process = false;
                    cancelled = false;
                    break;
                // done
                case GADGET_INPUT_RENAME2(0x64):
                case GADGET_INPUT_RENAME2(0x65):
                case GADGET_INPUT_RENAME2(0x66):
                case GADGET_INPUT_RENAME2(0x67): {
                    northbtn.Turn_Off();
                    southbtn.Turn_Off();
                    housebtn.Turn_Off();
                    multibtn.Turn_Off();
                    switch (input - GADGET_INPUT_RENAME2(0x64)) {
                        case 0:
                            player = SCEN_PLAYER_SPAIN;
                            northbtn.Turn_On();
                            break;
                        case 1:
                            player = SCEN_PLAYER_GREECE;
                            southbtn.Turn_On();
                            break;
                        case 2:
                            player = SCEN_PLAYER_USSR;
                            housebtn.Turn_On();
                            break;
                        case 3:
                            player = SCEN_PLAYER_5;
                            multibtn.Turn_On();
                            break;
                        default:
                            break;
                    }
                    break;
                }
                // done
                case GADGET_INPUT_RENAME2(0x68):
                case GADGET_INPUT_RENAME2(0x69): {
                    westbtn.Turn_Off();
                    eastbtn.Turn_Off();
                    switch (input - GADGET_INPUT_RENAME2(0x68)) {
                        case 0:
                            direction = SCEN_DIR_EAST;
                            eastbtn.Turn_On();
                            break;
                        case 1:
                            direction = SCEN_DIR_WEST;
                            westbtn.Turn_On();
                            break;
                        default:
                            break;
                    }
                    break;
                }
                case GADGET_INPUT_RENAME2(0x6B):
                    process = false;
                    cancelled = true;
                    break;
                case GADGET_INPUT_RENAME2(0x6C):
                    process = false;
                    break;
                // done
                case GADGET_INPUT_RENAME2(0x6D):
                case GADGET_INPUT_RENAME2(0x6E):
                case GADGET_INPUT_RENAME2(0x6F):
                case GADGET_INPUT_RENAME2(0x70): {
                    abtn.Turn_Off();
                    bbtn.Turn_Off();
                    cbtn.Turn_Off();
                    dbtn.Turn_Off();
                    switch (input - GADGET_INPUT_RENAME2(0x6D)) {
                        case 0:
                            variation = SCEN_VAR_A;
                            abtn.Turn_On();
                            break;
                        case 1:
                            variation = SCEN_VAR_B;
                            bbtn.Turn_On();
                            break;
                        case 2:
                            variation = SCEN_VAR_C;
                            cbtn.Turn_On();
                            break;
                        case 3:
                            variation = SCEN_VAR_D;
                            dbtn.Turn_On();
                            break;
                        default:
                            break;
                    }
                    break;
                }
            }
        }
    }

    g_hidPage.Clear(0);
    Flag_To_Redraw(true);
    Render();

    if (cancelled) {
        return -1;
    }

    a2 = atoi(scen_buf);
    return 0;
}
/**
 * @brief
 *
 * @address 0x (beta)
 */
int MapEditClass::Size_Map(int a1, int a2, int a3, int a4)
{
    return 0;
}

/**
 * @brief
 *
 * @address 0x005208E0 (beta)
 */
void MapEditClass::Popup_Controls()
{
    Remove_A_Button(*c.m_HouseTypeList);
    Remove_A_Button(*c.m_MissionList);
    Remove_A_Button(*c.m_HealthBarGauge);
    Remove_A_Button(*c.m_HealthLabel);
    Remove_A_Button(*c.m_FacingDial);

    // its added back anyway afterwards and there's no reason to remove them...
    Remove_A_Button(*c.m_BaseGauge);
    Remove_A_Button(*c.m_BaseLabel);
    Remove_A_Button(*c.m_InputHandler);

    Remove_A_Button(*c.m_SellableButton);
    Remove_A_Button(*c.m_RebuildButton);

    if (CurrentObjects.Count()) {
        TechnoClass *tptr = (TechnoClass *)CurrentObjects.Fetch_Head();
        if (tptr->Is_Techno()) {
            int current_mission = 0;
            for (int i = 0; i < 9; ++i) {
                if (tptr->Get_Mission() == MapEditClass::MapEditMissions[i]) {
                    current_mission = i;
                }
            }
            switch (tptr->Class_Of().What_Am_I()) {
                // mobile objects
                case RTTI_AIRCRAFTTYPE:
                case RTTI_INFANTRYTYPE:
                case RTTI_UNITTYPE:
                case RTTI_VESSELTYPE:
                    c.m_HealthBarGauge->Set_Value(tptr->Get_Health());
                    Add_A_Button(*c.m_HealthBarGauge);
                    c.m_HouseTypeList->Set_Selected_Index(tptr->Owner());
                    Add_A_Button(*c.m_HouseTypeList);
                    c.m_MissionList->Set_Selected_Index(current_mission);
                    Add_A_Button(*c.m_MissionList);
                    sprintf(HealthBuf, "%d", tptr->Get_Health());
                    Add_A_Button(*c.m_HealthLabel);
                    c.m_FacingDial->Set_Direction(tptr->Get_Facing().Get_Current());
                    Add_A_Button(*c.m_FacingDial);
                    break;
                // immobile objects
                case RTTI_BUILDINGTYPE: {
                    BuildingClass *bptr = (BuildingClass *)tptr;
                    c.m_HealthBarGauge->Set_Value(tptr->Get_Health());
                    Add_A_Button(*c.m_HealthBarGauge);
                    c.m_HouseTypeList->Set_Selected_Index(tptr->Owner());
                    Add_A_Button(*c.m_HouseTypeList);
                    sprintf(HealthBuf, "%d", tptr->Get_Health());
                    Add_A_Button(*c.m_HealthLabel);
                    Add_A_Button(*c.m_SellableButton);
                    bptr->Sellable() ? c.m_SellableButton->Turn_On() : c.m_SellableButton->Turn_Off();
                    Add_A_Button(*c.m_SellableButton);
                    bptr->Rebuild() ? c.m_RebuildButton->Turn_On() : c.m_RebuildButton->Turn_Off();
                    Add_A_Button(*c.m_RebuildButton);
                    if (bptr->Class_Of().Is_Turret_Equipped()) {
                        c.m_FacingDial->Set_Direction(tptr->Get_Facing().Get_Current());
                        Add_A_Button(*c.m_FacingDial);
                    }
                    break;
                }
                // catch cases we could make use of the in future
                case RTTI_OVERLAYTYPE:
                case RTTI_SMUDGETYPE:
                case RTTI_TEMPLATETYPE:
                case RTTI_TERRAINTYPE:
                    break;
                default:
                    DEBUG_LOG("MapEditClass::Popup_Controls Currently unsupported object selected %d\n",
                        tptr->Class_Of().What_Am_I());
                    break;
            }
        }
    }
    Add_A_Button(*c.m_BaseGauge);
    Add_A_Button(*c.m_BaseLabel);
    Add_A_Button(*c.m_InputHandler);
}

/**
 * @brief
 *
 * @address 0x0051FF44 (beta)
 */
void MapEditClass::AI_Menu()
{
    char *menu[4];

    menu[0] = "Pre-Build a Base";
    menu[1] = "Edit Triggers";
    menu[2] = "Edit Teams";
    menu[3] = nullptr;
    Override_Mouse_Shape(MOUSE_POINTER);
    bool process = true;
    while (process) {
        Call_Back();
        g_mouse->Hide_Mouse();
        int choice = Do_Menu(menu, 1);
        g_mouse->Show_Mouse();

        if (UnknownKey == 0x101B || UnknownKey == 0x1001 || UnknownKey == 0x1002) {
            break;
        }

        switch (choice) {
            case 0:
                Start_Base_Building();
                process = 0;
                break;
            case 1:
                Handle_Triggers();
                if (c.m_TriggerType != nullptr) {
                    Start_Trigger_Placement();
                }
                process = false;
                break;
            case 2:
                Handle_Teams("Teams");
                process = false;
                break;
            default:
                break;
        }
    }
}

/**
 * @brief
 *
 * @address 0x (beta)
 */
void MapEditClass::Handle_Teams(char *a1) {}

/**
 * @brief
 *
 * @address 0x0051BD18 (beta)
 */
void MapEditClass::Handle_Triggers()
{
    switch (Select_Trigger()) {
        case 0:
        default:
            break;

        case 1:
            if (c.m_TriggerType != nullptr) {
                if (c.m_TriggerType->Edit()) {
                    c.m_UnsavedChanges = true;
                }
                g_hidPage.Clear(0);
                Flag_To_Redraw(true);
                Render();
            }
            break;
        case 2:
            c.m_TriggerType = new TriggerTypeClass();
            if (c.m_TriggerType != nullptr) {
                if (c.m_TriggerType->Edit()) {
                    c.m_UnsavedChanges = true;
                } else {
                    if (c.m_TriggerType) {
                        delete c.m_TriggerType;
                    }
                    c.m_TriggerType = nullptr;
                }
            } else {
                MessageBoxClass msg;
                msg.Process("No more triggers available.", 23, 0, 0, 0);
            }
            g_hidPage.Clear(0);
            Flag_To_Redraw(true);
            Render();
            break;
        case 3:
            if (c.m_TriggerType != nullptr) {
                ObjectClass::Detach_This_From_All(c.m_TriggerType->As_Target(), true);
                if (c.m_TriggerType) {
                    delete c.m_TriggerType;
                }
                c.m_TriggerType = nullptr;
                c.m_UnsavedChanges = true;
            }
            break;
    }
    if (c.m_TriggerType != nullptr /*&& !(c.m_TriggerType->Attaches_To() & (AttachType_2 | AttachType_1))*/) {
        c.m_TriggerType = nullptr;
    }
}
/**
 * @brief
 *
 * @address 0x0051F9A0 (beta)
 */
void MapEditClass::Main_Menu()
{
    char *menu[9];
    menu[0] = "New Scenario";
    menu[1] = "Load Scenario";
    menu[2] = "Save Scenario";
    menu[3] = "Size Map";
    menu[4] = "Add Game Object";
    menu[5] = "Scenario Options";
    menu[6] = "AI Options";
    menu[7] = "Play Scenario";
    menu[8] = 0;
    Override_Mouse_Shape(MOUSE_POINTER);
    bool process = true;
    while (process) {
        Call_Back();
        g_mouse->Hide_Mouse();
        int choice = Do_Menu(menu, 1);
        g_mouse->Show_Mouse();

        if (UnknownKey == 0x101B || UnknownKey == 0x1001 || UnknownKey == 0x1002) {
            break;
        }

        switch (choice) {
            case 0: // done
                if (c.m_UnsavedChanges) {
                    MessageBoxClass msg;
                    int state = msg.Process("Save Changes?", 0xD, 0xE, 0, 0);
                    g_hidPage.Clear(0);
                    Flag_To_Redraw(true);
                    Render();
                    if (state == 0 && !Save_Scenario()) {
                        c.m_UnsavedChanges = false;
                    }
                }
                if (!New_Scenario()) {
                    Scen.Set_Carry_Over_Money_Amount(0);
                    c.m_UnsavedChanges = true;
                }
                process = false;
                break;
            case 1: // done
                if (c.m_UnsavedChanges) {
                    MessageBoxClass msg;
                    int state = msg.Process("Save Changes?", 0xD, 0xE, 0, 0);
                    g_hidPage.Clear(0);
                    Flag_To_Redraw(true);
                    Render();
                    if (state == 0 && !Save_Scenario()) {
                        c.m_UnsavedChanges = false;
                    }
                }
                if (!Load_Scenario()) {
                    Scen.Set_Carry_Over_Money_Amount(0);
                    c.m_UnsavedChanges = true;
                }
                process = false;
                break;
            case 2: // done
                if (!Save_Scenario()) {
                    c.m_UnsavedChanges = false;
                }
                process = false;
                break;
            case 3: // done
                if (!Size_Map(MapCellX, MapCellY, MapCellWidth, MapCellHeight)) {
                    c.m_UnsavedChanges = true;
                }
                process = false;
                break;
            case 4: // done
                if (!Placement_Dialog()) {
                    Start_Placement();
                }
                process = false;
                break;
            case 5: // done
                if (!Scenario_Dialog()) {
                    c.m_UnsavedChanges = true;
                }
                process = false;
                break;
            case 6: // done
                AI_Menu();
                process = false;
                break;
            case 7: // done
                if (c.m_UnsavedChanges) {
                    MessageBoxClass msg;
                    int state = msg.Process("Save Changes?", 0xD, 0xE, 0x13, 0);

                    g_hidPage.Clear(0);
                    Flag_To_Redraw(true);
                    Render();

                    if (state == 2) {
                        break;
                    }

                    if (state == 0 && !Save_Scenario()) {
                        c.m_UnsavedChanges = false;
                    }
                }
                c.m_UnsavedChanges = false;
                g_InMapEditor = false;
                Start_Scenario(Scen.Scenario_Name(), true);
                process = false;
                break;
            default:
                break;
        }
    }
    g_hidPage.Clear(0);
    Flag_To_Redraw(true);
    Render();
}

/**
 * @brief
 *
 * @address 0x0051F8B4 (beta)
 */
BOOL MapEditClass::Mouse_Moved()
{
    static int old_mx;
    static int old_my;
    static cell_t old_zonecell;

    if (g_mouse->Get_Mouse_X() == old_mx) {
        if (g_mouse->Get_Mouse_Y() == old_my) {
            return false;
        }
    }
    const ObjectTypeClass *objtype = nullptr;
    bool moved = false;
    if (PendingObjectTypePtr != nullptr) {
        objtype = PendingObjectTypePtr;
    } else {
        if (c.m_GrabbedObject != nullptr) {
            objtype = &c.m_GrabbedObject->Class_Of();
        }
    }

    //crash fix, added nullptr check, beta crashes here
    if (objtype != nullptr && objtype->What_Am_I() == RTTI_INFANTRYTYPE) {
        moved = true;
    } else {
        moved = old_zonecell != DisplayCursorStart;
    }
    old_mx = g_mouse->Get_Mouse_X();
    old_my = g_mouse->Get_Mouse_Y();
    old_zonecell = DisplayCursorStart;
    return moved;
}

/**
 * @brief
 *
 * @address 0x (beta)
 */
BOOL MapEditClass::Move_Grabbed_Object()
{
    return 0;
}

/**
 * @brief
 *
 * @address 0x00520C64 (beta)
 */
void MapEditClass::Grab_Object()
{
    if (CurrentObjects.Count() > 0) {
        ObjectClass *obj = CurrentObjects.Fetch_Head();
        c.m_GrabbedObject = obj;
        c.m_GrabbedObjectCell = obj->Get_Cell() - DisplayCursorStart;
    }
}

/**
 * @brief
 *
 * @address 0x (beta)
 */
void MapEditClass::Place_Home() {}

/**
 * @brief
 *
 * @address 0x (beta)
 */
BOOL MapEditClass::Place_Object()
{
    return 0;
}

/**
 * @brief
 *
 * @address 0x (beta)
 */
void MapEditClass::Place_Next() {}

/**
 * @brief
 *
 * @address 0x (beta)
 */
void MapEditClass::Place_Next_Category() {}

/**
 * @brief
 *
 * @address 0x (beta)
 */
void MapEditClass::Place_Prev() {}

/**
 * @brief
 *
 * @address 0x (beta)
 */
void MapEditClass::Place_Prev_Category() {}

/**
 * @brief
 *
 * @address 0x00523394 (beta)
 */
void MapEditClass::Place_Trigger()
{
    int mx = g_keyboard->Get_MouseQX();
    int my = g_keyboard->Get_MouseQY();
    cell_t clickcalc = Click_Cell_Calc(mx, my);
    ObjectClass *cellobj = Cell_Object(clickcalc, (mx - TacOffsetX) % 24, (my - TacOffsetY) % 24);
    // TODO
    AttachType attach = ATTACH_NONE; // c.m_TriggerType->Attaches_To();
    if (cellobj != nullptr && attach & ATTACH_2 && c.m_TriggerType != nullptr) {
        // TODO
        TriggerClass *trigger = 0; // Find_Or_Make(c.m_TriggerType);
        if (trigger != nullptr) {
            cellobj->Set_Attached_Trigger(trigger);
        }
    } else if (attach & ATTACH_1 && c.m_TriggerType != nullptr) {
        // TODO
        TriggerClass *trigger = 0; // Find_Or_Make(c.m_TriggerType);
        if (trigger != nullptr) {
            // not sure what cell its accessing here...
            // Array[0].CellTag = trigger;
        }
    }
    g_hidPage.Clear(0);
    Flag_To_Redraw(true);
}

/**
 * Selects next closest object in viewport and adjusts tactical's position accordingly
 *
 * @address 005206F4 (beta)
 */
void MapEditClass::Select_Next()
{
    // crash fix, original crashes if you select a next object with while nothing is selected
    if (CurrentObjects.Count() == 0 || CurrentObjects.Fetch_Head() == nullptr) {
        return;
    }

    // if we have a next object delect all current and select next object
    ObjectClass *optr = Next_Object(CurrentObjects.Fetch_Head());
    if (optr != nullptr) {
        Unselect_All();
        if (!optr->Select()){
            //we failed to select the next object, bail!
            return;
        }
    }

    // reset mouse cursor
    Set_Default_Mouse(MOUSE_POINTER);
    Override_Mouse_Shape(MOUSE_POINTER);

    // update popup controls
    Popup_Controls();

    // calculate new tactical position relative to selected object
    uint8_t coordx = Lepton_To_Cell_Coord(DisplayWidth);
    uint8_t coordy = Lepton_To_Cell_Coord(DisplayHeight);

    cell_t cell = CurrentObjects.Fetch_Head()->Get_Cell();
    uint8_t cellx = Cell_Get_X(cell);
    uint8_t celly = Cell_Get_Y(cell);

    uint8_t cellcoordx = Coord_Cell_X(DisplayPos);
    uint8_t cellcoordy = Coord_Cell_Y(DisplayPos);

    if (cellx < cellcoordx) {
        cellcoordx = cellx;
    } else if (coordx + cellcoordx <= cellx) {
        cellcoordx = cellx - coordx + 1;
    }
    if (celly < cellcoordy) {
        cellcoordy = celly;
    } else if (coordy + cellcoordy <= celly) {
        cellcoordy = celly - coordy + 1;
    }

    ++ScenarioInit;
    Set_Tactical_Position(Coord_From_Lepton_XY(Coord_Cell_To_Lepton(cellcoordx), Coord_Cell_To_Lepton(cellcoordy)));
    --ScenarioInit;

    g_hidPage.Clear(0);
    Flag_To_Redraw(true);
}

/**
 * @brief
 *
 * @address 0x00520574 (beta)
 */
int MapEditClass::Select_Object()
{
    bool selected = 0;
    int mx = g_keyboard->Get_MouseQX();
    int my = g_keyboard->Get_MouseQY();
    cell_t clickcalc = Click_Cell_Calc(mx, my);
    ObjectClass *cellobj = Cell_Object(clickcalc, (mx - TacOffsetX) % 24, (my - TacOffsetY) % 24);
    if (cellobj != nullptr) {
        if (!CurrentObjects.Count() || CurrentObjects.Count() && cellobj != CurrentObjects.Fetch_Head()) {
            Unselect_All();
            cellobj->Select();
            DEBUG_LOG("MapEditClass::Select_Object Selection made\n");
            Set_Default_Mouse(MOUSE_POINTER);
            Override_Mouse_Shape(MOUSE_POINTER);
            Popup_Controls();
        }
    } else if (CurrentObjects.Count()) {
        Unselect_All();
        Popup_Controls();
        selected = -1;
    }
    g_hidPage.Clear(0);
    Flag_To_Redraw(true);

    return selected;
}

/**
 * @brief
 *
 * @address 0x (beta)
 */
int MapEditClass::Select_Team(char *a1)
{
    return 0;
}

/**
 * @brief
 *
 * @address 0051C058 (beta)
 */
int MapEditClass::Select_Trigger()
{
    bool edit_pressed = false;
    bool new_pressed = false;
    bool delete_pressed = false;
    // TListClass<GamePtr<TriggerTypeClass>> triggerlist(0x64, 40, 25, 320, 180, 25, GameFileClass::Retrieve("ebtn-up.shp"),
    // GameFileClass::Retrieve("ebtn-dn.shp"));

    // shitty hack
    ListClass triggerlist(0x64,
        40,
        25,
        320,
        180,
        TPF_NOSHADOW | TPF_EDITOR,
        GameFileClass::Retrieve("ebtn-up.shp"),
        GameFileClass::Retrieve("ebtn-dn.shp"));

    TextButtonClass editbtn(0x65, "Edit", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 155, 221, 45, 9, 0);
    TextButtonClass newbtn(0x66, "New", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 210, 221, 45, 9, 0);
    TextButtonClass deletebtn(0x67, "Delete", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 265, 221, 45, 9, 0);
    TextButtonClass v32(0x68, 23, TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 320, 221, 45, 9, 0);

    for (int i = 0; i < g_TriggerTypes.Count(); ++i) {
        triggerlist.Add_Item((int)&g_TriggerTypes[i]);
    }
    Set_Logic_Page(g_seenBuff);
    if (c.m_TriggerType) {
        triggerlist.Set_Selected_Index((int)c.m_TriggerType);
    } else {
        triggerlist.Set_Selected_Index(0);
    }
    if (g_TriggerTypes.Count()) {
        c.m_TriggerType = (TriggerTypeClass *)triggerlist.Current_Item();
    }

    GadgetClass *activegdt = &triggerlist;
    editbtn.Add_Tail(*activegdt);
    newbtn.Add_Tail(*activegdt);
    deletebtn.Add_Tail(*activegdt);
    v32.Add_Tail(*activegdt);

    bool to_draw = true;
    bool process = true;
    while (process) {
        Call_Back();
        if (to_draw) {
            Dialog_Box(0, 0, 400, 250);
            Draw_Caption("Trigger Editor", 0, 0, 400);
            activegdt->Flag_List_To_Redraw();
            activegdt->Draw_All(true);
            to_draw = false;
        }
        KeyNumType input = activegdt->Input();
        if (input != KN_NONE) {
            switch (input) {
                case GADGET_INPUT_RENAME2(0x64):
                    c.m_TriggerType = (TriggerTypeClass *)triggerlist.Current_Item();
                    break;
                case GADGET_INPUT_RENAME2(0x65):
                    if (c.m_TriggerType != nullptr) {
                        process = false;
                        edit_pressed = true;
                    }
                    break;
                case GADGET_INPUT_RENAME2(0x66):
                    process = false;
                    new_pressed = true;
                    break;
                case GADGET_INPUT_RENAME2(0x67):
                    // add a trigger pointer check?
                    process = false;
                    delete_pressed = true;
                    break;
                case KA_RETURN:
                case GADGET_INPUT_RENAME2(0x68):
                    process = false;
                    break;
            }
        }
    }

    g_hidPage.Clear(0);
    Flag_To_Redraw(true);
    Render();

    if (edit_pressed) {
        return 1;
    }
    if (new_pressed) {
        return 2;
    }
    if (delete_pressed) {
        return 3;
    }
    return 0;
}

// workaround cause we don't have the needed virtual
// crude implamentations of ::Display functions
// NOTE!, original drew cameos when possible, this isn't efficient to test the code so its omitted from this
void Draw_Object_Type(ObjectTypeClass *obj, int x, int y, WindowNumberType window, HousesType house)
{
    if (obj == nullptr) {
        return;
    }

    int xoff, yoff;

    TechnoTypeClass *ttptr = (TechnoTypeClass *)obj;
    void *image = obj->Get_Image_Data();
    switch (obj->What_Am_I()) {
        default:
            break;
        case RTTI_AIRCRAFTTYPE:
            if (image != nullptr) {
                CC_Draw_Shape(image, 5, x, y, window, SHAPE_CENTER | SHAPE_WIN_REL);
            }
            break;
        case RTTI_BUILDINGTYPE:
            g_isTheaterShape = ttptr->Is_Theater();
            if (image != nullptr) {
                CC_Draw_Shape(image, 0, x, y, window, SHAPE_CENTER | SHAPE_WIN_REL);
            }
            g_isTheaterShape = false;
            break;
        case RTTI_TEMPLATETYPE: {
            // broken broken broken broken broken broken broken broken broken broken broken broken broken broken
            TemplateTypeClass *ttptr = (TemplateTypeClass *)obj;
            int width = std::clamp(ttptr->Get_Width(), 1, 13);
            int height = std::clamp(ttptr->Get_Height(), 1, 8);

            int xval = x;
            int yval = y;
            int scale = width > 3 || height > 3;
            int theight = 24 * height;
            int twidth = 24 * width;
            if (scale) {
                // 4 is a guess
                xval = x - (twidth / 4);
                yval = y - (theight / 4);
            } else {
                xval = x - twidth / 2;
                yval = y - theight / 2;
            }
            xoff = WindowList[window].X + xval;
            yoff = WindowList[window].Y + yval;
            void *data = obj->Get_Image_Data();
            //uint8_t *icon = Get_Icon_Set_Map(data);
            
            for (int i = 0; i < height * width; ++i) {
                //if (icon[i] != 255) {//dunno...... broken
            ///*
                    g_logicPage->Draw_Stamp(
                        data, i, 0, 0, 0, WindowList[0].X, WindowList[0].Y, WindowList[0].W, WindowList[0].H);
                    if (scale) {
                        g_hidPage.Scale(
                            *g_logicPage, 0, 0, 12 * (i % width) + xoff, yoff + 12 * (i / width), 24, 24, 12, 12);
                    } else {
                        g_hidPage.Blit(*g_logicPage, 0, 0, xoff + 24 * (i % width), yoff + 24 * (i / width), 24, 24);
                    }
                //}
            }
            //*/
            //// edwin code,  broken broken broken broken broken broken
            //        Buffer_Draw_Stamp(*g_logicPage, (IconControlType *)data, i, 24 * (i % width) + xoff, yoff + 24 * (i / width), 0);
            //    }
            //}
            break;
        }
        case RTTI_INFANTRYTYPE:
            if (owner != HOUSES_NONE) {
                if (image != nullptr) {
                    CC_Draw_Shape(image, 2, x, y, window, SHAPE_CENTER | SHAPE_WIN_REL);
                }
            }
            break;
        case RTTI_OVERLAYTYPE: {
            OverlayTypeClass *otptr = (OverlayTypeClass *)obj;
            if (image != nullptr) {
                int frame = 0;
                if (otptr->Is_Ore()) {
                    frame = 7;
                }

                OverlayType type = otptr->Get_Type();
                if (type == OVERLAY_GEM_01 || type == OVERLAY_GEM_02 || type == OVERLAY_GEM_03 || type == OVERLAY_GEM_04) {
                    frame = 2;
                }

                g_isTheaterShape = otptr->Is_Theater();
                CC_Draw_Shape(image, frame, x, y, window, SHAPE_CENTER | SHAPE_WIN_REL);
                g_isTheaterShape = false;
            }
            break;
        }
        case RTTI_SMUDGETYPE: {
            //draws off center(bug in this), draws multi cell smudges glitchy(original code bug)
            g_isTheaterShape = true;
            SmudgeTypeClass *stptr = (SmudgeTypeClass *)obj;
            xoff = WindowList[window].X + x;
            yoff = WindowList[window].Y + y;
            if (image != nullptr) {
                for (int i = 0; i < stptr->Get_Width(); ++i) {
                    for (int j = 0; j < stptr->Get_Height(); ++j) {
                        CC_Draw_Shape(image,
                            i + j * stptr->Get_Width(),
                            xoff + 24 * i,
                            yoff + 24 * j,
                            WINDOW_TACTICAL,
                            SHAPE_VIEWPORT_REL);
                    }
                }
            }
            g_isTheaterShape = false;

            break;
        }
        case RTTI_TERRAINTYPE:
            g_isTheaterShape = true;
            if (image != nullptr) {
                CC_Draw_Shape(image, 0, x, y, window, SHAPE_CENTER | SHAPE_WIN_REL);
            }
            g_isTheaterShape = false;
            break;
        case RTTI_UNITTYPE:
            if (image != nullptr) {
                int frame = ttptr->Get_ROTCount() / 6;
                CC_Draw_Shape(image, frame, x, y, window, SHAPE_CENTER | SHAPE_WIN_REL);
            }
            break;
        case RTTI_VESSELTYPE:
            if (image != nullptr) {
                int frame = ttptr->Get_ROTCount() / 6;
                CC_Draw_Shape(image, frame, x, y, window, SHAPE_CENTER | SHAPE_WIN_REL);
            }
            break;
    }
}

/**
 * @brief
 *
 * @address 0x (beta)
 */
int MapEditClass::Placement_Dialog()
{
    ListClass houselist(0x65,
        192,
        0x19,
        0x3C,
        0x80,
        TPF_NOSHADOW | TPF_EDITOR,
        GameFileClass::Retrieve("ebtn-up.shp"),
        GameFileClass::Retrieve("ebtn-dn.shp"));

    // fill up house list
    for (HousesType i = HOUSES_FIRST; i < HOUSES_COUNT; ++i) {
        houselist.Add_Item(HouseTypeClass::As_Reference(i).Get_Name());
    }

    TextButtonClass arrow1btn(0x66, 26, TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 0x74, 0x89, 0x2D, 9);
    TextButtonClass arrow2btn(0x67, 25, TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 0x3D, 0x89, 0x2D, 9);
    TextButtonClass okbtn(0x68, "OK", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 0x3D, 0x95, 0x2D, 9);
    TextButtonClass cancelbtn(0x69, "Cancel", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 0x74, 0x95, 0x2D, 9);
    TextButtonClass templatebtn(0x6A, "Template", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 0x125, 0x19, 0x46, 9);
    TextButtonClass overlaybtn(0x6B, "Overlay", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 0x125, 0x22, 0x46, 9);
    TextButtonClass smudgebtn(0x6C, "Smudge", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 0x125, 0x2B, 0x46, 9);
    TextButtonClass terrainbtn(0x6D, "Terrain", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 0x125, 0x34, 0x46, 9);
    TextButtonClass unitbtn(0x6E, "Unit", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 0x125, 0x3D, 0x46, 9);
    TextButtonClass infantrybtn(0x6F, "Infantry", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 0x125, 0x46, 0x46, 9);
    TextButtonClass shipsbtn(0x70, "Ships", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 0x125, 0x4F, 0x46, 9);
    TextButtonClass buildingbtn(0x71, "Building", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 0x125, 0x58, 0x46, 9);
    TextButtonClass aircraftbtn(0x72, "Aircraft", TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 0x125, 0x61, 0x46, 9);

    // fill placeable object lists
    Clear_List();
    TemplateTypeClass::Prep_For_Add();
    OverlayTypeClass::Prep_For_Add();
    SmudgeTypeClass::Prep_For_Add();
    TerrainTypeClass::Prep_For_Add();
    UnitTypeClass::Prep_For_Add();
    InfantryTypeClass::Prep_For_Add();
    VesselTypeClass::Prep_For_Add();
    BuildingTypeClass::Prep_For_Add();
    AircraftTypeClass::Prep_For_Add();

    //missing messed up code

    // prep to add totally failed, bail
    if (c.m_TotalObjectCount == 0) {
        return -1;
    }

    Set_Logic_Page(g_seenBuff);
    if (c.m_TotalObjectCount <= c.m_CurrentEntry){
        c.m_CurrentEntry = 0;
    }
    ObjectTypeClass *current_object = c.m_ObjectTypeList[c.m_CurrentEntry];
    GadgetClass *activegdt = &arrow1btn;
    houselist.Add_Tail(*activegdt);
    arrow2btn.Add_Tail(*activegdt);
    okbtn.Add_Tail(*activegdt);
    cancelbtn.Add_Tail(*activegdt);
    templatebtn.Add_Tail(*activegdt);
    overlaybtn.Add_Tail(*activegdt);
    smudgebtn.Add_Tail(*activegdt);
    terrainbtn.Add_Tail(*activegdt);
    unitbtn.Add_Tail(*activegdt);
    infantrybtn.Add_Tail(*activegdt);
    shipsbtn.Add_Tail(*activegdt);
    buildingbtn.Add_Tail(*activegdt);
    aircraftbtn.Add_Tail(*activegdt);

    c.m_CurrentOwner = (HousesType)houselist.Current_Index();

    bool process = true;
    bool to_draw = true;
    bool cancelled = false;
    int category = 0;
    while (process) {
        Call_Back();
        if (to_draw) {
            g_mouse->Hide_Mouse();
            Dialog_Box(0, 0, 0x190, 0xB4);
            Draw_Caption(0x17A, 0, 0, 0x190);
            WindowList[WINDOW_5].X = 0x23;
            WindowList[WINDOW_5].Y = 0x19;
            WindowList[WINDOW_5].W = 0x98;
            WindowList[WINDOW_5].H = 0x69;
            Change_Window(WINDOW_5);
            Draw_Box(0x23, 0x19, 0x98, 0x69, BOX_STYLE_0, false);
            // current_object->Display(WinW / 2, WinH / 2, 5, c.m_CurrentOwner); //not possible
            // so replacing with this
            Draw_Object_Type(current_object, WinW / 2, WinH / 2, WINDOW_5, c.m_CurrentOwner);
            g_logicPage->Fill_Rect(322, 108, 358, 138, 12);
            //int16_t *occupy = current_object->Occupy_List();
            // loop to draw occupy

            // draw grid over occupy list
            for (int j = 0; j <= 10; ++j) {
                for (int k = 0; k <= 10; ++k) {
                    g_logicPage->Draw_Line(3 * k + 328, 108, 3 * k + 328, 138, GadgetClass::Get_Color_Scheme()->WindowPalette[0]);
                }
                g_logicPage->Draw_Line(328, 3 * j + 108, 358, 3 * j + 108, GadgetClass::Get_Color_Scheme()->WindowPalette[0]);
            }

            // draw name
            Fancy_Text_Print((int)current_object->Full_Name(), 0x6F, 0x20, GadgetClass::Get_Color_Scheme(), COLOR_TBLACK, TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR);
            category = 0;
            // this loop is weird......
            for ( int l = 0; l < 9; ++l ) {
                category += c.m_CategoryCounts[l];
                if ( category > c.m_CurrentEntry ) {
                    break;
                }
            }
            // toggle all buttons to off
            templatebtn.Turn_Off();
            overlaybtn.Turn_Off();
            smudgebtn.Turn_Off();
            terrainbtn.Turn_Off();
            unitbtn.Turn_Off();
            infantrybtn.Turn_Off();
            shipsbtn.Turn_Off();
            aircraftbtn.Turn_Off();
            buildingbtn.Turn_Off();

            // toggle button of the current cat
            switch (category + 0x6A) {
                case 0x6A:
                    templatebtn.Turn_On();
                    break;
                case 0x6B:
                    overlaybtn.Turn_On();
                    break;
                case 0x6C:
                    smudgebtn.Turn_On();
                    break;
                case 0x6D:
                    terrainbtn.Turn_On();
                    break;
                case 0x6E:
                    unitbtn.Turn_On();
                    break;
                case 0x6F:
                    infantrybtn.Turn_On();
                    break;
                case 0x70:
                    shipsbtn.Turn_On();
                    break;
                case 0x71:
                    buildingbtn.Turn_On();
                    break;
                case 0x72:
                    aircraftbtn.Turn_On();
                    break;
                default:
                    break;
            }
            activegdt->Draw_All(true);
            g_mouse->Show_Mouse();
            to_draw = false;
        }
        KeyNumType input = activegdt->Input();
        if (input != KN_NONE) {
            switch (input) {
                case GADGET_INPUT_RENAME2(0x65): // done
                    c.m_CurrentOwner = (HousesType)houselist.Current_Index();
                    to_draw = true;
                    break;
                case KA_RETURN: // done
                case GADGET_INPUT_RENAME2(0x68): // done
                    process = false;
                    cancelled = false;
                    break;
                case KN_ESC: // done
                case GADGET_INPUT_RENAME2(0x69): // done
                    process = false;
                    cancelled = true;
                    break;

                //test hack
                case GADGET_INPUT_RENAME2(0x66):
                case KN_LEFT:
                    c.m_CurrentEntry--;
                    //if (c.m_CurrentEntry >= 0){
                    //    c.m_CurrentEntry = c.m_TotalObjectCount - 1;
                    //}
                    current_object = c.m_ObjectTypeList[c.m_CurrentEntry];
                    to_draw = 1; // maybe?
                    break;
                case GADGET_INPUT_RENAME2(0x67):
                case KN_RIGHT:
                    c.m_CurrentEntry++;
                    //f (c.m_CurrentEntry <= c.m_TotalObjectCount - 1){
                    //   c.m_CurrentEntry = 0;
                    //
                    current_object = c.m_ObjectTypeList[c.m_CurrentEntry];
                    to_draw = 1; // maybe?
                    break;
                /////////////
                case KN_PGUP:
                    if (--category < 0) {
                        category = 8;
                    }
                    c.m_CurrentEntry = c.m_CurrentIndex[category];
                    current_object = c.m_ObjectTypeList[c.m_CurrentEntry];
                    to_draw = 1; // maybe?
                    break;
                case KN_PGDN:
                    if (++category == 9) {
                        category = 0;
                    }
                    c.m_CurrentEntry = c.m_CurrentIndex[category];
                    current_object = c.m_ObjectTypeList[c.m_CurrentEntry];
                    to_draw = 1; // maybe?
                    break;
                case KN_HOME:
                    // c.m_CurrentEntry = 0;
                    // current_object = c.m_ObjectTypeList[c.m_CurrentEntry];
                    break;
            }
        }
    }

    g_hidPage.Clear(0);
    Flag_To_Redraw(true);
    Render();
    if (cancelled) {
        return -1;
    }
    return 0;
}

/**
 * @brief
 *
 * @address 0x00523538 (beta)
 */
void MapEditClass::Start_Base_Building()
{
    Build_Base_To(100);
    c.m_BaseBuilding = true;
    Start_Placement();
    g_hidPage.Clear();
    Flag_To_Redraw(true);
}

/**
 * @brief
 *
 * @address 0x (beta)
 */
void MapEditClass::Build_Base_To(int a1) {}

/**
 * @brief
 *
 * @address 0x00523608 (beta)
 */
void MapEditClass::Cancel_Base_Building()
{
    Build_Base_To(c.m_BasePercent);
    Cancel_Placement();
    c.m_BaseBuilding = false;
    g_hidPage.Clear();
    Flag_To_Redraw(true);
}

/**
 * @brief
 *
 * @address 0x (beta)
 */
void MapEditClass::Start_Placement()
{
    Clear_List();
    TemplateTypeClass::Prep_For_Add();
    OverlayTypeClass::Prep_For_Add();
    SmudgeTypeClass::Prep_For_Add();
    TerrainTypeClass::Prep_For_Add();
    UnitTypeClass::Prep_For_Add();
    InfantryTypeClass::Prep_For_Add();
    VesselTypeClass::Prep_For_Add();
    BuildingTypeClass::Prep_For_Add();
    AircraftTypeClass::Prep_For_Add();

    HousesType owner;
    // TODO
    if (c.m_BaseBuilding) {
        // TODO
    } else {
        if (c.m_CurrentEntry >= c.m_TotalObjectCount) {
            c.m_CurrentEntry = c.m_TotalObjectCount - 1;
        }
        PendingObjectTypePtr = c.m_ObjectTypeList[c.m_CurrentEntry];
        owner = c.m_CurrentOwner;
    }
    PendingObjectOwner = owner;
    // TODO
    if (PendingObjectPtr == nullptr) {
        MessageBoxClass msg;
        msg.Process("No more objects of this type available.", 23, 0, 0, 0);
        g_hidPage.Clear(0);
        Flag_To_Redraw(true);
        Render();
        PendingObjectTypePtr = nullptr;
        if (c.m_BaseBuilding) {
            Cancel_Base_Building();
        }
    } else {
        Set_Cursor_Pos(-1);
        Set_Cursor_Shape((int16_t *)PendingObjectTypePtr->Occupy_List(false));
    }
}

/**
 * @brief
 *
 * @address 0x00522810 (beta)
 */
void MapEditClass::Cancel_Placement()
{
    if (PendingObjectPtr != nullptr) {
        delete PendingObjectPtr;
    }
    PendingObjectTypePtr = nullptr;
    PendingObjectPtr = nullptr;
    PendingObjectOwner = HOUSES_NONE;
    Set_Cursor_Shape(nullptr);
    g_hidPage.Clear();
    Flag_To_Redraw(true);
    Render();
}

/**
 * @brief
 *
 * @address 0x00520E90 (beta)
 */
BOOL MapEditClass::Change_House(HousesType house)
{
    if (!CurrentObjects.Count()) {
        return false;
    }
    if (!CurrentObjects.Fetch_Head()->Is_Techno()) {
        return false;
    }
    // TODO add base
    if (!CurrentObjects.Fetch_Head()->What_Am_I() == RTTI_BUILDING /*&& BaseClass::Is_Node(&Base)*/) {
        return false;
    }
    HouseClass *hptr = HouseClass::As_Pointer(house);
    if (hptr != nullptr) {
        TechnoClass *optr = (TechnoClass *)CurrentObjects.Fetch_Head();
        optr->Set_Owner_House(hptr);
        optr->Set_Player_Owned();
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @address 0x0051DBF0 (beta)
 */
void MapEditClass::Clear_List()
{
    c.m_TotalObjectCount = 0;
    // shouldn't this clear index too?
    for (int i = 0; i < 9; ++i) {
        c.m_CategoryCounts[i] = 0;
    }
}

/**
 * @brief
 *
 * @address 0x00520040 (beta)
 */
HousesType MapEditClass::Cycle_House(HousesType house, ObjectTypeClass *objecttype)
{
    if (house == HOUSES_MULTI_LAST) {
        house = HOUSES_FIRST;
    } else {
        house++;
    }
    return house;
}

/**
 * @brief
 *
 * @address 0x (beta)
 */
void MapEditClass::Draw_Member(TechnoTypeClass *objecttype, int a2, int a3, HousesType house) {}

/**
 * @brief
 *
 * @address 0x005200F8 (beta)
 */
BOOL MapEditClass::Get_Waypoint_Name(char *a1)
{
    TextButtonClass okbtn(0x64, 0x17, TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 0x75, 0x70, 0x28);
    TextButtonClass cancelbtn(0x65, 0x13, TPF_CENTER | TPF_NOSHADOW | TPF_EDITOR, 0xA3, 0x70, 0x28);
    EditClass edit(0x66, a1, 3, TPF_NOSHADOW | TPF_EDITOR, 0x75, 0x5C, 0x56, -1, EDIT_SYMS | EDIT_NUMS | EDIT_TEXT);

    Set_Logic_Page(g_seenBuff);

    GadgetClass *activegdgt = &okbtn;
    cancelbtn.Add_Tail(*activegdgt);
    edit.Add_Tail(*activegdgt);

    edit.Set_Focus();

    bool cancelled = false;
    bool set_focus = true;
    bool to_draw = true;
    bool process = true;
    while (process) {
        if (Session.Game_To_Play() == GAME_CAMPAIGN) {
            Call_Back();
        } else if (Main_Loop()) {
            process = false;
            cancelled = true;
        }
        if (to_draw) {
            g_mouse->Hide_Mouse();
            Dialog_Box(110, 72, 100, 56);
            activegdgt->Flag_List_To_Redraw();
            g_mouse->Show_Mouse();
            to_draw = false;
        }

        if (set_focus) {
            set_focus = false;
            edit.Set_Focus();
            edit.Flag_To_Redraw();
        }
        KeyNumType input = activegdgt->Input();

        switch (input) {
            case KN_KEYPAD_RETURN:
            case GADGET_INPUT_RENAME2(0x64):
                process = false;
                cancelled = false;
                break;
            case KA_ESC:
            case GADGET_INPUT_RENAME2(0x65):
                cancelled = true;
                process = false;
                break;
            default:
                break;
        }
    }
    g_mouse->Hide_Mouse();
    g_seenBuff.Clear(0);
    GamePalette.Set(nullptr, 0);
    g_mouse->Show_Mouse();
    Flag_To_Redraw(true);
    if (cancelled) {
        return false;
    }
    return true;
}

/**
 * @brief
 *
 * @address 0x00523314 (beta)
 */
void MapEditClass::Set_House_Buttons(HousesType house, GadgetClass *a2, int a3)
{
    // or does it set house?
    c.m_HouseTypeList->Set_Selected_Index(a3);
}

/**
 * @brief
 *
 * @address 0x00520018 (beta)
 */
BOOL MapEditClass::Verify_House(HousesType house, ObjectTypeClass *objecttype)
{
    return ((1 << house) & objecttype->Get_Ownable()) != 0;
}

/**
 * @brief
 *
 * @address 0x00523330 (beta)
 */
void MapEditClass::Start_Trigger_Placement()
{
    Set_Default_Mouse(MOUSE_MOVE);
    Override_Mouse_Shape(MOUSE_MOVE);
}

/**
 * @brief
 *
 * @address 0x00523360 (beta)
 */
void MapEditClass::Stop_Trigger_Placement()
{
    c.m_TriggerType = nullptr;
    Set_Default_Mouse(MOUSE_POINTER);
    Override_Mouse_Shape(MOUSE_POINTER);
}

/**
 * @brief
 *
 * @address 0x (beta)
 */
BOOL MapEditClass::Team_Members(HousesType house)
{
    return false;
}

/**
 * @brief
 *
 * @address 0x0052326C (beta)
 */
void MapEditClass::Toggle_House()
{
    if (!c.m_BaseBuilding) {
        if (PendingObjectPtr->Is_Techno()) {
            c.m_CurrentOwner = Cycle_House(PendingObjectPtr->Owner(), PendingObjectTypePtr);
            reinterpret_cast<TechnoClass *>(PendingObjectPtr)->Set_Owner_House(HouseClass::As_Pointer(c.m_CurrentOwner));
            PendingObjectOwner = c.m_CurrentOwner;
        }
    }
}

/**
 * @brief
 *
 * @address 0x00520468 (beta)
 */
void MapEditClass::Update_Waypoint(int wp)
{
    cell_t waypoint = Scen.Get_Waypoint(wp);
    if (waypoint != -1) {
        if (waypoint != Scen.Get_Waypoint(WAYPOINT_HOME) && waypoint != Scen.Get_Waypoint(WAYPOINT_REINF)) {
            Map[waypoint].Set_Bit16(false);
        }
        Flag_Cell(waypoint);
    }
    Scen.Set_Waypoint(wp, CellClass::CurrentSelectedCell);
    Map[CellClass::CurrentSelectedCell].Set_Bit16(true);
    c.m_UnsavedChanges = true;
    Flag_Cell(CellClass::CurrentSelectedCell);
}
