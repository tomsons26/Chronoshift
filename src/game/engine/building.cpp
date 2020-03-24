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
#include "bullettype.h"
#include "drawshape.h"
#include "gameevent.h"
#include "globals.h"
#include "house.h"
#include "iomap.h"
#include "overlay.h"
#include "overlaytype.h"
#include "queue.h"
#include "rules.h"
#include "session.h"
#include "smudge.h"
#include <algorithm>

#ifndef GAME_DLL
TFixedIHeapClass<BuildingClass> g_Buildings;
#endif

BuildingClass::BuildingClass(BuildingType type, HousesType house) : TechnoClass(RTTI_BUILDING, g_Buildings.ID(this), house)
{
}

BuildingClass::BuildingClass(const BuildingClass &that) : TechnoClass(that) {}

BuildingClass::BuildingClass(const NoInitClass &noinit) : TechnoClass(noinit) {}

BuildingClass::~BuildingClass() {}

coord_t BuildingClass::Center_Coord() const
{
    const coord_t CenterOffset[] = {
        0x00800080, 0x008000FF, 0x00FF0080, 0x00FF00FF, 0x018000FF, 0x00FF0180, 0x01800180, 0x00FF0200, 0x02800280
    };

    return Coord_Add(m_Coord, CenterOffset[Class_Of().Building_Size()]);
}

/**
 *
 *
 */
coord_t BuildingClass::Target_Coord() const
{
    coord_t coord = Center_Coord();

    if (Class_Of().Deploy_Facing() != FACING_NONE) {
        return Coord_Get_Adjacent(coord, Class_Of().Deploy_Facing());
    }

    return coord;
}

MoveType BuildingClass::Can_Enter_Cell(cell_t cellnum, FacingType facing) const
{
    if (What_Type() == BUILDING_FACT && m_IsDown) {
        if (g_Map[cellnum].Is_Clear_To_Build(Class_Of().Get_Speed())) {
            return MOVE_OK;
        }

        return MOVE_NO;
    }

    if (!g_InMapEditor && g_ScenarioInit == 0 && g_Session.Game_To_Play() == GAME_CAMPAIGN
        && Get_Owner_House()->Player_Has_Control() && !g_Map[cellnum].Is_Visible()) {
        return MOVE_NO;
    }

    if (Class_Of().Legal_Placement(cellnum)) {
        return MOVE_OK;
    }

    return MOVE_NO;
}

void BuildingClass::AI()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x00456174, void, BuildingClass *);
    func(this);
#endif
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
    return TechnoClass::Get_Image_Data();
}

ActionType BuildingClass::What_Action(ObjectClass *object) const
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045A3BC, ActionType, const BuildingClass *, ObjectClass *);
    return func(this, object);
#else
    return ACTION_NONE;
#endif
}

/**
 *
 *
 */
ActionType BuildingClass::What_Action(cell_t cellnum) const
{
    ActionType action = TechnoClass::What_Action(cellnum);

    switch (action) {
        case ACTION_MOVE:
            if (What_Type() != BUILDING_FACT || !g_Rule.Allow_MCV_Undeploy()) {
                return ACTION_NONE;
            }
            break;

        case ACTION_ATTACK: {
            WeaponTypeClass *wptr = Class_Of().Get_Weapon(WEAPON_SLOT_PRIMARY);

            if (wptr != nullptr && !wptr->Get_Projectile()->Is_Anti_Ground()) {
                return ACTION_NONE;
            }
            break;
        }

        default:
            break;
    }

    return action;
}

BOOL BuildingClass::Can_Demolish() const
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045BAAC, BOOL, const BuildingClass *);
    return func(this);
#else
    return 0;
#endif
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

coord_t BuildingClass::Docking_Coord() const
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045A8E4, coord_t, const BuildingClass *);
    return func(this);
#else
    return 0;
#endif
}

/**
 *
 *
 */
coord_t BuildingClass::Sort_Y() const
{
    switch (What_Type()) {
        case BUILDING_FIX:
            return m_Coord;

        case BUILDING_BARR:
        case BUILDING_PROC:
            return Center_Coord();

        case BUILDING_MINV:
        case BUILDING_MINP:
            return Coord_Move(Center_Coord(), DIR_NORTH, 256);

        default:
            break;
    }

    return Coord_Add(Center_Coord(), Coord_From_Lepton_XY(0, (Class_Of().Height() * 256) / 3));
}

