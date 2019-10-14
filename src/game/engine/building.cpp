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
#include "globals.h"
#include "iomap.h"
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
    if (killer != nullptr && m_OwnerHouse->Player_Has_Control()) {
        Speak(VOX_STRUCTURE_DESTROYED);
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
    if ( !a1 || ScenarioInit || g_InMapEditor )
    {
        //BuildingClass::Begin_Mode(BSTATE_1);
        mission = MISSION_GUARD;
    } else {
        //BuildingClass::Begin_Mode(BSTATE_0);
        mission = MISSION_CONSTRUCTION;
    }
    Assign_Mission(mission);
>>>>>>> Implements various small/simple functions.
}

/**
 *
 *
 * @address 0x0045EAC0
 */
BOOL BuildingClass::Can_Player_Move() const
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

/**
 *
 *
 * @address 0x0046072C
 */
int BuildingClass::Value() const
{
    if (Class_Of().Is_Fake()) {
        BuildingTypeClass *btptr = nullptr;
        switch (What_Type()) {
            case BUILDING_WEAF:
                btptr = &BuildingTypeClass::As_Reference(BUILDING_WEAP);
                return btptr->Get_ThreatPosed() + btptr->Get_ThreatPoints();
            case BUILDING_FACF:
                btptr = &BuildingTypeClass::As_Reference(BUILDING_FACT);
                return btptr->Get_ThreatPosed() + btptr->Get_ThreatPoints();
            case BUILDING_SYRF:
                btptr = &BuildingTypeClass::As_Reference(BUILDING_SYRD);
                return btptr->Get_ThreatPosed() + btptr->Get_ThreatPoints();
            case BUILDING_SPEF:
                btptr = &BuildingTypeClass::As_Reference(BUILDING_SPEN);
                return btptr->Get_ThreatPosed() + btptr->Get_ThreatPoints();
            case BUILDING_DOMF:
                btptr = &BuildingTypeClass::As_Reference(BUILDING_DOME);
                return btptr->Get_ThreatPosed() + btptr->Get_ThreatPoints();
            default:
                DEBUG_LOG("BuildingClass::Value - Unhandled fake!\n");
                break;
        }
    }
    return TechnoClass::Value();
}

/**
 *
 *
 * @address 0x004606D0
 */
void *BuildingClass::Get_Image_Data() const
{
    if (m_CurrentState == BSTATE_0) {
        return Class_Of().Get_Buildup_Data();
    }
    return ObjectClass::Get_Image_Data();
}

/**
 *
 *
 * @address 0x0045EB90
 */
cell_t BuildingClass::Check_Point(CheckPointType check) const
{
    cell_t xoff = 6;
    cell_t yoff = 5;
    switch (check) {
        case CHECKPOINT_0:
            xoff = 0;
            break;
        case CHECKPOINT_2:
            yoff = 0;
            break;
        case CHECKPOINT_1:
        default:
            break;
    };

    cell_t cellnum = Center_Cell();
    if ((Map.Get_Map_Cell_Width() / 2) < (Cell_Get_X(cellnum) - Map.Get_Map_Cell_X())) {
        xoff = -xoff;
    }
    if ((Map.Get_Map_Cell_Height() / 2) < (Cell_Get_Y(cellnum) - Map.Get_Map_Cell_Y())) {
        yoff = -yoff;
    }
    return Cell_From_XY(xoff + Cell_Get_X(cellnum), yoff + Cell_Get_Y(cellnum));
>>>>>>> Implements various small/simple functions.
}
