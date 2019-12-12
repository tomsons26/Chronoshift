/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
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
#include "infantry.h"
#include "building.h"
<<<<<<< HEAD
#include "iomap.h"
=======
#include "buildingtype.h"
#include "cell.h"
#include "gamefile.h"
#include "iomap.h"
#include "missioncontrol.h"
>>>>>>> stuff
#include "rules.h"
#include "scenario.h"
#include "special.h"

#ifndef GAME_DLL
TFixedIHeapClass<InfantryClass> g_Infantry;
#endif

<<<<<<< HEAD
// clang-format off
InfantryClass::DoStruct InfantryClass::MasterDoControls[DO_COUNT] = {
    { true,  false, false, 0 }, // DO_READY
    { true,  false, false, 0 }, // DO_GUARD
    { true,  false, false, 0 }, // DO_PRONE
    { true,  true,  true,  2 }, // DO_WALK
    { true,  false, false, 1 }, // DO_FIRE_UP
    { false, true,  false, 2 }, // DO_DOWN
    { true,  true,  true,  2 }, // DO_CRAWL
    { false, false, false, 3 }, // DO_UP
    { true,  false, false, 1 }, // DO_FIRE_PRONE
    { true,  false, false, 2 }, // DO_IDLE1
    { true,  false, false, 2 }, // DO_IDLE2
    { false, false, false, 2 }, // DO_DIE1
    { false, false, false, 2 }, // DO_DIE2
    { false, false, false, 2 }, // DO_DIE3
    { false, false, false, 2 }, // DO_DIE4
    { false, false, false, 2 }, // DO_DIE5
    { false, false, false, 2 }, // DO_16
    { false, false, false, 2 }, // DO_17
    { false, false, false, 2 }, // DO_18
    { false, false, false, 2 }, // DO_19
    { false, false, false, 2 }  // DO_20
};
// clang-format on
=======
static InfantryClass::DoStruct InfantryClass::MasterDoControls[DO_COUNT] = {
#ifndef CHRONOSHIFT_NO_BITFIELDS
    { 1, 0 },
    { 1, 0 },
    { 1, 0 },
    { 1 | 2 | 4, 2 },
    { 1, 1 },
    { 2, 2 },
    { 1 | 2 | 4, 2 },
    { 0, 3 },
    { 1, 1 },
    { 1, 2 },
    { 1, 2 },
    { 0, 2 },
    { 0, 2 },
    { 0, 2 },
    { 0, 2 },
    { 0, 2 },
    { 0, 2 },
    { 0, 2 },
    { 0, 2 },
    { 0, 2 },
    { 0, 2 }
#else
    { true,  false, false, 0 },
    { true,  false, false, 0 },
    { true,  false, false, 0 },
    { true,  true,  true,  2 },
    { true,  false, false, 1 },
    { true,  true,  false, 2 },
    { true,  true,  true,  2 },
    { false, false, false, 3 },
    { true,  false, false, 1 },
    { true,  false, false, 1 },
    { true,  false, false, 2 },
    { true,  false, false, 2 },
    { false, false, false, 2 },
    { false, false, false, 2 },
    { false, false, false, 2 },
    { false, false, false, 2 },
    { false, false, false, 2 },
    { false, false, false, 2 },
    { false, false, false, 2 },
    { false, false, false, 2 },
    { false, false, false, 2 },
    { false, false, false, 2 }
#endif
};
>>>>>>> stuff

InfantryClass::InfantryClass(InfantryType type, HousesType house) :
    FootClass(RTTI_INFANTRY, g_Infantry.ID(this), house),
    m_Doing(DO_NONE),
    m_StokeTimer(0),
    m_Technician(false),
    m_Stoked(false),
    m_Prone(false),
    m_Bit8(false),
    m_Bit16(false),
    m_Fear(0)
{
    m_OwnerHouse->Tracking_Add(this);

    m_Cloakable = Class_Of().Is_Cloakable();
    m_Bit2_16 = !Class_Of().Is_Two_Shooter();
    m_Health = Class_Of().Get_Strength();
    m_Ammo = Class_Of().Get_Ammo();
}