coord_t BuildingClass::Exit_Coord() const
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045EB04, coord_t, const BuildingClass *);
    return func(this);
#else
    return 0;
#endif
}

/**
 *
 *
 */
BOOL BuildingClass::Limbo()
{
    if (!m_InLimbo) {
        m_OwnerHouse->Active_Remove(this);
        m_OwnerHouse->Flag_To_Recalc();
        m_OwnerHouse->Recalc_Center();
        m_OwnerHouse->Adjust_Power(-Power_Output());
        m_OwnerHouse->Adjust_Drain(-Class_Of().Power_Drain());
        m_OwnerHouse->Adjust_Capacity(-Class_Of().Storage_Capacity(), true);

        if (m_OwnerHouse == g_PlayerPtr) {
            g_Map.Flag_Power_To_Redraw();
            g_Map.Flag_To_Redraw();
        }
    }
    return TechnoClass::Limbo();
}

BOOL BuildingClass::Unlimbo(coord_t coord, DirType dir)
{
    HouseClass *owner = Get_Owner_House();

    if (Class_Of().Is_Wall()) {
        if (Can_Enter_Cell(Get_Cell()) != MOVE_OK) {
            return false;
        }

        OverlayType overlay = OVERLAY_NONE;
        switch (What_Type()) {
            case BUILDING_SBAG:
                overlay = OVERLAY_SANDBAG;
                break;
            case BUILDING_CYCL:
                overlay = OVERLAY_CYCLONE_FENCE;
                break;
            case BUILDING_BRIK:
                overlay = OVERLAY_BRICK_WALL;
                break;
            case BUILDING_BARB:
                overlay = OVERLAY_BARB_WIRE;
                break;
            case BUILDING_WOOD:
                overlay = OVERLAY_WOOD_FENCE;
                break;
            case BUILDING_FENC:
                overlay = OVERLAY_FENCE;
                break;
            default:
                overlay = OVERLAY_NONE;
                break;
        }

        if (overlay != OVERLAY_NONE) {
            OverlayTypeClass *otptr = &OverlayTypeClass::As_Reference(overlay);
            OverlayClass *optr = (OverlayClass *)otptr->Create_One_Of(owner);
            if (optr != nullptr && optr->Unlimbo(coord)) {
                g_Map[Coord_To_Cell(coord)].Set_Owner(owner->What_Type());

                Transmit_Message(RADIO_OVER_AND_OUT);

                g_Map.Sight_From(Coord_To_Cell(coord), Class_Of().Get_Sight(), owner, false);

                delete this;

                return true;
            }
        }
        return false;
    }

    if (!TechnoClass::Unlimbo(coord, dir)) {
        return false;
    }

    /*
    v24 = &CCPtr<HouseClass>::operator->(&v32->t.OwnerHouse)->BScan.field_0;
    v12 = CCPtr<BuildingTypeClass>::operator->(&v32->Class);
    *v24 |= 1 << v12->Type;
    v23 = &CCPtr<HouseClass>::operator->(&v32->t.OwnerHouse)->BScan.field_4;
    v13 = CCPtr<BuildingTypeClass>::operator->(&v32->Class);
    *v23 |= 1 << v13->Type;
    */

    owner->Recalc_Center();
    owner->Active_Add(this);
    owner->Flag_To_Recalc();

    m_field_EA = 0;

    if (!m_PlayerAware && g_Map[Coord_To_Cell(coord)].Is_Revealed() || g_Session.Game_To_Play() != GAME_CAMPAIGN) {
        Revealed(g_PlayerPtr);
    }

    if (!owner->Is_Human()) {
        Revealed(owner);
    }
    if (m_PlayerOwned) {
        g_Map.Flag_Power_To_Redraw();
        g_Map.Flag_To_Redraw();
    }
    /*
    if ((CCPtr<BuildingTypeClass>::operator->(&v32->Class)->tt.Owner & 0x300) != 768) {
        if (CCPtr<BuildingTypeClass>::operator->(&v32->Class)->tt.Owner & 0x100) {
            m_field_D5 = HOUSES_GREECE;
        } else {
            m_field_D5 = HOUSES_USSR;
        }
    }
    */
    return true;
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

void BuildingClass::Detach_All(int a1)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045E57C, void, BuildingClass *, int);
    func(this, a1);
#endif
}

