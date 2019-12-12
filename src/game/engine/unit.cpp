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
#include "unit.h"
#include "scenario.h"
#include "team.h"
#include "iomap.h"
#include "missioncontrol.h"
#include "rules.h"
#include "target.h"

#ifndef GAME_DLL
TFixedIHeapClass<UnitClass> g_Units;
#endif

UnitClass::UnitClass(UnitType type, HousesType house) :
    DriveClass(RTTI_UNIT, g_Units.ID(this), house)
{
}

UnitClass::UnitClass(const UnitClass &that) :
    DriveClass(that)
{
}

UnitClass::UnitClass(const NoInitClass &noinit) :
    DriveClass(noinit)
{
}

UnitClass::~UnitClass()
{
}

void *UnitClass::operator new(size_t size)
{
    UnitClass *this_ptr = g_Units.Alloc();
    if (this_ptr != nullptr) {
        this_ptr->m_IsActive = true;
    }
    return this_ptr;
}

void UnitClass::operator delete(void *ptr)
{
    UnitClass *this_ptr = static_cast<UnitClass *>(ptr);
    if (this_ptr != nullptr) {
        this_ptr->m_IsActive = false;
    }
    g_Units.Free(this_ptr);
}

MoveType UnitClass::Can_Enter_Cell(cell_t cellnum, FacingType facing) const
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0057EB30, MoveType, const UnitClass *, cell_t, FacingType);
    return func(this, cellnum, facing);
#else
    return MOVE_NONE;
#endif
}

ActionType UnitClass::What_Action(ObjectClass *object) const
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0057F3D0, ActionType, const UnitClass *, ObjectClass *);
    return func(this, object);
#else
    return ACTION_NONE;
#endif
}

/**
 *
 *
 */
ActionType UnitClass::What_Action(cell_t cellnum) const
{
    ActionType action = DriveClass::What_Action(cellnum);
    if (action == ACTION_MOVE && g_Map[cellnum].Get_Land() == LAND_ORE && Class_Of().Is_Harvester() ) {
        return ACTION_HARVEST;
    }

    if (What_Type() == UNIT_MAD_TANK) {
        if (m_IsDumping /*|| v8 & 62*/) {
            return ACTION_NO_MOVE;
        }
    }

    return action;
}

/**
 *
 *
 */
coord_t UnitClass::Sort_Y() const
{
    return m_Coord + 0x800000;
}

/**
 *
 *
 */
BOOL UnitClass::Limbo()
{
    if (!DriveClass::Limbo()) {
        return false;
    }
    if (m_FlagOwner != HOUSES_NONE) {
        HouseClass::As_Pointer(m_FlagOwner)->Flag_Attach(Get_Cell());
    }
    return true;
}

BOOL UnitClass::Unlimbo(coord_t coord, DirType dir)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0057A9A8, BOOL, UnitClass *, coord_t, DirType);
    return func(this, coord, dir);
#else
    return false;
#endif
}

const int16_t *UnitClass::Overlap_List(BOOL a1) const
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0057E8AC, const int16_t *, const UnitClass *, BOOL);
    return func(this, a1);
#else
    return nullptr;
#endif
}

void UnitClass::Draw_It(int x_pos, int y_pos, WindowNumberType window) const
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0057C9C4, void, const UnitClass *, int, int, WindowNumberType);
    func(this, x_pos, y_pos, window);
#endif
}

/**
 *
 *
 */
void UnitClass::Active_Click_With(ActionType action, ObjectClass *object)
{
    if (What_Action(object) != action) {
        action = What_Action(object);
        if (action == ACTION_CAPTURE || action == ACTION_SABOTAGE) {
            action = ACTION_ATTACK;
        }
        if (action == ACTION_ENTER) {
            action = ACTION_MOVE;
        }
    }
    if (object != this || action != ACTION_NO_MOVE) {
        if (What_Type() != UNIT_MAD_TANK || !m_IsDumping && !m_ToScatter) {
            DriveClass::Active_Click_With(action, object);
        }
    }
}