InfantryClass::InfantryClass(const InfantryClass &that) :
    FootClass(that),
    m_Class(that.m_Class),
    m_Doing(that.m_Doing),
    m_StokeTimer(that.m_StokeTimer),
    m_Technician(that.m_Technician),
    m_Stoked(that.m_Stoked),
    m_Prone(that.m_Prone),
    m_Bit8(that.m_Bit8),
    m_Bit16(that.m_Bit16),
    m_Fear(that.m_Fear)
{
}

InfantryClass::InfantryClass(const NoInitClass &noinit) :
    FootClass(noinit)
{
}

InfantryClass::~InfantryClass()
{
<<<<<<< HEAD
    if (g_GameActive) {
=======
    if (g_GameActive && m_Class != nullptr) {
>>>>>>> stuff
        if (m_Team != nullptr) {
            m_Team->Remove(this);
            m_Team = nullptr;
        }
        m_OwnerHouse->Tracking_Remove(this);
        Limbo();
    }
<<<<<<< HEAD
    m_Class = nullptr;
    m_Doing = DO_NONE;
=======
    m_Doing = DO_NONE;
    delete this;
>>>>>>> stuff
}

void *InfantryClass::operator new(size_t size)
{
    InfantryClass *this_ptr = g_Infantry.Alloc();
    if (this_ptr != nullptr) {
        this_ptr->m_IsActive = true;
    }
    return this_ptr;
}

void InfantryClass::operator delete(void *ptr)
{
    InfantryClass *this_ptr = static_cast<InfantryClass *>(ptr);
    if (this_ptr != nullptr) {
        this_ptr->m_IsActive = false;
    }
    g_Infantry.Free(this_ptr);
}

<<<<<<< HEAD
=======
MoveType InfantryClass::Can_Enter_Cell(cell_t cellnum, FacingType facing) const
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x004ED7B4, MoveType, const InfantryClass *, cell_t, FacingType);
    return func(this, cellnum, facing);
#else
    return MOVE_NONE;
#endif
}

/**
 *
 *
 */
void InfantryClass::AI()
{
    FootClass::AI();
    if (m_IsActive) {
        if (m_Bit2_1) {
            Mark(MARK_3);
        }

        if (In_Which_Layer() != LAYER_GROUND) {
            Mark(MARK_REDRAW);
        }

        if (m_Firing && m_AnimStage.Get_Delay() == 0) {
            Mark(MARK_5);
            m_Firing = false;
            Do_Action(DO_READY);
            Mark(MARK_4);
        }

        if (!Edge_Of_World_AI()) {
            if (!m_Firing && !m_IsFalling && !m_Moving && (m_Doing == DO_NONE || MasterDoControls[m_Doing].m_Bit1)) {
                if (m_Mission == MISSION_NONE && m_MissionQueue == MISSION_NONE) {
                    Enter_Idle_Mode();
                }
                Commence();
            }

            if (Class_Of().Is_Canine() && Target_Legal(m_TarCom) && Target_Get_RTTI(m_TarCom) == RTTI_CELL) {
                Assign_Target(0);
            }

            Fear_AI();
            if (!Target_Legal(m_NavCom) && !m_Prone && m_Stoked) {
                if (m_StokeTimer.Expired()) {
                    m_Stoked = false;
                    Do_Action(g_Scen.Get_Random_Value(0, 99) < 50 ? DO_16 : DO_18);
                }
            }

            Firing_AI();
            Doing_AI();
            Movement_AI();
        }
    }
}

>>>>>>> stuff
/**
 *
 *
 */
void *InfantryClass::Get_Image_Data() const
{
    if (What_Type() == INFANTRY_SPY && !m_PlayerOwned) {
        return InfantryTypeClass::As_Reference(INFANTRY_E1).Get_Image_Data();
    }

    return FootClass::Get_Image_Data();
}

<<<<<<< HEAD
=======
ActionType InfantryClass::What_Action(ObjectClass *object) const
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x004EFB58, ActionType, const InfantryClass *, ObjectClass *);
    return func(this, object);
#else
    return ACTION_NONE;
#endif
}

/**
 *
 *
 */