int BuildingClass::Exit_Object(TechnoClass *object)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x00458A40, int, BuildingClass *, TechnoClass *);
    return func(this, object);
#else
    return 0;
#endif
}

const int16_t *BuildingClass::Overlap_List(BOOL a1) const
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x00460A04, const int16_t *, const BuildingClass *, BOOL);
    return func(this, a1);
#else
    return 0;
#endif
}

/**
 *
 *
 */
void BuildingClass::Draw_It(int x, int y, WindowNumberType window) const
{
    void *image = Get_Image_Data();
    if (image != nullptr) {
        g_IsTheaterShape = Class_Of().Is_Theater();
        Techno_Draw_Object(image, Shape_Number(), x, y, window);
        g_IsTheaterShape = false;
        if (m_CurrentState != BSTATE_0) {
            if (m_Tethered && m_Radio != nullptr && !m_Radio->In_Limbo() && m_Radio->What_Am_I() != RTTI_BUILDING) {
                int radio_x = Lepton_To_Pixel(Coord_Lepton_X(m_Radio->Render_Coord()));
                int offset_x = radio_x - Lepton_To_Pixel(Coord_Lepton_X(Render_Coord()));
                int radio_y = Lepton_To_Pixel(Coord_Lepton_Y(m_Radio->Render_Coord()));
                int offset_y = radio_y - Lepton_To_Pixel(Coord_Lepton_Y(Render_Coord()));

                m_Radio->Draw_It(x + offset_x, y + offset_y, window);
                m_Radio->Set_ToDisplay(false);
            }

            // draw war factory doors
            if (What_Type() == BUILDING_WEAP || What_Type() == BUILDING_WEAF) {
                int frame = m_Door.Door_Stage();
                if (Health_Ratio() <= g_Rule.Condition_Yellow()) {
                    frame += 4;
                }
                Techno_Draw_Object(BuildingTypeClass::g_WarFactoryOverlay, frame, x, y, window);
            }

            // draw repair shape on building
            if (m_IsRepairing && m_Bit32) {
                CC_Draw_Shape(ObjectTypeClass::SelectShapes, 2, x, y, window, SHAPE_CENTER | SHAPE_VIEWPORT_REL);
            }
        }
        TechnoClass::Draw_It(x, y, window);

        // draw what's currently in production has spied this
        if ((1 << g_PlayerPtr->What_Type()) & m_Spied && m_Selected) {
            FactoryClass *fptr = nullptr;
            if (m_OwnerHouse->Is_Human()) {
                fptr = m_OwnerHouse->Fetch_Factory(Class_Of().Factory_Type());
            } else {
                fptr = m_Factory;
            }
            if (fptr != nullptr) {
                TechnoClass *tptr = fptr->Get_Object();
                if (tptr != nullptr) {
                    CC_Draw_Shape(tptr->Class_Of().Get_Cameo_Data(), 0, x, y, window, SHAPE_CENTER | SHAPE_VIEWPORT_REL);
                }
            }
        }
    }
}

BOOL BuildingClass::Mark(MarkType mark)
{
    SmudgeType smudge = SMUDGE_NONE;

    if (!TechnoClass::Mark(mark)) {
        return false;
    }

    cell_t cellnum = Get_Cell();

    switch (mark) {
        case MARK_REMOVE: {
            g_Map.Pick_Up(cellnum, this);

            if (Class_Of().Bib_And_Offset(smudge, cellnum)) {
                SmudgeClass *sptr = new SmudgeClass(smudge);

                if (sptr != nullptr) {
                    sptr->Disown(cellnum);
                    delete sptr;
                }
            }
            break;
        }

        case MARK_PUT: {
            if (Class_Of().Is_Wall()) {
                switch (What_Type()) {
                    case BUILDING_SBAG:
                        new OverlayClass(OVERLAY_SANDBAG, cellnum, Get_Owner_House()->What_Type());
                        break;

                    case BUILDING_CYCL:
                        new OverlayClass(OVERLAY_CYCLONE_FENCE, cellnum, Get_Owner_House()->What_Type());
                        break;

                    case BUILDING_BRIK:
                        new OverlayClass(OVERLAY_BRICK_WALL, cellnum, Get_Owner_House()->What_Type());
                        break;

                    case BUILDING_BARB:
                        new OverlayClass(OVERLAY_BARB_WIRE, cellnum, Get_Owner_House()->What_Type());
                        break;

                    case BUILDING_WOOD:
                        new OverlayClass(OVERLAY_WOOD_FENCE, cellnum, Get_Owner_House()->What_Type());
                        break;

                    case BUILDING_FENC:
                        new OverlayClass(OVERLAY_FENCE, cellnum, Get_Owner_House()->What_Type());
                        break;

                    default:
                        break;
                }

                Transmit_Message(RADIO_OVER_AND_OUT);
                // walls are placed as overlays, so the building is deleted
                delete this;

            } else {
                if (Can_Enter_Cell(cellnum) != MOVE_OK) {
                    return false;
                }

                cell_t smudge_cell = cellnum;

                if (Class_Of().Bib_And_Offset(smudge, smudge_cell)) {
                    new SmudgeClass(smudge,
                        Cell_To_Coord(smudge_cell),
                        Class_Of().Base_Normal() ? Get_Owner_House()->What_Type() : HOUSES_NONE);
                }

                g_Map.Place_Down(cellnum, this);
            }
            break;
        }

        case MARK_REDRAW:
            g_Map.Refresh_Cells(cellnum, Overlap_List(true));
            break;

        default:
            g_Map.Refresh_Cells(cellnum, Overlap_List());
            g_Map.Refresh_Cells(cellnum, Occupy_List());
            break;
    }
    return true;
}

