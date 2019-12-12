/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
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
#pragma once

#ifndef BUILDING_H
#define BUILDING_H

#include "always.h"
#include "buildingtype.h"
#include "factory.h"
#include "techno.h"

class BuildingClass : public TechnoClass
{
public:
    BuildingClass(BuildingType type, HousesType house);
    BuildingClass(const BuildingClass &that);
    BuildingClass(const NoInitClass &noinit);
    virtual ~BuildingClass();

    void *operator new(size_t size);
    void *operator new(size_t size, void *ptr) { return ptr; }
    void operator delete(void *ptr);
#ifndef COMPILER_WATCOM // Watcom doesn't like this, MSVC/GCC does.
    void operator delete(void *ptr, void *place) {}
#endif

    virtual coord_t Center_Coord() const final;
    //Target_Coord
    //AI

    // ObjectClass
    virtual void *Get_Image_Data() const final;
    //What_Action
    //What_Action
    virtual const BuildingTypeClass &Class_Of() const final { return *m_Class; }
    //Can_Demolish
    //Can_Player_Fire
    virtual BOOL Can_Player_Move() const final;
    virtual coord_t Docking_Coord() const final;
    //Sort_Y
    virtual coord_t Exit_Coord() const final;
    virtual BOOL Limbo() final;
    //Unlimbo
    virtual void Detach(target_t target, int a2) final;
    //Detach_All
    //Exit_Object
    //Overlap_List
    virtual void Draw_It(int x, int y, WindowNumberType window) final;
    //Mark
    virtual void Active_Click_With(ActionType action, ObjectClass *object) final;
    virtual void Active_Click_With(ActionType action, cell_t cellnum) final;
    //Take_Damage

    virtual void Fire_Out() final {}
    virtual int Value() const final;
    //Receive_Message
    //Revealed
    //Repair
    //Sell_Back

    // MissionClass
    //Mission_Attack
    //Mission_Guard
    //Mission_Harvest
    //Mission_Unload
    //Mission_Construction
    //Mission_Deconstruction
    //Mission_Repair
    //Mission_Missile

 
    // TechnoClass   
    //How_Many_Survivors
    virtual DirType Turret_Facing() const final;
    //Find_Exit_Cell
    virtual DirType Fire_Direction() const final;
    //Crew_Type
    //Pip_Count
    virtual void Death_Announcement(TechnoClass *killer) const final;
    //Can_Fire
    virtual target_t Greatest_Threat(ThreatType threat) final;
    virtual void Assign_Target(target_t target) final;
    //Captured
    virtual void Enter_Idle_Mode(BOOL a1 = false) final;
    //Grand_Opening
    //Update_Buildables
    virtual uint8_t *Remap_Table() const final;
    //Toggle_Primary

    BuildingType What_Type() const { return m_Class->What_Type(); }

    int Shape_Number();
    int Power_Output();
    void Begin_Mode(BStateType state);
    int Flush_For_Placement(TechnoClass *techno, cell_t cellnum);
    

    const BuildingTypeClass::AnimControlType &Fetch_Anim_Control() const { return m_Class->Fetch_Anim(m_CurrentState); }

#ifdef GAME_DLL
    friend void Setup_Hooks();

public:
    void Hook_Death_Announcement(TechnoClass *killer) { BuildingClass::Death_Announcement(killer); }
    int Hook_Value() { return BuildingClass::Value(); }
    void *Hook_Get_Image_Data() { return BuildingClass::Get_Image_Data(); }
    void Hook_Draw_It(int x, int y, WindowNumberType window)
    {
        BuildingClass::Draw_It(x, y, window);
    }
    void Hook_Active_Click_With_Obj(ActionType action, ObjectClass *object)
    {
        BuildingClass::Active_Click_With(action, object);
    }
    void Hook_Active_Click_With_Cell(ActionType action, cell_t cellnum)
    {
        BuildingClass::Active_Click_With(action, cellnum);
    }
#endif


private:
    GamePtr<BuildingTypeClass> m_Class;
    GamePtr<FactoryClass> m_Factory;
    HousesType m_field_D5;
#ifndef CHRONOSHIFT_NO_BITFIELDS
    BOOL m_Bit1 : 1; // 1
    BOOL m_Bit2 : 1; // 2
    BOOL m_Bit4 : 1; // 4
    BOOL m_Bit8 : 1; // 8
    BOOL m_Bit16 : 1; // 16
    BOOL m_Bit32 : 1; // 32
#else
    bool m_Bit1;
    bool m_Bit2;
    bool m_Bit4;
    bool m_Bit8;
    bool m_Bit16;
    bool m_Bit32;
#endif
    TCountDownTimerClass<FrameTimerClass> m_C4FuseTimer;
    BStateType m_CurrentState;
    BStateType m_NextState; // name is subject to change after researching
    HousesType m_LastAttackedBy; // house that last attacked this building, this is kept track for scoring purposes
    target_t m_SabotagedBy; // what sabotaged this building
    int m_field_EA;
    target_t m_AttachedAnim; // currently attached animation, seems to be only used for SPUTDOOR
    TCountDownTimerClass<FrameTimerClass> m_PlacementDelayTimer;
};

#ifdef GAME_DLL
extern TFixedIHeapClass<BuildingClass> &g_Buildings;
#else
extern TFixedIHeapClass<BuildingClass> g_Buildings;
#endif

#endif // BUILDING_H