/**
 *
 *
 */
void UnitClass::Active_Click_With(ActionType action, cell_t cellnum)
{
    if (What_Type() != UNIT_MAD_TANK || !m_IsDumping && !m_ToScatter) {
        DriveClass::Active_Click_With(action, cellnum);
    }
}

DamageResultType UnitClass::Take_Damage(
    int &damage, int a2, WarheadType warhead, TechnoClass *object, BOOL a5)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0057AAE8, DamageResultType, UnitClass *, int &, int, WarheadType, TechnoClass *, BOOL);
    return func(this, damage, a2, warhead, object, a5);
#else
    return DAMAGE_NONE;
#endif
}

void UnitClass::Scatter(coord_t coord, int a2, BOOL a3)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x00581644, void, UnitClass *, coord_t, int, BOOL);
    func(this, coord, a2, a3);
#endif
}

void UnitClass::Per_Cell_Process(PCPType pcp)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0057BD10, void, UnitClass *, PCPType);
    func(this, pcp);
#endif
}

RadioMessageType UnitClass::Receive_Message(RadioClass *radio, RadioMessageType message, target_t &target)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0057A354, RadioMessageType, UnitClass *, RadioClass *, RadioMessageType, target_t &);
    return func(this, radio, message, target);
#else
    return RADIO_NONE;
#endif
}

int UnitClass::Mission_Guard()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0057FB98, int, UnitClass *);
    return func(this);
#else
    return 0;
#endif
}

int UnitClass::Mission_Guard_Area()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x005819E8, int, UnitClass *);
    return func(this);
#else
    return 0;
#endif
}

int UnitClass::Mission_Harvest()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0057E2D0, int, UnitClass *);
    return func(this);
#else
    return 0;
#endif
}

/**
 *
 *
 */
int UnitClass::Mission_Hunt()
{
    if (What_Type() == UNIT_MCV) {
        switch(m_Status) {
            case 0:
                if (Goto_Clear_Spot() && Try_To_Deploy()) {
                    m_Status = 1;
                }
                break;
            case 1:
                if (!m_Deploying) {
                    m_Status = 0;
                }
                break;
            default:
                break;
        }
        return (900 * Get_Mission_Control(m_Mission).Get_Rate()) + g_Scen.Get_Random_Value(0, 2);
    }
    return DriveClass::Mission_Hunt();
}

/**
 *
 *
 */
int UnitClass::Mission_Move()
{
    if (m_Door.Is_Closed()) {
        APC_Close_Door();
    }
    return DriveClass::Mission_Move();
}

int UnitClass::Mission_Unload()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0057D38C, int, UnitClass *);
    return func(this);
#else
    return 0;
#endif
}

int UnitClass::Mission_Repair()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x005802F4, int, UnitClass *);
    return func(this);
#else
    return 0;
#endif
}

/**
 *
 *
 */
DirType UnitClass::Turret_Facing() const
{
    if (Class_Of().Is_Turret_Equipped()) {
        return m_TurretFacing.Get_Current();
    }
    return m_Facing.Get_Current();
}

DirType UnitClass::Desired_Load_Dir(ObjectClass *object, cell_t &cellnum) const
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0057FD8C, DirType, const UnitClass *, ObjectClass *, cell_t &);
    return func(this, object, cellnum);
#else
    return DIR_NONE;
#endif
}

DirType UnitClass::Fire_Direction() const
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x005803C4, DirType, const UnitClass *);
    return func(this);
#else
    return DIR_NONE;
#endif
}

/**
 *
 *
 */
InfantryType UnitClass::Crew_Type() const
{
    if (Is_Weapon_Equipped()) {
        return DriveClass::Crew_Type();
    }

    if (g_Scen.Get_Random_Value(0, 99) < 50) {
        return INFANTRY_C1;
    }
    return INFANTRY_C7;
}