/**
 *
 *
 */
void BuildingClass::Active_Click_With(ActionType action, ObjectClass *object)
{
    switch (action) {
        case ACTION_ATTACK:
            if (object != nullptr) {
                Player_Assign_Mission(MISSION_ATTACK, object->As_Target());
            }
            break;

        case ACTION_SELF:
            if (Class_Of().Factory_Type() != RTTI_NONE) {
                GameEventClass event(GameEventClass::EVENT_PRIMARY, object);
                g_OutgoingEvents.Add(event);
            }
            break;

        default:
            break;
    }
}

/**
 *
 *
 */
void BuildingClass::Active_Click_With(ActionType action, cell_t cellnum)
{
    switch (action) {
        case ACTION_ATTACK:
            Player_Assign_Mission(MISSION_ATTACK, ::As_Target(cellnum));
            break;

        case ACTION_MOVE:
            if (What_Type() == BUILDING_FACT) {
                GameEventClass archev(GameEventClass::EVENT_ARCHIVE, TargetClass(this), TargetClass(cellnum));
                g_OutgoingEvents.Add(archev);

                GameEventClass sellev(GameEventClass::EVENT_SELL, this);
                g_OutgoingEvents.Add(sellev);
            }
            break;
        default:
            break;
    }
}

DamageResultType BuildingClass::Take_Damage(int &damage, int a2, WarheadType warhead, TechnoClass *object, BOOL a5)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x00456D18, DamageResultType, BuildingClass *, int &, int, WarheadType, TechnoClass *, BOOL);
    return func(this, damage, a2, warhead, object, a5);
#else
    return DAMAGE_NONE;
#endif
}

/**
 *
 *
 */
void BuildingClass::Fire_Out()
{
    // empty
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
                captainslog_debug("BuildingClass::Value - Unhandled fake!");
                break;
        }
    }
    return TechnoClass::Value();
}

RadioMessageType BuildingClass::Receive_Message(RadioClass *radio, RadioMessageType message, target_t &target)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x00454980, RadioMessageType, BuildingClass *, RadioClass *, RadioMessageType, target_t &);
    return func(this, radio, message, target);
#else
    return RADIO_NONE;
#endif
}

BOOL BuildingClass::Revealed(HouseClass *house)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045DC30, BOOL, BuildingClass *, HouseClass *);
    return func(this, house);
#else
    return 0;
#endif
}

/**
 *
 *
 */
void BuildingClass::Repair(int mode)
{
    switch (mode) {
        case -1:
            m_IsRepairing = !m_IsRepairing;
            break;

        case 0:
            if (!m_IsRepairing) {
                return;
            }

            m_IsRepairing = false;
            break;

        case 1:
            if (m_IsRepairing) {
                return;
            }

            m_IsRepairing = true;
            break;

        default:
            break;
    }

    VocType voc = VOC_NONE;

    if (m_IsRepairing) {
        if (m_Health == Class_Of().Get_Strength()) {
            voc = VOC_SCOLDY1;
        } else {
            voc = VOC_RAMENU1;

            if (m_OwnerHouse->Player_Has_Control()) {
                Clicked_As_Target(7);
            }

            m_Bit32 = true;
        }
    } else {
        voc = VOC_RAMENU1;
    }

    if (m_OwnerHouse->Player_Has_Control()) {
        Sound_Effect(voc, m_Coord);
    }
}

