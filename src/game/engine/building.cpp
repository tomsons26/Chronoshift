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
#include "building.h"
#include <algorithm>

#ifndef GAME_DLL
TFixedIHeapClass<BuildingClass> g_Buildings;
#endif

BuildingClass::BuildingClass(BuildingType type, HousesType house) :
    TechnoClass(RTTI_BUILDING, g_Buildings.ID(this), house)
{
}

BuildingClass::BuildingClass(const BuildingClass &that) :
    TechnoClass(that)
{
}

BuildingClass::BuildingClass(const NoInitClass &noinit) :
    TechnoClass(noinit)
{
}

BuildingClass::~BuildingClass()
{}

/**
 *
 *
 * @address 0x00460AD0
 */
const BuildingTypeClass &BuildingClass::Class_Of() const
{
    return *m_Class;
}

/**
 *
 *
 * @address 0x0045DE68
 */
void BuildingClass::Death_Announcement(TechnoClass *killer) const
{
    if (killer != nullptr) {
        if (m_OwnerHouse->Player_Has_Control()){
            Speak(VOX_STRUCTURE_DESTROYED);
        }
    }
}

/**
 *
 *
 * @address 0x0045DD94
 */
void BuildingClass::Enter_Idle_Mode(BOOL a1)
{
    MissionType mission;
    if ( !a1 || ScenarioInit || g_Debug_Map )
    {
        BuildingClass::Begin_Mode(BSTATE_1);
        mission = MISSION_GUARD;
    } else {
        BuildingClass::Begin_Mode(BSTATE_0);
        mission = MISSION_CONSTRUCTION;
    }
    Assign_Mission(mission);
}

/**
 *
 *
 * @address 0x0045EAC0
 */
BOOL BuildingClass::Can_Player_Move()
{
    return What_Type() == BUILDING_FACT;
}

/**
 *
 *
 * @address 0x0045E3E4
 */
void BuildingClass::Detach(target_t target, int a2)
{
    TechnoClass::Detach(target, a2);
    if (m_SabotagedBy == target) {
        m_SabotagedBy = 0;
    }
    if (m_AttachedAnim == target) {
        m_AttachedAnim = 0;
    }
}