ActionType InfantryClass::What_Action(cell_t cellnum) const
{
    ActionType action = FootClass::What_Action(cellnum);

    if (Class_Of().Is_Canine() && action == ACTION_ATTACK) {
        action = ACTION_NONE;
    }

    if (Combat_Damage(WEAPON_SLOT_NONE) < 0 && m_OwnerHouse->Player_Has_Control() && action == ACTION_ATTACK) {
        return ACTION_NO_MOVE;
    }

    if (Class_Of().Has_C4() && action == ACTION_MOVE && !s_Special.Capture_The_Flag()) {
        if (g_Map[cellnum].Is_Bridge_Here()) {
            return ACTION_SABOTAGE;
        }
    }
    return action;
}

>>>>>>> stuff
/**
 *
 *
 */
BOOL InfantryClass::Paradrop(coord_t coord)
{
    if (!FootClass::Paradrop(coord)) {
        return false;
    }

    Assign_Mission(m_OwnerHouse->Is_Human() ? MISSION_GUARD : MISSION_HUNT);
    return true;
}

/**
 *
 *
 */
void InfantryClass::Draw_It(int x, int y, WindowNumberType window) const
{
    void *image = Get_Image_Data();
    if (image != nullptr) {
        Techno_Draw_Object(image, Shape_Number(), x - 2, y + 4, window);
        FootClass::Draw_It(x - 2, y + 4, window);
    }
}

/**
 *
 *
 */
void InfantryClass::Active_Click_With(ActionType action, cell_t cellnum)
{
    FootClass::Active_Click_With(action, cellnum);
}

/**
 *
 *
 */
void InfantryClass::Active_Click_With(ActionType action, ObjectClass *object)
{
    action = What_Action(object);
    switch (action) {
        case ACTION_CAPTURE:
        case ACTION_DAMAGE:
        case ACTION_GREPAIR:
            action = ACTION_CAPTURE;
            break;
        case ACTION_HEAL:
            action = ACTION_ATTACK;
            break;
        default:
            break;
    }

    FootClass::Active_Click_With(action, object);
}

/**
 *
 *
 */
int InfantryClass::Full_Name() const
{
    if (m_Technician) {
        return TXT_TECHNICIAN;
    }

    if (What_Type() == INFANTRY_SPY && !m_OwnerHouse->Player_Has_Control()){
        return InfantryTypeClass::As_Reference(INFANTRY_E1).Full_Name();
    }

    return Class_Of().Full_Name();
}

/**
 *
 *
 */
BOOL InfantryClass::Limbo()
{
    if (!m_InLimbo) {
        Stop_Driver();
        Clear_Occupy_Spot(m_Coord);
    }

    return FootClass::Limbo();
}

/**
 *
 *
 */
<<<<<<< HEAD
=======
BOOL InfantryClass::Unlimbo(coord_t coord, DirType dir)
{
    coord_t spot = g_Map[Coord_To_Cell(coord)].Closest_Free_Spot(coord, g_ScenarioInit != 0);
    if (spot == 0){
        return false;
    }
    if (!FootClass::Unlimbo(spot, dir)){
        return false;
    }

    m_OwnerHouse->Add_To_Built_Scan(What_Type());
    m_OwnerHouse->Add_To_Prereq_Scan(What_Type());

    if (Class_Of().Get_Sight() == 0) {
        m_PlayerAware = false;
    }
    
    Set_Occupy_Spot(spot);
    return true;
}

DamageResultType InfantryClass::Take_Damage(
    int &damage, int a2, WarheadType warhead, TechnoClass *object, BOOL a5)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x004EBD48, DamageResultType, InfantryClass *, int &, int, WarheadType, TechnoClass *, BOOL);
    return func(this, damage, a2, warhead, object, a5);
#else
    return DAMAGE_NONE;
#endif
}

void InfantryClass::Scatter(coord_t coord, int a2, BOOL a3)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x004EE6A4, void, InfantryClass *, coord_t, int, BOOL);
    func(this, coord, a2, a3);
#endif
}

void InfantryClass::Per_Cell_Process(PCPType pcp)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x004EC408, void, InfantryClass *, PCPType);
    func(this, pcp);
#endif
}

/**
 *
 *
 */