/**
 *
 *
 */
void BuildingClass::Sell_Back(int mode)
{
    if (Class_Of().Get_Buildup_Data()) {
        bool to_deconstruct = false;

        switch (mode) {
            case -1:
                to_deconstruct = m_Mission != MISSION_DECONSTRUCTION;
                break;
            case 0:
                if (m_Mission != MISSION_DECONSTRUCTION) {
                    return;
                }
                break;
            case 1:
                if (m_Mission == MISSION_DECONSTRUCTION || m_Bit64) {
                    return;
                }
                to_deconstruct = true;
                break;
            default:
                break;
        }

        if (to_deconstruct) {
            Assign_Mission(MISSION_DECONSTRUCTION);
            Commence();

            if (m_OwnerHouse->Player_Has_Control()) {
                Clicked_As_Target(7);
            }
        }

        if (m_OwnerHouse->Player_Has_Control()) {
            Sound_Effect(VOC_RAMENU1);
        }
    }
}

int BuildingClass::Mission_Attack()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045C860, int, BuildingClass *);
    return func(this);
#else
    return 0;
#endif
}

int BuildingClass::Mission_Guard()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045BB88, int, BuildingClass *);
    return func(this);
#else
    return 0;
#endif
}

int BuildingClass::Mission_Harvest()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045CD84, int, BuildingClass *);
    return func(this);
#else
    return 0;
#endif
}

int BuildingClass::Mission_Unload()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045DFB8, int, BuildingClass *);
    return func(this);
#else
    return 0;
#endif
}

int BuildingClass::Mission_Construction()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045BE04, int, BuildingClass *);
    return func(this);
#else
    return 0;
#endif
}

int BuildingClass::Mission_Deconstruction()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045BF54, int, BuildingClass *);
    return func(this);
#else
    return 0;
#endif
}

int BuildingClass::Mission_Repair()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045CEE0, int, BuildingClass *);
    return func(this);
#else
    return 0;
#endif
}

int BuildingClass::Mission_Missile()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045D700, int, BuildingClass *);
    return func(this);
#else
    return 0;
#endif
}

int BuildingClass::How_Many_Survivors() const
{
    if (m_Bit128 || !Class_Of().Is_Crewed()) {
        return 0;
    }

    int cost = InfantryTypeClass::As_Reference(INFANTRY_E1).Raw_Cost();

    if (cost == 0) {
        return 0;
    }

    if (m_IsCaptured) {
        cost *= 2;
    }

    int calc = Class_Of().Raw_Cost() * g_Rule.Survivor_Rate();

    return std::clamp(calc / cost, 1, 5);
}

/**
 *
 *
 */
DirType BuildingClass::Turret_Facing() const
{
    if (Class_Of().Is_Turret_Equipped() || !Target_Legal(m_TarCom)) {
        return m_Facing.Get_Current();
    }

    return Direction_To_Target(m_TarCom);
}

/**
 *
 *
 */
cell_t BuildingClass::Find_Exit_Cell(TechnoClass *object) const
{
    cell_t cell = Get_Cell();

    const int16_t *list = Class_Of().Exit_List();
    if (list != nullptr) {
        while (list[0] != 32767) {
            cell_t offset_cell = list[0] + cell;
            ++list;

            if (g_Map.In_Radar(offset_cell) && object->Can_Enter_Cell(offset_cell) == MOVE_OK) {
                return offset_cell;
            }
        }
    } else {
        int width = Class_Of().Width();
        int height = Class_Of().Height(0);

        cell_t exit_cell;

        int j = -1;
        int i = -1;

        // TODO clean up fors, don't have good pseudo for the loop currently

        for (;; ++i) {
            if (i > width) {
                break;
            }

            exit_cell = (j << 7) + i + cell;

            if (g_Map.In_Radar(exit_cell) && object->Can_Enter_Cell(exit_cell) == MOVE_OK) {
                return exit_cell;
            }

            exit_cell = (height << 7) + i + cell;

            if (g_Map.In_Radar(exit_cell) && object->Can_Enter_Cell(exit_cell) == MOVE_OK) {
                return exit_cell;
            }
        }

        i = -1;
        j = -1;
        for (;; ++j) {
            if (j > height) {
                break;
            }

            exit_cell = i + cell + (j << 7);

            if (g_Map.In_Radar(exit_cell) && object->Can_Enter_Cell(exit_cell) == MOVE_OK) {
                return exit_cell;
            }

            exit_cell = width + cell + (j << 7);

            if (g_Map.In_Radar(exit_cell) && object->Can_Enter_Cell(exit_cell) == MOVE_OK) {
                return exit_cell;
            }
        }
    }

    return 0;
}

