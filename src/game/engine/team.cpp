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
#include "team.h"
#include "gamedebug.h"
#include "foot.h"
#include "target.h"

#ifndef GAME_DLL
TFixedIHeapClass<TeamClass> g_Teams;
#endif

TeamClass::TeamClass()
{
}

TeamClass::TeamClass(const TeamClass &that)
{
}

TeamClass::TeamClass(const NoInitClass &noinit)
{
}

TeamClass::~TeamClass()
{
}

void *TeamClass::operator new(size_t size)
{
    DEBUG_ASSERT(size == sizeof(TeamClass) && size == g_Teams.Heap_Size());
    TeamClass *this_ptr = g_Teams.Alloc();
    DEBUG_ASSERT(this_ptr != nullptr);
    if (this_ptr != nullptr) {
        this_ptr->m_IsActive = true;
    }
    return this_ptr;
}

void TeamClass::operator delete(void *ptr)
{
    TeamClass *this_ptr = static_cast<TeamClass *>(ptr);
    DEBUG_ASSERT(this_ptr != nullptr);
    if (this_ptr != nullptr) {
        this_ptr->m_IsActive = false;
    }
    g_Teams.Free(this_ptr);
}

void TeamClass::Assign_Mission_Target(target_t target)
{
#ifdef GAME_DLL
    
    DEFINE_CALL(func, 0x0055C0D8, void, TeamClass *, target_t);
    func(this, target);
#endif
}

BOOL TeamClass::Remove(FootClass *object, int a2)
{
#ifdef GAME_DLL
    BOOL (*func)(TeamClass *, FootClass *, int) = reinterpret_cast<BOOL (*)(TeamClass *, FootClass *, int)>(0x004C3794);
    return func(this, object, a2);
#else
    //DEBUG_ASSERT_PRINT(false, "Unimplemented function '%s' called!\n", __FUNCTION__);
    return false;
#endif
}


void TeamClass::Coordinate_Attack()
{
#ifdef GAME_DLL
    
    DEFINE_CALL(func, 0x0055E04C, void, TeamClass *);
    func(this);
#endif
}

/**
 * @brief
 *
 * @address 0x004F93DC
 */
void TeamClass::Code_Pointers()
{
    if (m_field_54 != nullptr) {
        m_field_54 = reinterpret_cast<FootClass *>(m_field_54->As_Target());
    }
}

/**
 * @brief
 *
 * @address 0x004F9420
 */
void TeamClass::Decode_Pointers()
{
    if (m_field_54 != nullptr) {
        m_field_54 = reinterpret_cast<FootClass *>(As_Techno((uintptr_t)m_field_54));
    }
}

// TODO, move
ThreatType Threat_From_Quarry(QuarryType quarry)
{
    switch (quarry) {
        // deviates from function found in YR here, original code returned THREAT_ANY
        default:
            return THREAT_NONE;

        case QUARRY_ANYTHING:
            return THREAT_ANY;

        case QUARRY_ANY_BUILDING:
            return THREAT_BUILDINGS;

        case QUARRY_HARVESTERS:
            return THREAT_ECONOMY;

        case QUARRY_INFANTRY:
            return THREAT_INFANTRY;

        case QUARRY_ANY_VEHICLES:
            return THREAT_VEHICLES;

        // deviates from function found in YR here, RA code doesn't handle vessels
        case QUARRY_ANY_VESSEL:
            return THREAT_NONE; // THREAT_VESSELS;

        case QUARRY_FACTORIES:
            return THREAT_FACTORIES;

        case QUARRY_BASE_DEFENSES:
            return THREAT_BASE_DEFENSES;

        case QUARRY_BASE_THREATS:
            return THREAT_ANY;

        case QUARRY_POWER_FACILITIES:
            return THREAT_POWER_FACILTIES;

        case QUARRY_FAKE_BUILDINGS:
            return THREAT_FAKES;
    }
}

/**
 * @brief
 *
 * @address 0x0055F798
 */
BOOL TeamClass::TMission_Attack()
{
    if (m_field_29 == 0 && m_Members) {
        FootClass *leader = Fetch_A_Leader();
        TeamMissionClass &tmission = m_Class->Get_Team_Mission(m_Mission);
        /*
        switch (tmission.Get_Quarry()) {
            case QUARRY_ANYTHING:
            case QUARRY_BASE_THREATS:
                Assign_Mission_Target(leader->Greatest_Threat(THREAT_ANY));
                break;
            case QUARRY_ANY_BUILDING:
                Assign_Mission_Target(leader->Greatest_Threat(THREAT_BUILDINGS));
                break;
            case QUARRY_HARVESTERS:
                Assign_Mission_Target(leader->Greatest_Threat(THREAT_HARVESTERS));
                break;
            case QUARRY_INFANTRY:
                Assign_Mission_Target(leader->Greatest_Threat(THREAT_INFANTRY));
                break;
            case QUARRY_ANY_VEHICLES:
                Assign_Mission_Target(leader->Greatest_Threat(THREAT_VEHICLES));
                break;
            // not handled in original
            //case QUARRY_ANY_VESSEL:
            //    Assign_Mission_Target(leader->Greatest_Threat(THREAT_VESSELS));
            //    break;
            //
            case QUARRY_FACTORIES:
                Assign_Mission_Target(leader->Greatest_Threat(THREAT_FACTORIES));
                break;
            case QUARRY_BASE_DEFENSES:
                Assign_Mission_Target(leader->Greatest_Threat(THREAT_BASE_DEFENSES));
                break;
            case QUARRY_POWER_FACILITIES:
                Assign_Mission_Target(leader->Greatest_Threat(THREAT_POWER_FACILTIES));
                break;
            case QUARRY_FAKE_BUILDINGS:
                Assign_Mission_Target(leader->Greatest_Threat(THREAT_FAKE_BUILDINGS));
                break;
            default:
                break;
        }
        */
        ThreatType threat = Threat_From_Quarry(tmission.Get_Quarry());
        if (threat != THREAT_NONE) {
            Assign_Mission_Target(leader->Greatest_Threat(threat));
        }
        if (m_field_29 == 0) {
            m_Bit2_2 = true;
        }
    }
    Coordinate_Attack();
    return true;
}

FootClass *TeamClass::Fetch_A_Leader()
{
    return nullptr;
}