>>>>>>> stuff
void InfantryClass::Detach(target_t target, int a2)
{
    if (m_TarCom == target) {
        Mark(MARK_5);
        m_Firing = false;
        Mark(MARK_4);
    }
    FootClass::Detach(target, a2);
}

/**
 *
 *
 */
int InfantryClass::Mission_Attack()
{
    if (Class_Of().Has_C4() && As_Building(m_TarCom) != nullptr) {
        Assign_Destination(m_TarCom);
        Assign_Mission(MISSION_SABOTAGE);
        return 1;
    }

    if (Class_Of().Is_Infiltrator() && As_Building(m_TarCom) != nullptr) {
        Assign_Destination(m_TarCom);
        Assign_Mission(MISSION_CAPTURE);
        return 1;
    }
    return FootClass::Mission_Attack();
}

<<<<<<< HEAD
=======
void InfantryClass::Response_Select()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x004EF428, void, InfantryClass *);
    func(this);
#endif
}

void InfantryClass::Response_Move()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x004EF688, void, InfantryClass *);
    func(this);
#endif
}

void InfantryClass::Response_Attack()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x004EF8F0, void, InfantryClass *);
    func(this);
#endif
}

>>>>>>> stuff
/**
 *
 *
 */
FireErrorType InfantryClass::Can_Fire(target_t target, WeaponSlotType weapon) const
{
    TechnoClass *tptr = As_Techno(target);

    if (Combat_Damage() < 0 && (tptr == nullptr || tptr->Health_Ratio() >= g_Rule.Condition_Green())) {
        return FIRE_ILLEGAL;
    }

    if (m_Moving || Target_Legal(m_NavCom) && m_Doing != DO_NONE && !MasterDoControls[m_Doing].m_Bit1) {
        return FIRE_MOVING;
    }

    return FootClass::Can_Fire(target, weapon);
}

<<<<<<< HEAD
=======
target_t InfantryClass::Greatest_Threat(ThreatType threat)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x004EF020, target_t, InfantryClass *, ThreatType);
    return func(this, threat);
#else
    return 0;
#endif
}

>>>>>>> stuff
/**
 *
 *
 */
void InfantryClass::Assign_Target(target_t target)
{
    m_Paths[0] = FACING_NONE;

    ObjectClass *optr = As_Object(target);
    if (Class_Of().Is_Canine() && optr != nullptr && optr->What_Am_I() != RTTI_INFANTRY) {
        target = 0;
    }

<<<<<<< HEAD
    TechnoClass::Assign_Target(target);
=======
    Assign_Target(target);
>>>>>>> stuff

    if (!Target_Legal(m_NavCom) && Class_Of().Is_Infiltrator() && !Is_Weapon_Equipped()) {
        BuildingClass *bptr = As_Building(target);
        if (bptr != nullptr) {
            if (bptr->Class_Of().Is_Capturable()){
                Assign_Destination(target);
            }
        }

    }
}

/**
 *
 *
 */
BulletClass *InfantryClass::Fire_At(target_t target, WeaponSlotType weapon)
{
    Mark(MARK_5);
    m_Firing = false;
    Mark(MARK_4);

    BulletClass *bullet = FootClass::Fire_At(target, weapon);
    if (bullet == nullptr || m_InLimbo) {
        return nullptr;
    }

    if (Class_Of().Is_Fraidycat() && m_Ammo == 0) {
        m_Fear = 255;
        if (m_Mission == MISSION_ATTACK || m_Mission == MISSION_HUNT) {
            Assign_Mission(MISSION_GUARD);
        }
    }
    return bullet;
}

/**
 *
 *
 */
BOOL InfantryClass::Is_Ready_To_Random_Animate() const
{
    if (!FootClass::Is_Ready_To_Random_Animate()) {
        return false;
    }
    if (m_Height > 0) {
        return false;
    }
    if (m_Moving) {
        return false;
    }
    if (m_Bit1_4) {
        return false;
    }
    if (m_Firing) {
        return false;
    }
    return m_Doing == DO_GUARD || m_Doing == DO_READY;
}

/**
 *
 *
 */