/**
 *
 *
 */
DirType BuildingClass::Fire_Direction() const
{
    if (Class_Of().Is_Turret_Equipped()) {
        return m_Facing.Get_Current();
    }

    return Direction_To_Target(m_TarCom);
}

InfantryType BuildingClass::Crew_Type() const
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045E420, InfantryType, const BuildingClass *);
    return func(this);
#else
    return INFANTRY_NONE;
#endif
}

/**
 *
 *
 */
int BuildingClass::Pip_Count() const
{
    return m_OwnerHouse->Ore_Fraction() * Class_Of().Max_Pips();
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

FireErrorType BuildingClass::Can_Fire(target_t target, WeaponSlotType weapon) const
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045A9DC, FireErrorType, const BuildingClass *, target_t, WeaponSlotType);
    return func(this, target, weapon);
#else
    return FIRE_NONE;
#endif
}

target_t BuildingClass::Greatest_Threat(ThreatType threat)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x00459AC0, target_t, BuildingClass *, ThreatType);
    return func(this, threat);
#else
    return 0;
#endif
}

/**
 *
 *
 */
void BuildingClass::Assign_Target(target_t target)
{
    // Anti-Air defenses are excluded from range checks?
    if (What_Type() != BUILDING_SAM && What_Type() != BUILDING_AGUN) {
        if (!In_Range(target)) {
            target = 0;
        }
    }

    TechnoClass::Assign_Target(target);
}

BOOL BuildingClass::Captured(HouseClass *house)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045ADD0, BOOL, BuildingClass *, HouseClass *);
    return func(this, house);
#else
    return false;
#endif
}

void BuildingClass::Enter_Idle_Mode(BOOL a1)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045DD94, void, BuildingClass *, BOOL);
    func(this, a1);
#endif
}

void BuildingClass::Grand_Opening(int a1)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x00459BBC, void, BuildingClass *, int);
    func(this, a1);
#endif
}

/**
 *
 *
 */
void BuildingClass::Update_Buildables()
{
    if (m_OwnerHouse == g_PlayerPtr && !m_InLimbo && m_PlayerAware) {
        TechnoTypeClass *ttptr;

        switch (Class_Of().Factory_Type()) {
            case RTTI_VESSELTYPE:
                for (VesselType v = VESSEL_FIRST; v < VESSEL_COUNT; ++v) {
                    ttptr = &VesselTypeClass::As_Reference(v);

                    if (g_PlayerPtr->Can_Build(ttptr, m_field_D5)) {
                        g_Map.Add(RTTI_VESSELTYPE, v);
                    }
                }
                break;

            case RTTI_BUILDINGTYPE:
                for (BuildingType b = BUILDING_FIRST; b < BUILDING_COUNT; ++b) {
                    ttptr = &BuildingTypeClass::As_Reference(b);

                    if (g_PlayerPtr->Can_Build(ttptr, m_field_D5)) {
                        g_Map.Add(RTTI_BUILDINGTYPE, b);
                    }
                }
                break;

            case RTTI_UNITTYPE:
                for (UnitType u = UNIT_FIRST; u < UNIT_COUNT; ++u) {
                    ttptr = &UnitTypeClass::As_Reference(u);

                    if (g_PlayerPtr->Can_Build(ttptr, m_field_D5)) {
                        g_Map.Add(RTTI_UNITTYPE, u);
                    }
                }
                break;

            case RTTI_INFANTRYTYPE:
                for (InfantryType i = INFANTRY_FIRST; i < INFANTRY_COUNT; ++i) {
                    ttptr = &InfantryTypeClass::As_Reference(i);

                    if (g_PlayerPtr->Can_Build(ttptr, m_field_D5)) {
                        if (reinterpret_cast<InfantryTypeClass *>(ttptr)->Is_Canine()) {
                            if (What_Type() == BUILDING_KENN) {
                                g_Map.Add(RTTI_INFANTRYTYPE, i);
                            }
                        } else if (What_Type() != BUILDING_KENN) {
                            g_Map.Add(RTTI_INFANTRYTYPE, i);
                        }
                    }
                }
                break;

            case RTTI_AIRCRAFTTYPE:
                for (AircraftType a = AIRCRAFT_FIRST; a < AIRCRAFT_COUNT; ++a) {
                    ttptr = &AircraftTypeClass::As_Reference(a);

                    if (g_PlayerPtr->Can_Build(ttptr, m_field_D5)) {
                        g_Map.Add(RTTI_AIRCRAFTTYPE, a);
                    }
                }
                break;

            default:
                return;
        }
    }
}

