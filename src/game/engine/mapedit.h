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
#pragma once

#ifndef MAPEDIT_H
#define MAPEDIT_H

#include "always.h"
#include "gmouse.h"
#include "scenario.h"
#include "textbtn.h"
#include "list.h"
#include "txtlabel.h"
#include "dial8.h"
#include "trigauge.h"

class NoInitClass;
class TechnoTypeClass;

class MapEditClass : public GameMouseClass
{
public:
    MapEditClass();
    MapEditClass(const NoInitClass &noinit) : GameMouseClass(noinit) {}

    ~MapEditClass();

    virtual void One_Time() override;
    virtual void Init_IO() override;
    virtual void AI(KeyNumType &key, int mouse_x, int mouse_y) override;
    virtual void Draw_It(BOOL force_redraw = false) override;
    virtual void Detach(ObjectClass *object) override;
    virtual void Read_INI(GameINIClass &ini) override;
    virtual BOOL Scroll_Map(DirType dir, int &distance, BOOL redraw = true) override;
    virtual void Write_INI(GameINIClass &ini) override;

    BOOL Add_To_List(ObjectTypeClass *objecttype);
    static void Toggle_Editor_Mode(BOOL editor_mode);
    static void Spec(int);

#ifdef GAME_DLL
    MapEditClass *Hook_Ctor() { return new (this) MapEditClass(); }
#endif

private:
    BOOL New_Scenario();
    BOOL Load_Scenario();
    int Save_Scenario();
    int Scenario_Dialog();
    int Pick_Scenario(char *a1, int &a2, ScenarioPlayerEnum &a3, ScenarioDirEnum &a4, ScenarioVarEnum &a5);
    int Size_Map(int a1, int a2, int a3, int a4);
    void Popup_Controls();
    void AI_Menu();
    void Handle_Teams(char *a1);
    void Handle_Triggers();
    void Main_Menu();
    BOOL Mouse_Moved();
    BOOL Move_Grabbed_Object();
    void Grab_Object();
    void Place_Home();
    BOOL Place_Object();
    void Place_Next();
    void Place_Next_Category();
    void Place_Prev();
    void Place_Prev_Category();
    void Place_Trigger();
    void Select_Next();
    int Select_Object();
    int Select_Team(char *a1);
    int Select_Trigger();
    int Placement_Dialog();
    void Start_Base_Building();
    void Build_Base_To(int a1);
    void Cancel_Base_Building();
    void Start_Placement();
    void Cancel_Placement();
    BOOL Change_House(HousesType house);
    void Clear_List();
    HousesType Cycle_House(HousesType house, ObjectTypeClass *objecttype);
    void Draw_Member(TechnoTypeClass *objecttype, int a2, int a3, HousesType house);
    BOOL Get_Waypoint_Name(char *a1);
    void Set_House_Buttons(HousesType house, GadgetClass *a2, int a3);
    BOOL Verify_House(HousesType house, ObjectTypeClass *objecttype);
    void Start_Trigger_Placement();
    void Stop_Trigger_Placement();
    BOOL Team_Members(HousesType house);
    void Toggle_House();
    void Update_Waypoint(int a2);

protected:
    // NOTE: All members for this class must be static otherwise it
    //       it will break the existing MapIO we hook into in RA!
    struct Struct
    {
        ObjectTypeClass *m_ObjectTypeList[601]; // pointers to all placeable object types
        int m_TotalObjectCount; // total object count in all categories
        int m_CurrentEntry; // current ObjectTypeList entry
        HousesType m_CurrentOwner; // current owner of current and to be placed objects
        ObjectClass *m_GrabbedObject; // currently picked up object
        int m_GrabbedObjectCell; // cell the object was picked up from
        int m_UpdateRate; // rate at which various draw routines are executed and various data is captured
        int m_CategoryCounts[9]; // number of objects per category
        int m_CurrentIndex[9]; // currently selected object in a category
        TriggerTypeClass *m_TriggerType; // trigger currently being edited
        TeamTypeClass *m_TeamType; // team currently being edited
        BOOL m_UnsavedChanges;
        BOOL m_Bit2;
        BOOL m_BaseBuilding;
        int m_BasePercent;
        ListClass *m_HouseTypeList;
        ListClass *m_MissionList;
        TriColorGaugeClass *m_HealthBarGauge;
        Dial8Class *m_FacingDial;
        ControlClass *m_InputHandler;
        TextLabelClass *m_HealthLabel;
        TextButtonClass *m_SellableButton;
        TextButtonClass *m_RebuildButton;
        GaugeClass *m_BaseGauge;
        TextLabelClass *m_BaseLabel;
    };

    static Struct c;
    static char HealthBuf[20];
    static MissionType MapEditMissions[9];
};

#endif // MAPEDIT_H