<<<<<<< HEAD
void InfantryClass::Assign_Destination(target_t dest)
{
    if (m_Moving && !m_InFormation && Target_Legal(dest)) {
        if (g_Map[Get_Cell()].Is_Clear_To_Move(Class_Of().Get_Speed(), true)) {
            Stop_Driver();
        }
    }

    if (m_OwnerHouse->Is_Human() && Target_Legal(dest) && m_NavCom == dest && m_Prone) {
        if (!Class_Of().Is_Fraidycat() && !Class_Of().Is_Canine()) {
            Do_Action(DO_UP);
        }
    }

    TechnoClass *tptr = As_Techno(dest);
    if (tptr != nullptr && (m_Mission == MISSION_ENTER || m_MissionQueue == MISSION_ENTER) && !Radio_Valid()) {
        if (tptr->Radio_Valid()) {
            m_Archive = dest;
        } else if (Transmit_Message(RADIO_HELLO, tptr) == RADIO_ROGER) {
            if (Transmit_Message(RADIO_TRYING_TO_LOAD) == RADIO_ROGER) {
                return;
            }
            Transmit_Message(RADIO_OVER_AND_OUT);
        }
    } else {
        m_Paths[0] = FACING_NONE;
    }
    FootClass::Assign_Destination(dest);
=======
BOOL InfantryClass::Random_Animate()
{
    if (!Is_Ready_To_Random_Animate()) {
        return false;
    }

    m_IdleActionTimer = g_Scen.Get_Random_Value(450 * g_Rule.Idle_Action_Frequency(), 1800 * g_Rule.Idle_Action_Frequency());

    if (Class_Of().Is_Fraidycat() && !m_OwnerHouse->Is_Human() && m_Fear > 10) {
        Scatter(0, 1);
        return true;
    }

    switch (g_Scen.Get_Random_Value(0, 10)) {
        case 0:
            /*
            if (!Class_Of().Is_Canine()) {
                break;
            }
            Do_Action(DO_IDLE1);
            */
            if (Class_Of().Is_Canine()) {
                Do_Action(DO_IDLE1);
            }
            break;

        case 5:
            Do_Action(DO_IDLE1);
            break;

        case 1:
            Do_Action(DO_17);
            break;

        case 2:
            Do_Action(DO_19);
            break;

        case 3:
            Do_Action(DO_16);
            break;

        case 4:
            Do_Action(DO_18);
            break;

        case 6:
        case 9:
        case 10: {
            Mark(MARK_3);
            DirType dir = (DirType)32 * g_Scen.Get_Random_Value(FACING_FIRST, FACING_NORTH_WEST);
            m_Facing.Set_Current(dir);
            m_Facing.Set_Desired(dir);
            Mark(MARK_3);
            break;
        }
        case 7: {
            Do_Action(DO_IDLE2);
            Mark(MARK_3);
            DirType dir = (DirType)32 * g_Scen.Get_Random_Value(FACING_FIRST, FACING_NORTH_WEST);
            m_Facing.Set_Current(dir);
            m_Facing.Set_Desired(dir);
            Mark(MARK_3);

            /*
            if (m_Selected || !m_PlayerOwned) {
                break;
            }
            if (What_Type() != INFANTRY_TANYA || g_NonCriticalRandom(0, 2)) {
                break;
            }
            Sound_Effect(VOC_SHAKE_IT_BABY, m_Coord);
            */
            if (!m_Selected && m_PlayerOwned) {
                if (What_Type() == INFANTRY_TANYA && !g_NonCriticalRandom(0, 2)) {
                    Sound_Effect(VOC_SHAKE_IT_BABY, m_Coord);
                }
            }
            break;
        }
        case 8: {
            Mark(MARK_3);
            DirType dir = (DirType)32 * g_Scen.Get_Random_Value(FACING_FIRST, FACING_NORTH_WEST);
            m_Facing.Set_Current(dir);
            m_Facing.Set_Desired(dir);
            Mark(MARK_3);

            /*
            if (m_OwnerHouse->Is_Human()) {
                break;
            }
            if (!Class_Of().Is_Fraidycat()) {
                break;
            }
            Scatter(0, 1);
            */
            if (!m_OwnerHouse->Is_Human()) {
                if (Class_Of().Is_Fraidycat()) {
                    Scatter(0, 1);
                }
            }
            break;
        }
        default:
            break;
    }
    return true;
}


void InfantryClass::Assign_Destination(target_t dest)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x004ED260, void, InfantryClass *, target_t);
    func(this, dest);