/**
 *
 *
 */
fixed_t UnitClass::Ore_Load() const
{
    if (What_Type() == UNIT_HARVESTER) {
        return fixed_t(m_BailCount, g_Rule.Bail_Count());   
    }
    return fixed_t(0);
}

int UnitClass::Pip_Count() const
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x00580004, int, const UnitClass *);
    return func(this);
#else
    return 0;
#endif
}

/**
 *
 *
 */
FireErrorType UnitClass::Can_Fire(target_t target, WeaponSlotType weapon) const
{
    FireErrorType fireerror = DriveClass::Can_Fire(target, weapon);
    if (fireerror != FIRE_OK) {
        return fireerror;
    }

    WeaponTypeClass *wptr = Class_Of().Get_Weapon(weapon);

    if (Class_Of().Cant_Fire_Moving() && Target_Legal(m_NavCom)) {
        return FIRE_MOVING;
    }

    if (!m_Firing && m_Rotating && wptr->Get_Projectile()->Get_ROT() == 0) {
        return FIRE_ROTATING;
    }

    DirType dir = Direction(Center_Coord(), As_Coord(target));

    DirType finaldir;

    if (Class_Of().Is_Turret_Equipped()) {
        finaldir = dir - m_TurretFacing.Get_Current();
    } else {
        finaldir = dir - m_Facing.Get_Current();
    }
    DirType absdir = (DirType)std::abs(finaldir);
    if (wptr->Get_Projectile()->Get_ROT()) {
        absdir >>= 2;
    }
    if (absdir >= 8) {
        return FIRE_FACING;
    }

    return DriveClass::Can_Fire(target, weapon);
}

/**
 *
 *
 */
target_t UnitClass::Greatest_Threat(ThreatType threat)
{
    WeaponTypeClass *wptr = Class_Of().Get_Weapon(WEAPON_SLOT_PRIMARY);
    if (wptr != nullptr) {
        threat |= wptr->Allowed_Threats();
    }

    wptr = Class_Of().Get_Weapon(WEAPON_SLOT_SECONDARY);
    if (wptr != nullptr) {
        threat |= wptr->Allowed_Threats();
    }

    return DriveClass::Greatest_Threat(threat);
}

BulletClass *UnitClass::Fire_At(target_t target, WeaponSlotType weapon)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x00580554, BulletClass *, UnitClass *, target_t, WeaponSlotType);
    return func(this, target, weapon);
#else
    return nullptr;
#endif
}

void UnitClass::Assign_Destination(target_t dest)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x00580B94, void, UnitClass *, target_t);
    func(this, dest);
#endif
}

void UnitClass::Enter_Idle_Mode(BOOL a1)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0057B508, void, UnitClass *, BOOL);
    func(this, a1);
#endif
}

/**
 *
 *
 */
BOOL UnitClass::Start_Driver(coord_t &dest)
{
    if (!DriveClass::Start_Driver(dest)) {
        return false;
    }
    if (m_IsActive) {
        Mark_Track(dest, MARK_PUT);
        return true;
    }
    return false;
}

/**
 *
 *
 */
BOOL UnitClass::Offload_Ore_Bail()
{
    if (m_BailCount != 0) {
        --m_BailCount;
    }
    return false;
}

void UnitClass::Approach_Target()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x005808FC, void, UnitClass *);
    func(this);
#endif
}

void UnitClass::Overrun_Cell(cell_t cell, int a2)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x005809EC, void, UnitClass *, cell_t, int);
    func(this, cell, a2);
#endif
}

/**
 *
 *
 */
BOOL UnitClass::Ok_To_Move(DirType dir)
{
    if (!Class_Of().Is_Bit32()) {
        return false;
    }
    if (m_Rotating) {
        return false;
    }
    if (dir - m_TurretFacing.Get_Current() != 0) {
        m_TurretFacing.Set_Desired(dir);
        return false;
    }
    return true;
}

