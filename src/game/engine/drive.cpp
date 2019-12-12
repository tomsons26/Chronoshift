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
#include "drive.h"
#include "building.h"
#include "gametypes.h"
#include "globals.h"
#include "iomap.h"
#include "rules.h"
#include "scenario.h"
#include "target.h"
#include "unit.h"
#include "voc.h"

DriveClass::DriveClass(RTTIType type, int id, HousesType house) :
    FootClass(type, id, house),
    m_IsHarvesting(false),
    m_Teleported(false),
    m_Bit_4(false),
    m_Bit_8(false),
    m_field_145(),
    m_TeleportReturnLocation(0),
    m_SpeedAccum(0),
    m_TrackNumber(-1),
    m_TrackIndex(0)
{
}

DriveClass::DriveClass(const DriveClass &that) :
    FootClass(that),
    m_IsHarvesting(that.m_IsHarvesting),
    m_Teleported(that.m_Teleported),
    m_Bit_4(that.m_Bit_4),
    m_Bit_8(that.m_Bit_8),
    m_field_145(that.m_field_145),
    m_TeleportReturnLocation(that.m_TeleportReturnLocation),
    m_SpeedAccum(that.m_SpeedAccum),
    m_TrackNumber(that.m_TrackNumber),
    m_TrackIndex(that.m_TrackIndex)
{
}

DriveClass::DriveClass(const NoInitClass &noinit) :
    FootClass(noinit),
    m_field_145(noinit)
{
}

#ifdef CHRONOSHIFT_DEBUG
void DriveClass::Debug_Dump(MonoClass *mono) const
{
    FootClass::Debug_Dump(mono);
}
#endif

void DriveClass::AI()
{
    FootClass::AI();

    UnitClass *uptr = (UnitClass *)this;

    if (m_IsActive && m_Height <= 0) {
        if (m_Teleported) {
            // change based on Ghidra output
            //if (m_RTTI != RTTI_UNIT || uptr->What_Type() != UNIT_CHRONO) {
            if (m_RTTI == RTTI_UNIT && uptr->What_Type() != UNIT_CHRONO) {
                if (m_field_145.Expired()){
                    m_Teleported = false;
                    Teleport_To(m_TeleportReturnLocation);
                    m_TeleportReturnLocation = 0;
                }
            }
        }
        if (m_TrackNumber != -1) {
            While_Moving();
            //FIXME
            if (m_IsActive && m_TrackNumber == -1 && (Target_Legal(m_NavCom) || m_Paths[0] != FACING_NONE) && (m_RTTI != RTTI_UNIT || !uptr->Is_Dumping())){
                Start_Of_Move();
                if (m_IsActive) {
                    While_Moving();
                }
            }
            
        } else if (m_Facing.Has_Changed()){
            Mark(MARK_3);
            //FIXME
            if (/*m_Facing.Rotation_Adjust()*/ 1) {
                Mark(MARK_3);
            }
            if (!m_Rotating) {
                Per_Cell_Process(PCP_0);
            }

        //FIXME
        } else if ((m_Mission != MISSION_GUARD || Target_Legal(m_NavCom)) && m_Mission != MISSION_UNLOAD) {
            if (Target_Legal(m_NavCom) || m_Paths[0] != FACING_NONE) {
                //FIXME
                if (m_LockedOnMap && m_Mission != MISSION_ENTER && Target_Legal(m_NavCom) && !Is_In_Same_Zone(As_Cell(m_NavCom))) {
                    Stop_Driver();
                    Assign_Destination(0);
                } else {
                    Start_Of_Move();
                    if (m_IsActive) {
                        While_Moving();
                    }
                }
            } else {
                Stop_Driver();
            }
        }
    }
}

/**
 *
 *
 */
BOOL DriveClass::Limbo()
{
    if (!m_InLimbo) {
        Stop_Driver();
        m_TrackNumber = -1;
    }
    return FootClass::Limbo();
}