#endif
}


void InfantryClass::Enter_Idle_Mode(BOOL a1)
{
    FootClass::Enter_Idle_Mode();

    MissionType mission;
    if (Target_Legal(m_TarCom)) {
        mission = MISSION_ATTACK;
        if (m_Mission == MISSION_SABOTAGE) {
            mission = MISSION_SABOTAGE;
        }
        if (m_Mission == MISSION_CAPTURE) {
            mission = MISSION_CAPTURE;
        }
        Assign_Mission(mission);
        return;
    }

    Handle_Navigation_List();

    if (Target_Legal(m_NavCom)) {
        mission = MISSION_MOVE;
        if (m_Mission == MISSION_SABOTAGE) {
            mission = MISSION_SABOTAGE;
        }
        if (m_Mission == MISSION_CAPTURE) {
            mission = MISSION_CAPTURE;
        }
        Assign_Mission(mission);
        return;
    }

    if (m_Mission != MISSION_GUARD && m_Mission != MISSION_AREA_GUARD) {
        if (!Get_Mission_Control(m_Mission).Is_Zombie() && !Get_Mission_Control(m_Mission).Is_Paralyzed()) {
            if (Class_Of().Is_Canine()) {
                if (!m_OwnerHouse->Is_Human() && m_Team == nullptr) {
                    m_Archive = Get_Cell();
                    Assign_Mission(MISSION_AREA_GUARD);
                    return;
                }
            } else {
                if (!m_OwnerHouse->Is_Human() && m_Team == nullptr) {
                    if (g_Rule.IQ_Controls().m_GuardArea <= m_OwnerHouse->Get_Current_IQ() && Is_Weapon_Equipped()) {
                        Assign_Mission(MISSION_AREA_GUARD);
                        return;
                    }
                }
            }
            Assign_Mission(MISSION_GUARD);
        }
    }
}

/**
 *
 *
 */
BOOL InfantryClass::Start_Driver(coord_t &dest)
{
    coord_t initial_dest = dest;

    DirType dir = Direction(Center_Coord(), dest);
    coord_t moved_coord = Coord_Move(dest, dir + DIR_SOUTH, 124);
    dest = g_Map[Coord_To_Cell(dest)].Closest_Free_Spot(moved_coord, false);
    if (dest == 0) {
        if (Can_Enter_Cell(Coord_To_Cell(initial_dest)) == MOVE_OK) {
            dir = Direction(Center_Coord(), dest);
            moved_coord = Coord_Move(initial_dest, dir + DIR_SOUTH, 128);
            dest = g_Map[Coord_To_Cell(initial_dest)].Closest_Free_Spot(moved_coord, true);
        }
    }
    if (dest != 0 && FootClass::Start_Driver(dest) && m_IsActive) {
        Clear_Occupy_Spot(m_Coord);
        Set_Occupy_Spot(dest);
        return true;
    }

    return false;
>>>>>>> stuff
}

/**
 *
 *
 */
BOOL InfantryClass::Stop_Driver()
{
    if (m_HeadTo) {
        Clear_Occupy_Spot(m_HeadTo);
    }
    Set_Occupy_Spot(m_Coord);

    Do_Action((!Class_Of().Is_Canine() && m_Prone) ? DO_PRONE : DO_READY);

    m_Bit8 = Can_Enter_Cell(Get_Cell()) != MOVE_OK;

    return FootClass::Stop_Driver();
}

BOOL InfantryClass::Do_Action(DoType dotype, BOOL a1)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x004EE980, BOOL, InfantryClass *, DoType, BOOL);
    return func(this, dotype, a1);
#else
    return false;
#endif
}

int InfantryClass::Shape_Number() const
{
<<<<<<< HEAD
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x004EC224, int, const InfantryClass *);
    return func(this);