/**
 *
 *
 */
BOOL UnitClass::Edge_Of_World_AI()
{
    if (m_Mission == MISSION_GUARD && !g_Map.In_Radar(Get_Cell()) && m_LockedOnMap) {
        if (m_Team != nullptr) {
            m_Team->Set_Bit2_4(true);
        }
        Stun();
        delete this;
        return true;
    }
    return false;
}

/**
 *
 *
 */
BOOL UnitClass::Flag_Attach(HousesType house)
{
    if (house != HOUSES_NONE && m_FlagOwner == HOUSES_NONE) {
        m_FlagOwner = house;
        Mark(MARK_REDRAW);
        return true;
    }
    return false;
}

/**
 *
 *
 */
BOOL UnitClass::Flag_Remove()
{
    if (m_FlagOwner != HOUSES_NONE) {
        m_FlagOwner = HOUSES_NONE;
        Mark(MARK_REDRAW);
        return true;
    }
    return false;
}

/**
 *
 *
 */
void UnitClass::APC_Close_Door()
{
    m_Door.Close_Door(10, 2);
}

/**
 *
 *
 */
void UnitClass::APC_Open_Door()
{
    int delay;

    if (!m_Moving && !m_Rotating) {
        DirType dir = m_Facing.Get_Current();
        if (dir == DIR_NORTH_WEST || dir == DIR_NORTH_EAST) {
            delay = 10;
        } else {
            delay = 1;
        }
        m_Door.Open_Door(delay, 2);
    }
}

/**
 *
 *
 */
int UnitClass::Credit_Load()
{
    return g_Rule.Get_Gold_Value() * m_Gold + g_Rule.Get_Gem_Value() * m_Gems;
}

BOOL UnitClass::Ore_Check(short &cellnum, int a2, int a3)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0057CD9C, BOOL, UnitClass *, short &, int, int);
    return func(this, cellnum, a2, a3);
#else
    /*
    cell_t check_cell;

    if (g_Scen.Game_To_Play() == GAME_CAMPAIGN && m_PlayerOwned && !Map[check_cell].Is_Visible()) {
        return false;
    }

    cell_t cell = Get_Cell();
    CellClass *cptr = &Map[cell];
    if (cptr->Get_Zone(MZONE_NORMAL) == Class_Of().Get_Movement_Zone() && !Cell_Techno(0, 0) && Map[check_cell].Get_Land() ==
    LAND_ORE) { return true;
    }
    */
    return false;
#endif
}

/**
 * Finds nearest ore location and directs the unit to it, return value is if its on ore already.
 *
 */
BOOL UnitClass::Goto_Ore(int scan_radius)
{
    if (Target_Legal(m_NavCom)) {
        return false;
    }

    // is the cell im under ore?
    cell_t cell = Get_Cell();
    if (g_Map[cell].Get_Land() == LAND_ORE) {
        return true;
    }

    cell_t adj_cell;
    for (int i = 1; i < scan_radius; ++i) {
        for (int j = -i; j <= i; ++j) {
            adj_cell = cell;
            if (Ore_Check(adj_cell, j, -i)) {
                Assign_Destination(::As_Target(adj_cell));
                return false;
            }

            adj_cell = cell;
            if (Ore_Check(adj_cell, j, i)) {
                Assign_Destination(::As_Target(adj_cell));
                return false;
            }

            adj_cell = cell;
            if (Ore_Check(adj_cell, -i, j)) {
                Assign_Destination(::As_Target(adj_cell));
                return false;
            }

            adj_cell = cell;
            if (Ore_Check(adj_cell, i, j)) {
                Assign_Destination(::As_Target(adj_cell));
                return false;
            }
        }
    }
    return false;
}

/**
 *
 *
 */