void DriveClass::Scatter(coord_t coord, int a2, BOOL a3)
{
#ifdef GAME_DLL
    void (*func)(DriveClass *, coord_t, int, BOOL) = reinterpret_cast<void (*)(DriveClass *, coord_t, int, BOOL)>(0x004B6304);
    func(this, coord, a2, a3);
#else
    DEBUG_ASSERT_PRINT(false, "Unimplemented function '%s' called!\n", __FUNCTION__);
    if (!Get_Mission_Control().Is_Paralyzed() && (m_RTTI != RTTI_UNIT || !m_IsDumping)
      && (!Target_Legal(m_NavCom) || a3 && !m_Rotating)
      && (!Target_Legal(m_TarCom) || a2 || g_Scen.Get_Random_Value(1, 4) == 1)) {
        if offset = 0;
        if (coord != 0) {
            int v5 = 0;//(Desired_Facing8(coord, HIWORD(coord), this->d.f.t.r.m.o.a.Coord, this->d.f.t.r.m.o.a.Coord >> 16) + 16) >> 5;
            offset = (v5 +  g_Scen.Get_Random_Value(0, 2) - 1) & 7;
        } else {
            int v6 = 0;//(this->d.f.t.Facing.Current + 16) >> 5;
            offset = (v6 +  g_Scen.Get_Random_Value(0, 2) - 1) & 7;
        }
        for (FacingType i = FACING_FIRST; i < FACING_COUNT; ++i) {
            cell_t cell = 0; //AdjacentCell[(i + offset) & 7] + Get_Cell();
            if (g_Map.In_Radar(cell) && Can_Enter_Cell(cell) == MOVE_OK) {
                Assign_Destination(As_Target(cell));
            }
        }
    }

#endif
}

void DriveClass::Per_Cell_Process(PCPType pcp)
{
/*#ifdef GAME_DLL
    void(*func)(DriveClass *, PCPType) = reinterpret_cast<void(*)(DriveClass *, PCPType)>(0x004B6F40);
    func(this, pcp);
#else*/
    if ( pcp == 2 ) {
        if (As_Cell(m_NavCom) == Get_Cell()) {
            m_NavCom = 0;
            m_Paths[0] = FACING_NONE;
            m_Bit_4 = false;
        }
        Lay_Track();
    }
    FootClass::Per_Cell_Process(pcp);
//#endif
}

/**
 *
 *
 */
void DriveClass::Response_Select()
{
    static VocType _response[] = {
        VOC_VEHIC1, VOC_REPORT1, VOC_YESSIR1, VOC_AWAIT1
    };
    if (g_AllowVoice) {
        Sound_Effect(_response[g_Scen.Get_Random_Value(0, ARRAY_SIZE(_response))], fixed_t(1, 1), -(Get_Heap_ID() + 1));
    }
}

/**
 *
 *
 */
void DriveClass::Response_Move()
{
    static VocType _response[] = {
        VOC_ACKNO, VOC_AFFIRM1
    };
    if (g_AllowVoice) {
        Sound_Effect(_response[g_Scen.Get_Random_Value(0, ARRAY_SIZE(_response))], fixed_t(1, 1), -(Get_Heap_ID() + 1));
    }
}

/**
 *
 *
 */
void DriveClass::Response_Attack()
{
    static VocType _response[] = {
        VOC_AFFIRM1, VOC_ACKNO
    };
    if (g_AllowVoice) {
        Sound_Effect(_response[g_Scen.Get_Random_Value(0, ARRAY_SIZE(_response))], fixed_t(1, 1), -(Get_Heap_ID() + 1));
    }
}

/**
 *
 *
 */
void DriveClass::Assign_Destination(target_t dest)
{
    if (m_NavCom != dest) {

        BuildingClass *bptr = As_Building(dest);
        if (bptr != nullptr && bptr->What_Type() == BUILDING_PROC) {
            if (What_Am_I() == RTTI_UNIT && reinterpret_cast<UnitClass *>(this)->Class_Of().Is_Harvester()) {
                if (m_Radio != bptr && m_Radio == nullptr && Transmit_Message(RADIO_HELLO, bptr) == RADIO_ROGER) {
                    if (m_Mission != MISSION_ENTER && m_Mission != MISSION_HARVEST) {
                        dest = 0;
                        Assign_Mission(MISSION_ENTER);
                    }
                }
            }
        }

        FootClass::Assign_Destination(dest);
        m_Paths[0] = FACING_NONE;
        if (!m_Moving && m_Mission != MISSION_UNLOAD) {
            Start_Of_Move();
        }
    }
}

/**
 *
 *
 */
BOOL DriveClass::Stop_Driver()
{
    if (m_HeadTo) {
        if (m_IsDown) {
            Mark(MARK_REMOVE);
        }
        Mark_Track(m_HeadTo, MARK_REMOVE);
        if (m_IsDown) {
            Mark(MARK_PUT);
        }
    }
    return FootClass::Stop_Driver();
}

void DriveClass::Fixup_Path(PathType *path)
{
#ifdef GAME_DLL
    void (*func)(DriveClass *, PathType *) = reinterpret_cast<void (*)(DriveClass *, PathType *)>(0x004B7F4C);
    func(this, path);
#else
    DEBUG_ASSERT_PRINT(false, "Unimplemented function '%s' called!\n", __FUNCTION__);
#endif
}