#else
    return 0;
    //TODO recheck this as im not sure its correct
    /*
=======
>>>>>>> stuff
    static int HumanShape[32] = {
        0, 0, 7, 7, 7, 7, 6, 6, 6, 6, 5, 5, 5, 5, 5, 4, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0
    };

    DoType doing = m_Doing;
    if (doing == DO_NONE) {
        doing = DO_READY;
    }
    const DoInfoStruct &doinfo = Class_Of().Fetch_DoInfo(doing);

    int frames = (doinfo.m_NumFrames <= 1 ? 1 : doinfo.m_NumFrames);
    int number = m_AnimStage.Get_Stage() % frames;
    if (doinfo.m_FacingMultiplier > 0) {
        number += (doinfo.m_FacingMultiplier * HumanShape[FacingClass::s_Facing32[m_Facing.Get_Current()]]);
    }
    return number + doinfo.m_StartingFrame;
<<<<<<< HEAD
    */
#endif
=======
>>>>>>> stuff
}

/**
 *
 *
 */
void InfantryClass::Set_Occupy_Spot(cell_t cellnum, int spot_index)
{
    DEBUG_ASSERT(spot_index < CELL_SPOT_COUNT);

<<<<<<< HEAD
    CellClass &cell = g_Map[cellnum];
    cell.Set_Occupants(CellOccupantEnum(1 << spot_index));
    cell.Set_Owner(Owner());
=======
    g_Map[cellnum].Set_Occupants(CellOccupantEnum(1 << spot_index));
    g_Map[cellnum].Set_Owner(Owner());
>>>>>>> stuff
}

/**
 *
 *
 */
void InfantryClass::Clear_Occupy_Spot(cell_t cellnum, int spot_index)
{
    DEBUG_ASSERT(spot_index < CELL_SPOT_COUNT);

<<<<<<< HEAD
    CellClass &cell = g_Map[cellnum];
    cell.Clear_Occupants(CellOccupantEnum(1 << spot_index));

    // If the cell isn't occupied by any infantry the cell isn't owned anymore
    if (!cell.Check_Occupants(OCCUPANT_INFANTRY)) {
        cell.Set_Owner(HOUSES_NONE);
=======
    CellClass &cptr = g_Map[cellnum];
    cptr.Clear_Occupants(CellOccupantEnum(1 << spot_index));

    // If the cell isn't occupied by any infantry the cell isn't owned anymore
    if (!cptr.Check_Occupants(OCCUPANT_INFANTRY)) {
        cptr.Set_Owner(HOUSES_NONE);
>>>>>>> stuff
    }
}

/**
 *
 *
 */
BOOL InfantryClass::Edge_Of_World_AI()
{
    if (m_Team != nullptr && m_LockedOnMap) {
        m_Team->Set_Bit2_4(true);
    }
    if (m_Team == nullptr && m_Mission == MISSION_GUARD && !g_Map.In_Radar(Get_Cell())) {
        Stun();
        delete this;
        return true;
    }
    return false;
<<<<<<< HEAD
=======
}

/**
 *
 *
 */
void InfantryClass::Fear_AI()
{
    if (m_Fear > 0) {
        if (--m_Fear == 0 && m_Ammo == 0 && Is_Weapon_Equipped()) {
            m_Ammo = Class_Of().Get_Ammo();
        }

        if (m_Prone) {
            if (m_Fear < 10) {
                Do_Action(DO_UP);
            }
        } else {
            if (!Class_Of().Is_Canine() && m_Height == 0 && m_Fear >= 10 && !Target_Legal(m_NavCom) && !m_Moving) {
                Do_Action(DO_DOWN);
            }
        }
    }

    if (Class_Of().Is_Fraidycat() && m_Fear > 10 && !m_IsFalling && !m_Moving && !Target_Legal(m_NavCom)) {
        Scatter(0, 1);
    }
}

void InfantryClass::Firing_AI()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x004F0E58, void, InfantryClass *);
    func(this);
#endif
}

void InfantryClass::Doing_AI()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x004F1220, void, InfantryClass *);
    func(this);
#endif
}

void InfantryClass::Movement_AI()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x004F1658, void, InfantryClass *);
    func(this);
#endif
}


void InfantryClass::Init()
{
    g_Infantry.Free_All();
>>>>>>> stuff
}