void UnitClass::Firing_AI()
{
    if (Target_Legal(m_TarCom) && Is_Weapon_Equipped()) {
        WeaponSlotType slot = What_Weapon_Should_I_Use(m_TarCom);
        FireErrorType error = Can_Fire(m_TarCom, slot);

        switch (error) {
            case FIRE_OK:
                if (Class_Of().Is_Bit16()) {
                    Mark(MARK_5);
                    m_Firing = false;
                    Mark(MARK_4);
                }
                Fire_At(m_TarCom, slot);
                break;
            case FIRE_FACING:
                if (!Class_Of().Is_Bit32() || What_Type() == UNIT_DEMO_TRUCK) {
                    if (!Target_Legal(m_NavCom)) {
                        DirType dir = Direction(Center_Coord(), As_Coord(m_TarCom));
                        m_Facing.Set_Desired(dir);
                        m_TurretFacing.Set_Desired(m_Facing.Get_Desired());
                    }
                } else {
                    DirType dir = Direction(Center_Coord(), As_Coord(m_TarCom));
                    m_TurretFacing.Set_Desired(dir);
                }
                break;
            case FIRE_CLOAKED:
                Mark(MARK_5);
                m_Firing = false;
                Mark(MARK_4);
                Do_Uncloak();
                break;
            default:
                break;
        }
    }
}

/**
 *
 *
 */
BOOL UnitClass::Goto_Clear_Spot()
{
    static int _offsets[] ={
        -128, -256, -255, -257,
        -384, -383, -385, -382,
        -386, -512, -511, -513,
        -510, -514, 128, 256,
        257, 255, 384, 385,
        383, 386, 382, 512,
        513, 511, 514, 510,
        -1, -2, -3, -4,
        1, 2, 3, 4,
        0
    };

    BuildingTypeClass &bptr = BuildingTypeClass::As_Reference(BUILDING_FACT);

    Mark(MARK_REMOVE);

    if (!Target_Legal(m_NavCom) && bptr.Legal_Placement(AdjacentCell[7] + Get_Cell())) {
        Mark(MARK_PUT);
        return true;
    }

    if (!Target_Legal(m_NavCom)) {
        int *offset = _offsets;
        while (!*offset != 0) {
            cell_t cell = Coord_To_Cell(m_Coord) + *offset;
            ++offset;

            if (bptr.Legal_Placement(cell + AdjacentCell[7])) {
                Assign_Destination(::As_Target(cell));
                break;
            }
        }  
    }

    Mark(MARK_PUT);
    if (!Target_Legal(m_NavCom)) {
        if (!m_OwnerHouse->Is_Human()) {
            Scatter(0, 0, 0);
        }
    }
    return false;
}

BOOL UnitClass::Try_To_Deploy()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0057B8F4, BOOL, UnitClass *);
    return func(this);
#else
    return false;
#endif
}

void UnitClass::AI()
{
    DriveClass::AI();
    if (m_IsActive && m_Height <= 0) {
        if (m_Mission != MISSION_HARVEST) {
            m_IsHarvesting = false;
        }
        Firing_AI();
        if (m_IsActive) {
            //Rotation_AI();
            if (!Edge_Of_World_AI()) {
                //Reload_AI();
                if (Class_Of().Max_Passengers() > 0) {
                    if (/**(&this->d.f.t.Door.DoorTimer.Delay + 2) >> 24*/ 1) {
                        if (m_Mission != MISSION_UNLOAD && Transmit_Message(RADIO_NEED_A_LIFT) != RADIO_ROGER) {
                            APC_Close_Door();
                        }
                    }
                }

                if (!m_IsDumping && !m_Moving /*!(*(&this->d.f.t.Door.DoorTimer.Delay + 2) >> 24)*/) {
                    Commence();
                }

                if (m_CloakState == 2 && m_FlagOwner != HOUSES_NONE) {
                    Do_Shimmer();
                }

                if (Class_Of().Is_Mobile_Gap_Gen()) {
                    if (!m_Moving && !(g_GameFrame % 15)) {
                        //Shroud_Regen();
                    }
                }
            }
        }
    }
}