void DriveClass::Overrun_Cell(cell_t cell, int a2)
{
#ifdef GAME_DLL
    int (*func)(DriveClass *, cell_t, int) = reinterpret_cast<int (*)(DriveClass *, cell_t, int)>(0x004B8470);
    func(this, cell, a2);
#else
    DEBUG_ASSERT_PRINT(false, "Unimplemented function '%s' called!\n", __FUNCTION__);
#endif
}

BOOL DriveClass::Ok_To_Move(DirType dir)
{
#ifdef GAME_DLL
    BOOL (*func)(DriveClass *, DirType) = reinterpret_cast<BOOL (*)(DriveClass *, DirType)>(0x004B83AC);
    return func(this, dir);
#else
    DEBUG_ASSERT_PRINT(false, "Unimplemented function '%s' called!\n", __FUNCTION__);
    return false;
#endif
}

void DriveClass::Do_Turn(DirType dir)
{
#ifdef GAME_DLL
    void (*func)(DriveClass *, DirType) = reinterpret_cast<void (*)(DriveClass *, DirType)>(0x004B6514);
    func(this, dir);
#else
    DEBUG_ASSERT_PRINT(false, "Unimplemented function '%s' called!\n", __FUNCTION__);
#endif
}

/**
 *
 *
 */
BOOL DriveClass::Teleport_To(cell_t cell)
{
    DEBUG_ASSERT(Valid_Cell(cell));

    if (g_Rule.Chrono_Kills_Cargo()) {
        Kill_Cargo(nullptr);
    }
    Stop_Driver();

    Force_Track(-1, 0);

    m_Facing.Set_Current(m_Facing.Get_Desired());

    Transmit_Message(RADIO_OVER_AND_OUT);

    Assign_Destination(0);
    Assign_Target(0);

    Assign_Mission(MISSION_NONE);
    Commence();

    Mark(MARK_REMOVE);

    UnitClass *uptr = reinterpret_cast<UnitClass *>(this);
    if (m_RTTI == RTTI_UNIT && uptr->Flag_Owner() != HOUSES_NONE) {
        // needs used housetype member verification
        HouseClass::As_Pointer(uptr->Flag_Owner())->Flag_Attach(Get_Cell());
    }
    if (Can_Enter_Cell(cell)) {
        // needs speed access verification
        cell = g_Map.Nearby_Location(cell, Techno_Class_Of().Get_Speed());
    }
    // set coord code needs verification
    m_Coord = Cell_To_Coord(cell);
    Mark(MARK_PUT);
    return true;
}

void DriveClass::Force_Track(int track, coord_t coord)
{
#ifdef GAME_DLL
    void (*func)(DriveClass *, int, coord_t) = reinterpret_cast<void (*)(DriveClass *, int, coord_t)>(0x004B669C);
    func(this, track, coord);
#else
    DEBUG_ASSERT_PRINT(false, "Unimplemented function '%s' called!\n", __FUNCTION__);
#endif
}

coord_t DriveClass::Smooth_Turn(coord_t coord, DirType &dir)
{
#ifdef GAME_DLL
    coord_t (*func)(DriveClass *, coord_t, DirType &) =
        reinterpret_cast<coord_t (*)(DriveClass *, coord_t, DirType &)>(0x004B6748);
    return func(this, coord, dir);
#else
    DEBUG_ASSERT_PRINT(false, "Unimplemented function '%s' called!\n", __FUNCTION__);
    return 0;
#endif
}

BOOL DriveClass::While_Moving()
{
#ifdef GAME_DLL
    BOOL (*func)(DriveClass *) = reinterpret_cast<BOOL (*)(DriveClass *)>(0x004B68B0);
    return func(this);
#else
    DEBUG_ASSERT_PRINT(false, "Unimplemented function '%s' called!\n", __FUNCTION__);
    return false;
#endif
}

BOOL DriveClass::Start_Of_Move()
{
#ifdef GAME_DLL
    BOOL (*func)(DriveClass *) = reinterpret_cast<BOOL (*)(DriveClass *)>(0x004B6FA0);
    return func(this);
#else
    DEBUG_ASSERT_PRINT(false, "Unimplemented function '%s' called!\n", __FUNCTION__);
    return false;
#endif
}

void DriveClass::Lay_Track()
{
    // TODO: Leftover from Dune2 for laying the visual track?
}

void DriveClass::Mark_Track(coord_t coord, MarkType mark)
{
#ifdef GAME_DLL
    void (*func)(DriveClass *, coord_t, MarkType) = reinterpret_cast<void (*)(DriveClass *, coord_t, MarkType)>(0x004B82AC);
    func(this, coord, mark);
#else
    DEBUG_ASSERT_PRINT(false, "Unimplemented function '%s' called!\n", __FUNCTION__);
#endif
}
