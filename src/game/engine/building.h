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

enum CheckPointType {
    CHECKPOINT_0,
    CHECKPOINT_1,
    CHECKPOINT_2,
    CHECKPOINT_COUNT,
    CHECKPOINT_NONE = -1,
    CHECKPOINT_FIRST = CHECKPOINT_0,
};

class BuildingClass : public TechnoClass
{
public:
    BuildingClass(BuildingType type, HousesType house);
    BuildingClass(const BuildingClass &that);
    BuildingClass(const NoInitClass &noinit);
    virtual ~BuildingClass();

<<<<<<< HEAD
    // AbstractClass
    virtual coord_t Center_Coord() const final;
    virtual coord_t Target_Coord() const final;
    virtual MoveType Can_Enter_Cell(cell_t cellnum, FacingType facing = FACING_NONE) const final;
    virtual void AI() final;

    // ObjectClass
    virtual void *Get_Image_Data() const final;
    virtual ActionType What_Action(ObjectClass *object) const final;
    virtual ActionType What_Action(cell_t cellnum) const final;
    virtual const BuildingTypeClass &Class_Of() const final { return *m_Class; }
    virtual BOOL Can_Demolish() const final;
    virtual BOOL Can_Player_Move() const final;
    virtual coord_t Docking_Coord() const final;
    virtual coord_t Sort_Y() const final;
    virtual coord_t Exit_Coord() const final;
    virtual BOOL Limbo() final;
    virtual BOOL Unlimbo(coord_t coord, DirType dir = DIR_NORTH) final;
    virtual void Detach(target_t target, int a2) final;
    virtual void Detach_All(int a1 = 1) final;
    virtual int Exit_Object(TechnoClass *object) final;
    virtual const int16_t *Overlap_List(BOOL a1 = false) const final;
    virtual void Draw_It(int x, int y, WindowNumberType window) const final;
    virtual BOOL Mark(MarkType mark) final;
    virtual void Active_Click_With(ActionType action, ObjectClass *object) final;
    virtual void Active_Click_With(ActionType action, cell_t cellnum) final;
    virtual DamageResultType Take_Damage(int &damage, int a2, WarheadType warhead, TechnoClass *object = nullptr, BOOL a5 = false) final;
    virtual void Fire_Out() final;
    virtual int Value() const final;
    virtual RadioMessageType Receive_Message(RadioClass *radio, RadioMessageType message, target_t &target) final;
    virtual BOOL Revealed(HouseClass *house) final;
    virtual void Repair(int a1 = -1) final;
    virtual void Sell_Back(int a1) final;

    // MissionClass
    virtual int Mission_Attack() final;
    virtual int Mission_Guard() final;
    virtual int Mission_Harvest() final;
    virtual int Mission_Unload() final;
    virtual int Mission_Construction() final;
    virtual int Mission_Deconstruction() final;
    virtual int Mission_Repair() final;
    virtual int Mission_Missile() final;

    // TechnoClass
    virtual int How_Many_Survivors() const final;
    virtual DirType Turret_Facing() const final;
    virtual cell_t Find_Exit_Cell(TechnoClass *object) const final;
    virtual DirType Fire_Direction() const final;
    virtual InfantryType Crew_Type() const final;
    virtual int Pip_Count() const final;
    virtual void Death_Announcement(TechnoClass *killer) const final;
    virtual FireErrorType Can_Fire(target_t target, WeaponSlotType weapon = WEAPON_SLOT_PRIMARY) const final;
    virtual target_t Greatest_Threat(ThreatType threat) final;
    virtual void Assign_Target(target_t target) final;
    virtual BOOL Captured(HouseClass *house) final;

    // BuildingClass
    virtual void Enter_Idle_Mode(BOOL a1 = false) final;
    virtual void Grand_Opening(int a1) final;
    virtual void Update_Buildables() final;
    virtual uint8_t *Remap_Table() const final;
    virtual int Toggle_Primary() final;

    cell_t Check_Point(CheckPointType check) const; 
=======
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
    virtual void Draw_It(int x, int y, WindowNumberType window) const final;
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
>>>>>>> stuff

    BuildingType What_Type() const { return m_Class->What_Type(); }

    static void Read_INI(GameINIClass &ini);
    static void Write_INI(GameINIClass &ini);

private:
    int Shape_Number() const;
    void Drop_Debris(target_t target);
    void Begin_Mode(BStateType state);
    int Power_Output();
<<<<<<< HEAD
    int Flush_For_Placement(TechnoClass *techno, cell_t cellnum) const;
    void Update_Radar_Spied();
    void Factory_AI();
    void Rotation_AI();
    void Charging_AI();
    void Repair_AI();
    void Animation_AI();
    void Add_Gap_Effect() {} // To be implemented, inlined in AI().
    void Remove_Gap_Effect();
=======
    void Begin_Mode(BStateType state);
    int Flush_For_Placement(TechnoClass *techno, cell_t cellnum);
    

    const BuildingTypeClass::AnimControlType &Fetch_Anim_Control() const { return m_Class->Fetch_Anim(m_CurrentState); }
>>>>>>> stuff

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
    BOOL m_ToRebuild : 1; // 1
    BOOL m_ToAutoRepair : 1; // 2
    BOOL m_ToSell : 1; // 4
    BOOL m_Bit8 : 1; // 8
    BOOL m_IsRepairing : 1; // 16
    BOOL m_Bit32 : 1; // 32
    BOOL m_Bit64 : 1; // 64
    BOOL m_Bit128 : 1; // 128

    BOOL m_IsCharging : 1; // 1
    BOOL m_IsCharged : 1; // 2
    BOOL m_IsCaptured : 1; // 4
    BOOL m_IsJamming : 1; // 8
    BOOL m_IsJammed : 1; // 16
    BOOL m_Bit2_32 : 1; // 32
    BOOL m_Bit2_64 : 1; // 64
    BOOL m_Bit2_128 : 1; // 128
#else
    bool m_ToRebuild;
    bool m_ToAutoRepair;
    bool m_ToSell;
    bool m_Bit8;
    bool m_IsRepairing;
    bool m_Bit32;
    bool m_Bit64;
    bool m_Bit128;

    bool m_IsCharging;
    bool m_IsCharged;
    bool m_IsCaptured;
    bool m_IsJamming;
    bool m_IsJammed;
    bool m_Bit2_32;
    bool m_Bit2_64;
    bool m_Bit2_128;
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