uint8_t *BuildingClass::Remap_Table() const
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045DF38, uint8_t *, const BuildingClass *);
    return func(this);
#else
    return 0;
#endif
}

int BuildingClass::Toggle_Primary()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045ABAC, int, BuildingClass *);
    return func(this);
#else
    return 0;
#endif
}

int BuildingClass::Shape_Number() const
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x00455618, int, const BuildingClass *);
    return func(this);
#else
    return 0;
#endif
}

void BuildingClass::Drop_Debris(target_t target)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x00458270, void, BuildingClass *, target_t);
    func(this, target);
#endif
}

void BuildingClass::Begin_Mode(BStateType state)
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045A794, void, BuildingClass *, BStateType);
    func(this, state);
#endif
}

/**
 *
 *
 */
int BuildingClass::Power_Output()
{
    if (Class_Of().Power_Output() > 0) {
        fixed_t power(m_field_EA, Class_Of().Get_Strength());
        return power * Class_Of().Power_Output();
    }
    return 0;
}

int BuildingClass::Flush_For_Placement(TechnoClass *techno, cell_t cellnum) const
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045E77C, cell_t, const BuildingClass *, TechnoClass *, cell_t);
    return func(this, techno, cellnum);
#else
    return 0;
#endif
}

cell_t BuildingClass::Check_Point(CheckPointType check) const
{
    int x = 6;
    int y = 5;

    cell_t cellnum = Center_Cell();

    switch (check) {
        case CHECKPOINT_0:
            x = 0;
            break;

        case CHECKPOINT_1:
            break;

        case CHECKPOINT_2:
            y = 0;
            break;
        default:
            break;
    }

    if (Cell_Get_X(cellnum) - g_Map.Get_Map_Cell_X() > g_Map.Get_Map_Cell_Width() / 2) {
        x = -x;
    }

    if ((signed int)(Cell_Get_Y(cellnum) - g_Map.Get_Map_Cell_Y()) > g_Map.Get_Map_Cell_Height() / 2) {
        y = -y;
    }

    return Cell_From_XY(x + Cell_Get_X(cellnum), y + Cell_Get_Y(cellnum));
}

void BuildingClass::Update_Radar_Spied()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045EC70, void, BuildingClass *);
    func(this);
#endif
}

void BuildingClass::Factory_AI()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045F2D8, void, BuildingClass *);
    func(this);
#endif
}

void BuildingClass::Rotation_AI()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045FAD0, void, BuildingClass *);
    func(this);
#endif
}

void BuildingClass::Charging_AI()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045FBF0, void, BuildingClass *);
    func(this);
#endif
}

void BuildingClass::Repair_AI()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x0045FE2C, void, BuildingClass *);
    func(this);
#endif
}

void BuildingClass::Animation_AI()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x00460314, void, BuildingClass *);
    func(this);
#endif
}

void BuildingClass::Remove_Gap_Effect()
{
#ifdef GAME_DLL
    DEFINE_CALL(func, 0x00460890, void, BuildingClass *);
    func(this);
#endif
}

void BuildingClass::Read_INI(GameINIClass &ini)
{
#ifdef GAME_DLL
    void (*func)(GameINIClass &) = reinterpret_cast<void (*)(GameINIClass &)>(0x0045ED5C);
    return func(ini);
#endif
}

void BuildingClass::Write_INI(GameINIClass &ini)
{
#ifdef GAME_DLL
    void (*func)(GameINIClass &) = reinterpret_cast<void (*)(GameINIClass &)>(0x0045F07C);
    return func(ini);
#endif
}
