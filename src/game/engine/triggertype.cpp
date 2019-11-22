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
#include "triggertype.h"
#include "callback.h"
#include "droplist.h"
#include "gadget.h"
#include "gamefile.h"
#include "gameini.h"
#include "gbuffer.h"
#include "globals.h"
#include "language.h"
#include "movie.h"
#include "quarry.h"
#include "super.h"
#include "tdroplist.h"
#include "textbtn.h"
#include "theme.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

using std::free;
using std::qsort;
using std::snprintf;
using std::sprintf;
using std::strcat;
using std::strtok;

#ifndef GAME_DLL
TFixedIHeapClass<TriggerTypeClass> g_TriggerTypes;
#endif

TriggerTypeClass::TriggerTypeClass() :
    AbstractTypeClass(RTTI_TRIGGERTYPE, g_TriggerTypes.ID(this), 0, "x"),
    m_State(STATE_VOLATILE),
    m_House(HOUSES_NONE),
    m_EventLinkage(EVLINK_SINGLE),
    m_ActionLinkage(ACTLINK_SINGLE)
{
}

/**
 * @address 0x0056D3F0
 */
void *TriggerTypeClass::operator new(size_t size)
{
    TriggerTypeClass *this_ptr = g_TriggerTypes.Alloc();

    if (this_ptr != nullptr) {
        this_ptr->m_IsActive = true;
    }

    return this_ptr;
}

/**
 * @address 0x0056D410
 */
void TriggerTypeClass::operator delete(void *ptr)
{
    TriggerTypeClass *this_ptr = static_cast<TriggerTypeClass *>(ptr);

    if (this_ptr != nullptr) {
        this_ptr->m_IsActive = false;
    }

    g_TriggerTypes.Free(this_ptr);
}

/**
 * Encodes pointers for serialisation.
 *
 * @address 0x004F9448
 */
void TriggerTypeClass::Code_Pointers()
{
    m_EventOne.Code_Pointers();
    m_EventTwo.Code_Pointers();
    m_ActionOne.Code_Pointers();
    m_ActionTwo.Code_Pointers();
}

/**
 * Decodes pointers for deserialisation.
 *
 * @address 0x004F9464
 */
void TriggerTypeClass::Decode_Pointers()
{
    m_EventOne.Decode_Pointers();
    m_EventTwo.Decode_Pointers();
    m_ActionOne.Decode_Pointers();
    m_ActionTwo.Decode_Pointers();
}

/**
 * Finds and existed Trigger instance or makes a new one from the TriggerTypeClass.
 */
TriggerClass *TriggerTypeClass::Find_Or_Make()
{
    // Find or make is a free function in original, but takes TriggerTypeClass so makes sense as a member.
#ifdef GAME_DLL
    TriggerClass *(*func)(TriggerTypeClass *) = reinterpret_cast<TriggerClass *(*)(TriggerTypeClass *)>(0x0056D248);
    return func(this);
#else
    return nullptr;
#endif
}

/**
 * Detaches this trigger type from the target.
 */
void TriggerTypeClass::Detach(target_t target, int unk)
{
    m_ActionOne.Detach(target);
    m_ActionTwo.Detach(target);
}

/**
 * Generates a bitfield that indicates what this TriggerType can attach to.
 */
AttachType TriggerTypeClass::Attaches_To()
{
    AttachType ret = m_EventOne.Attaches_To();

    if (m_EventLinkage != EVLINK_SINGLE) {
        ret |= m_EventTwo.Attaches_To();
    }

    return ret;
}

BOOL TriggerTypeClass::Edit()
{
    static const char *_perstext[] = { "Volatile", "Semi-persistent", "Persistent" };
    void *up_btn = GameMixFile::Retrieve("ebtn-up.shp");
    void *dn_btn = GameMixFile::Retrieve("ebtn-dn.shp");

    // Setup the event choice lists.
    char ev_one_buff[35] = { 0 };
    char ev_two_buff[35] = { 0 };
    TDropListClass<EventChoiceClass *> event_one_list(
        100, ev_one_buff, sizeof(ev_one_buff), TPF_3PT | TPF_SHADOW, 45, 65, 160, 40, up_btn, dn_btn);
    TDropListClass<EventChoiceClass *> event_two_list(
        101, ev_two_buff, sizeof(ev_two_buff), TPF_3PT | TPF_SHADOW, 45, 87, 160, 40, up_btn, dn_btn);

    for (int i = 0; i < TEVENT_COUNT; ++i) {
        event_one_list.Add_Item(&EventChoiceClass::s_EventChoices[i]);
        event_two_list.Add_Item(&EventChoiceClass::s_EventChoices[i]);
    }

    // Sort case insensitive alphabetically based on event name.
    qsort(event_one_list.Get_Entries(), sizeof(EventChoiceClass *), event_one_list.Count(), EventChoiceClass::Comp);
    qsort(event_two_list.Get_Entries(), sizeof(EventChoiceClass *), event_two_list.Count(), EventChoiceClass::Comp);
    event_one_list.Set_Selected_Index(&EventChoiceClass::s_EventChoices[m_EventOne.m_Type]);
    event_two_list.Set_Selected_Index(&EventChoiceClass::s_EventChoices[m_EventTwo.m_Type]);

    // Setup the action choice lists
    char act_one_buff[35] = { 0 };
    char act_two_buff[35] = { 0 };
    TDropListClass<ActionChoiceClass *> act_one_list(
        100, act_one_buff, sizeof(act_one_buff), TPF_3PT | TPF_SHADOW, 45, 120, 160, 40, up_btn, dn_btn);
    TDropListClass<ActionChoiceClass *> act_two_list(
        101, act_two_buff, sizeof(act_two_buff), TPF_3PT | TPF_SHADOW, 45, 142, 160, 40, up_btn, dn_btn);

    for (int i = 0; i < TEVENT_COUNT; ++i) {
        act_one_list.Add_Item(&ActionChoiceClass::s_ActionChoices[i]);
        act_two_list.Add_Item(&ActionChoiceClass::s_ActionChoices[i]);
    }

    // Sort case insensitive alphabetically based on action name.
    qsort(act_one_list.Get_Entries(), sizeof(ActionChoiceClass *), act_one_list.Count(), ActionChoiceClass::Comp);
    qsort(act_two_list.Get_Entries(), sizeof(ActionChoiceClass *), act_two_list.Count(), ActionChoiceClass::Comp);
    act_one_list.Set_Selected_Index(&ActionChoiceClass::s_ActionChoices[m_ActionOne.m_Type]);
    act_two_list.Set_Selected_Index(&ActionChoiceClass::s_ActionChoices[m_ActionTwo.m_Type]);

    // *** Editors for waypoints
    // Setup value editor for event one.
    char ev_one_wp_buf[3] = { 'A' };
    EditClass ev_one_wp_edit(
        115, ev_one_wp_buf, sizeof(ev_one_wp_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 65, 95, 9, EDIT_TEXT);
    if (m_EventOne.Event_Needs() == NEED_WAYPOINT) {
        if (m_EventOne.m_IntegerValue > 26) {
            sprintf(ev_one_wp_edit.Get_Text(),
                "%c%c",
                m_EventOne.m_IntegerValue / 26 + '@',
                m_EventOne.m_IntegerValue % 26 + 'A');
        } else {
            sprintf(ev_one_wp_edit.Get_Text(), "%c", m_EventOne.m_IntegerValue + 'A');
        }
    }

    // Setup value editor for event two.
    char ev_two_wp_buf[3] = { 'A' };
    EditClass ev_two_wp_edit(
        116, ev_two_wp_buf, sizeof(ev_two_wp_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 87, 95, 9, EDIT_TEXT);
    if (m_EventTwo.Event_Needs() == NEED_WAYPOINT) {
        if (m_EventTwo.m_IntegerValue > 26) {
            sprintf(ev_two_wp_edit.Get_Text(),
                "%c%c",
                m_EventTwo.m_IntegerValue / 26 + '@',
                m_EventTwo.m_IntegerValue % 26 + 'A');
        } else {
            sprintf(ev_two_wp_edit.Get_Text(), "%c", m_EventTwo.m_IntegerValue + 'A');
        }
    }

    // Setup value editor for action one.
    char act_one_wp_buf[3] = { 'A' };
    EditClass act_one_wp_edit(
        117, act_one_wp_buf, sizeof(act_one_wp_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 120, 95, 9, EDIT_TEXT);
    if (m_ActionOne.Action_Needs() == NEED_WAYPOINT) {
        if (m_ActionOne.m_IntegerValue > 26) {
            sprintf(act_one_wp_edit.Get_Text(),
                "%c%c",
                m_ActionOne.m_IntegerValue / 26 + '@',
                m_ActionOne.m_IntegerValue % 26 + 'A');
        } else {
            sprintf(act_one_wp_edit.Get_Text(), "%c", m_ActionOne.m_IntegerValue + 'A');
        }
    }

    // Setup value editor for action two.
    char act_two_wp_buf[3] = { 'A' };
    EditClass act_two_wp_edit(
        118, act_two_wp_buf, sizeof(act_two_wp_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 142, 95, 9, EDIT_TEXT);
    if (m_ActionTwo.Action_Needs() == NEED_WAYPOINT) {
        if (m_ActionTwo.m_IntegerValue > 26) {
            sprintf(act_two_wp_edit.Get_Text(),
                "%c%c",
                m_ActionTwo.m_IntegerValue / 26 + '@',
                m_ActionTwo.m_IntegerValue % 26 + 'A');
        } else {
            sprintf(act_two_wp_edit.Get_Text(), "%c", m_ActionTwo.m_IntegerValue + 'A');
        }
    }

    // *** Editors for numbers
    // Setup value editor for event one.
    char ev_one_num_buf[10] = { 0 };
    EditClass ev_one_num_edit(
        125, ev_one_num_buf, sizeof(ev_one_num_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 65, 95, 9, EDIT_TEXT);
    if (m_EventOne.Event_Needs() == NEED_NUMBER || m_EventOne.Event_Needs() == NEED_14) {
        sprintf(ev_one_num_edit.Get_Text(), "%d", m_EventOne.m_IntegerValue);
    }

    // Setup value editor for event two.
    char ev_two_num_buf[10] = { 0 };
    EditClass ev_two_num_edit(
        126, ev_two_num_buf, sizeof(ev_two_num_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 87, 95, 9, EDIT_TEXT);
    if (m_EventTwo.Event_Needs() == NEED_NUMBER || m_EventTwo.Event_Needs() == NEED_14) {
        sprintf(ev_two_num_edit.Get_Text(), "%d", m_EventTwo.m_IntegerValue);
    }

    // Setup value editor for action one.
    char act_one_num_buf[10] = { 0 };
    EditClass act_one_num_edit(
        127, act_one_num_buf, sizeof(act_one_num_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 120, 95, 9, EDIT_TEXT);
    if (m_ActionOne.Action_Needs() == NEED_NUMBER) {
        sprintf(act_one_num_edit.Get_Text(), "%d", m_ActionOne.m_IntegerValue);
    }

    // Setup value editor for action two.
    char act_two_num_buf[10] = { 0 };
    EditClass act_two_num_edit(
        128, act_two_num_buf, sizeof(act_two_num_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 142, 95, 9, EDIT_TEXT);
    if (m_ActionTwo.Action_Needs() == NEED_NUMBER) {
        sprintf(act_two_num_edit.Get_Text(), "%d", m_ActionTwo.m_IntegerValue);
    }

    // *** Editors for teams
    // Setup value editor for event one.
    char ev_one_team_buf[10] = { 0 };
    DropListClass ev_one_team_edit(
        137, ev_one_team_buf, sizeof(ev_one_team_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 65, 95, 40, up_btn, dn_btn);

    // Setup value editor for event two.
    char ev_two_team_buf[10] = { 0 };
    DropListClass ev_two_team_edit(
        138, ev_two_team_buf, sizeof(ev_two_team_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 87, 95, 40, up_btn, dn_btn);

    // Setup value editor for action one.
    char act_one_team_buf[10] = { 0 };
    DropListClass act_one_team_edit(
        139, act_one_team_buf, sizeof(act_one_team_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 120, 95, 40, up_btn, dn_btn);

    // Setup value editor for action two.
    char act_two_team_buf[10] = { 0 };
    DropListClass act_two_team_edit(
        140, act_two_team_buf, sizeof(act_two_team_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 142, 95, 40, up_btn, dn_btn);

    // Populate the lists.
    for (int i = 0; i < g_TeamTypes.Count(); ++i) {
        ev_one_team_edit.Add_Item(g_TeamTypes[i].Get_Name());
        ev_two_team_edit.Add_Item(g_TeamTypes[i].Get_Name());
        act_one_team_edit.Add_Item(g_TeamTypes[i].Get_Name());
        act_two_team_edit.Add_Item(g_TeamTypes[i].Get_Name());
    }

    // Set selected index if any.
    if (m_EventOne.m_TeamType == nullptr) {
        ev_one_team_edit.Set_Selected_Index(nullptr);
    } else {
        ev_one_team_edit.Set_Selected_Index(m_EventOne.m_TeamType->Get_Name());
    }

    if (m_EventTwo.m_TeamType == nullptr) {
        ev_two_team_edit.Set_Selected_Index(nullptr);
    } else {
        ev_two_team_edit.Set_Selected_Index(m_EventTwo.m_TeamType->Get_Name());
    }

    if (m_ActionOne.m_TeamType == nullptr) {
        act_one_team_edit.Set_Selected_Index(nullptr);
    } else {
        act_one_team_edit.Set_Selected_Index(m_ActionOne.m_TeamType->Get_Name());
    }

    if (m_ActionTwo.m_TeamType == nullptr) {
        act_two_team_edit.Set_Selected_Index(nullptr);
    } else {
        act_two_team_edit.Set_Selected_Index(m_ActionTwo.m_TeamType->Get_Name());
    }

    // *** Editors for triggers
    // Setup value editor for action one.
    char act_one_trig_buf[10] = { 0 };
    DropListClass act_one_trig_edit(
        141, act_one_trig_buf, sizeof(act_one_trig_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 120, 95, 40, up_btn, dn_btn);

    // Setup value editor for action two.
    char act_two_trig_buf[10] = { 0 };
    DropListClass act_two_trig_edit(
        142, act_two_trig_buf, sizeof(act_two_trig_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 142, 95, 40, up_btn, dn_btn);

    // Populate the lists.
    for (int i = 0; i < g_TriggerTypes.Count(); ++i) {
        act_one_trig_edit.Add_Item(g_TriggerTypes[i].Get_Name());
        act_two_trig_edit.Add_Item(g_TriggerTypes[i].Get_Name());
    }

    if (m_ActionOne.m_TriggerType == nullptr) {
        act_one_trig_edit.Set_Selected_Index(0);
    } else {
        act_one_trig_edit.Set_Selected_Index(m_ActionOne.m_TriggerType->Get_Name());
    }

    if (m_ActionTwo.m_TriggerType == nullptr) {
        act_two_trig_edit.Set_Selected_Index(0);
    } else {
        act_two_trig_edit.Set_Selected_Index(m_ActionTwo.m_TriggerType->Get_Name());
    }

    // *** Editors for bools
    // Setup value editor for action one.
    char act_one_bool_buf[10] = { 0 };
    DropListClass act_one_bool_edit(
        123, act_one_bool_buf, sizeof(act_one_bool_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 120, 95, 40, up_btn, dn_btn);

    // Setup value editor for action two.
    char act_two_bool_buf[10] = { 0 };
    DropListClass act_two_bool_edit(
        124, act_two_bool_buf, sizeof(act_two_bool_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 142, 95, 40, up_btn, dn_btn);

    // Populate the lists.
    act_one_bool_edit.Add_Item("OFF");
    act_one_bool_edit.Add_Item("ON");
    act_two_bool_edit.Add_Item("OFF");
    act_two_bool_edit.Add_Item("ON");
    act_one_bool_edit.Set_Selected_Index(m_ActionOne.m_IntegerValue);
    act_two_bool_edit.Set_Selected_Index(m_ActionTwo.m_IntegerValue);

    // *** Editors for themes
    // Setup value editor for action one.
    char act_one_theme_buf[35] = { 0 };
    DropListClass act_one_theme_edit(
        107, act_one_theme_buf, sizeof(act_one_theme_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 120, 95, 40, up_btn, dn_btn);

    // Setup value editor for action two.
    char act_two_theme_buf[35] = { 0 };
    DropListClass act_two_theme_edit(
        108, act_two_theme_buf, sizeof(act_two_theme_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 142, 95, 40, up_btn, dn_btn);

    // Populate the lists.
    for (ThemeType i = THEME_FIRST; i < THEME_COUNT; ++i) {
        act_one_theme_edit.Add_Item(Theme.Full_Name(i));
        act_two_theme_edit.Add_Item(Theme.Full_Name(i));
    }

    if (m_ActionOne.Action_Needs() != NEED_THEME) {
        act_one_theme_edit.Set_Selected_Index(0);
    } else {
        act_one_theme_edit.Set_Selected_Index(ThemeType(m_ActionOne.m_IntegerValue));
    }

    if (m_ActionOne.Action_Needs() != NEED_THEME) {
        act_two_theme_edit.Set_Selected_Index(0);
    } else {
        act_two_theme_edit.Set_Selected_Index(ThemeType(m_ActionOne.m_IntegerValue));
    }

    // *** Editors for movies
    // Setup value editor for action one.
    char act_one_movie_buf[35] = { 0 };
    DropListClass act_one_movie_edit(
        109, act_one_movie_buf, sizeof(act_one_movie_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 120, 95, 40, up_btn, dn_btn);

    // Setup value editor for action two.
    char act_two_movie_buf[35] = { 0 };
    DropListClass act_two_movie_edit(
        110, act_two_movie_buf, sizeof(act_two_movie_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 142, 95, 40, up_btn, dn_btn);

    // Populate the lists.
    for (MovieType i = MOVIE_FIRST; i < MOVIE_COUNT; ++i) {
        act_one_movie_edit.Add_Item(Name_From_Movie(i));
        act_two_movie_edit.Add_Item(Name_From_Movie(i));
    }

    if (m_ActionOne.Action_Needs() != NEED_MOVIE) {
        act_one_movie_edit.Set_Selected_Index(0);
    } else {
        act_one_movie_edit.Set_Selected_Index(MovieType(m_ActionOne.m_IntegerValue));
    }

    if (m_ActionOne.Action_Needs() != NEED_MOVIE) {
        act_two_movie_edit.Set_Selected_Index(0);
    } else {
        act_two_movie_edit.Set_Selected_Index(MovieType(m_ActionOne.m_IntegerValue));
    }

    // *** Editors for voc
    // Setup value editor for action one.
    char act_one_voc_buf[35] = { 0 };
    DropListClass act_one_voc_edit(
        111, act_one_voc_buf, sizeof(act_one_voc_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 120, 95, 40, up_btn, dn_btn);

    // Setup value editor for action two.
    char act_two_voc_buf[35] = { 0 };
    DropListClass act_two_voc_edit(
        112, act_two_voc_buf, sizeof(act_two_voc_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 142, 95, 40, up_btn, dn_btn);

    // Populate the lists.
    for (VocType i = VOC_FIRST; i < VOC_COUNT; ++i) {
        act_one_voc_edit.Add_Item(Name_From_Voc(i));
        act_two_voc_edit.Add_Item(Name_From_Voc(i));
    }

    if (m_ActionOne.Action_Needs() != NEED_SOUND) {
        act_one_voc_edit.Set_Selected_Index(0);
    } else {
        act_one_voc_edit.Set_Selected_Index(VocType(m_ActionOne.m_IntegerValue));
    }

    if (m_ActionOne.Action_Needs() != NEED_SOUND) {
        act_two_voc_edit.Set_Selected_Index(0);
    } else {
        act_two_voc_edit.Set_Selected_Index(VocType(m_ActionOne.m_IntegerValue));
    }

    // *** Editors for vox
    // Setup value editor for action one.
    char act_one_vox_buf[35] = { 0 };
    DropListClass act_one_vox_edit(
        105, act_one_vox_buf, sizeof(act_one_vox_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 120, 95, 40, up_btn, dn_btn);

    // Setup value editor for action two.
    char act_two_vox_buf[35] = { 0 };
    DropListClass act_two_vox_edit(
        106, act_two_vox_buf, sizeof(act_two_vox_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 142, 95, 40, up_btn, dn_btn);

    // Populate the lists.
    for (VoxType i = VOX_FIRST; i < VOX_COUNT; ++i) {
        act_one_vox_edit.Add_Item(Name_From_Vox(i));
        act_two_vox_edit.Add_Item(Name_From_Vox(i));
    }

    if (m_ActionOne.Action_Needs() != NEED_SPEECH) {
        act_one_vox_edit.Set_Selected_Index(0);
    } else {
        act_one_vox_edit.Set_Selected_Index(VoxType(m_ActionOne.m_IntegerValue));
    }

    if (m_ActionOne.Action_Needs() != NEED_SPEECH) {
        act_two_vox_edit.Set_Selected_Index(0);
    } else {
        act_two_vox_edit.Set_Selected_Index(VoxType(m_ActionOne.m_IntegerValue));
    }

    // *** Editors for Buildings
    // Setup value editor for event one.
    char ev_one_build_buf[35] = { 0 };
    DropListClass ev_one_build_edit(
        129, ev_one_build_buf, sizeof(ev_one_build_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 65, 95, 40, up_btn, dn_btn);

    // Setup value editor for event two.
    char ev_two_build_buf[35] = { 0 };
    DropListClass ev_two_build_edit(
        130, ev_two_build_buf, sizeof(ev_two_build_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 87, 95, 40, up_btn, dn_btn);

    // Populate the lists.
    for (BuildingType i = BUILDING_FIRST; i < BUILDING_COUNT; ++i) {
        ev_one_build_edit.Add_Item(Text_String(BuildingTypeClass::As_Reference(i).Full_Name()));
        ev_two_build_edit.Add_Item(Text_String(BuildingTypeClass::As_Reference(i).Full_Name()));
    }

    if (m_EventOne.Event_Needs() != NEED_BUILDING) {
        ev_one_build_edit.Set_Selected_Index(0);
    } else {
        ev_one_build_edit.Set_Selected_Index(BuildingType(m_EventOne.m_IntegerValue));
    }

    if (m_EventOne.Event_Needs() != NEED_BUILDING) {
        ev_two_build_edit.Set_Selected_Index(0);
    } else {
        ev_two_build_edit.Set_Selected_Index(BuildingType(m_EventOne.m_IntegerValue));
    }

    // *** Editors for Infantry
    // Setup value editor for event one.
    char ev_one_inf_buf[35] = { 0 };
    DropListClass ev_one_inf_edit(
        131, ev_one_inf_buf, sizeof(ev_one_inf_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 65, 95, 40, up_btn, dn_btn);

    // Setup value editor for event two.
    char ev_two_inf_buf[35] = { 0 };
    DropListClass ev_two_inf_edit(
        132, ev_two_inf_buf, sizeof(ev_two_inf_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 87, 95, 40, up_btn, dn_btn);

    // Populate the lists.
    for (InfantryType i = INFANTRY_FIRST; i < INFANTRY_COUNT; ++i) {
        ev_one_inf_edit.Add_Item(Text_String(InfantryTypeClass::As_Reference(i).Full_Name()));
        ev_two_inf_edit.Add_Item(Text_String(InfantryTypeClass::As_Reference(i).Full_Name()));
    }

    if (m_EventOne.Event_Needs() != NEED_INFANTRY) {
        ev_one_inf_edit.Set_Selected_Index(0);
    } else {
        ev_one_inf_edit.Set_Selected_Index(InfantryType(m_EventOne.m_IntegerValue));
    }

    if (m_EventOne.Event_Needs() != NEED_INFANTRY) {
        ev_two_inf_edit.Set_Selected_Index(0);
    } else {
        ev_two_inf_edit.Set_Selected_Index(InfantryType(m_EventOne.m_IntegerValue));
    }

    // *** Editors for Aircraft
    // Setup value editor for event one.
    char ev_one_air_buf[35] = { 0 };
    DropListClass ev_one_air_edit(
        133, ev_one_air_buf, sizeof(ev_one_air_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 65, 95, 40, up_btn, dn_btn);

    // Setup value editor for event two.
    char ev_two_air_buf[35] = { 0 };
    DropListClass ev_two_air_edit(
        134, ev_two_air_buf, sizeof(ev_two_air_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 87, 95, 40, up_btn, dn_btn);

    // Populate the lists.
    for (AircraftType i = AIRCRAFT_FIRST; i < AIRCRAFT_COUNT; ++i) {
        ev_one_air_edit.Add_Item(Text_String(AircraftTypeClass::As_Reference(i).Full_Name()));
        ev_two_air_edit.Add_Item(Text_String(AircraftTypeClass::As_Reference(i).Full_Name()));
    }

    if (m_EventOne.Event_Needs() != NEED_AIRCRAFT) {
        ev_one_air_edit.Set_Selected_Index(0);
    } else {
        ev_one_air_edit.Set_Selected_Index(AircraftType(m_EventOne.m_IntegerValue));
    }

    if (m_EventOne.Event_Needs() != NEED_AIRCRAFT) {
        ev_two_air_edit.Set_Selected_Index(0);
    } else {
        ev_two_air_edit.Set_Selected_Index(AircraftType(m_EventOne.m_IntegerValue));
    }

    // *** Editors for Units
    // Setup value editor for event one.
    char ev_one_unit_buf[35] = { 0 };
    DropListClass ev_one_unit_edit(
        135, ev_one_air_buf, sizeof(ev_one_unit_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 65, 95, 40, up_btn, dn_btn);

    // Setup value editor for event two.
    char ev_two_unit_buf[35] = { 0 };
    DropListClass ev_two_unit_edit(
        136, ev_two_air_buf, sizeof(ev_two_unit_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 87, 95, 40, up_btn, dn_btn);

    // Populate the lists.
    for (UnitType i = UNIT_FIRST; i < UNIT_COUNT; ++i) {
        ev_one_unit_edit.Add_Item(Text_String(UnitTypeClass::As_Reference(i).Full_Name()));
        ev_two_unit_edit.Add_Item(Text_String(UnitTypeClass::As_Reference(i).Full_Name()));
    }

    if (m_EventOne.Event_Needs() != NEED_UNIT) {
        ev_one_unit_edit.Set_Selected_Index(0);
    } else {
        ev_one_unit_edit.Set_Selected_Index(UnitType(m_EventOne.m_IntegerValue));
    }

    if (m_EventTwo.Event_Needs() != NEED_UNIT) {
        ev_two_unit_edit.Set_Selected_Index(0);
    } else {
        ev_two_unit_edit.Set_Selected_Index(UnitType(m_EventTwo.m_IntegerValue));
    }

    // *** Editors for Houses

    // Setup value editor for event one.
    char ev_one_house_buf[35] = { 0 };
    DropListClass ev_one_house_edit(
        119, ev_one_house_buf, sizeof(ev_one_house_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 65, 95, 40, up_btn, dn_btn);

    // Setup value editor for event two.
    char ev_two_house_buf[35] = { 0 };
    DropListClass ev_two_house_edit(
        120, ev_two_house_buf, sizeof(ev_two_house_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 87, 95, 40, up_btn, dn_btn);

    // Setup value editor for action one.
    char act_one_house_buf[35] = { 0 };
    DropListClass act_one_house_edit(
        121, act_one_house_buf, sizeof(act_one_house_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 120, 95, 40, up_btn, dn_btn);

    // Setup value editor for action two.
    char act_two_house_buf[35] = { 0 };
    DropListClass act_two_house_edit(
        122, act_two_house_buf, sizeof(act_two_house_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 142, 95, 40, up_btn, dn_btn);

    // Populate the lists.
    for (HousesType i = HOUSES_FIRST; i < HOUSES_COUNT; ++i) {
        ev_one_house_edit.Add_Item(HouseTypeClass::As_Reference(i).Get_Name());
        ev_two_house_edit.Add_Item(HouseTypeClass::As_Reference(i).Get_Name());
        act_one_house_edit.Add_Item(HouseTypeClass::As_Reference(i).Get_Name());
        act_two_house_edit.Add_Item(HouseTypeClass::As_Reference(i).Get_Name());
    }

    // Set selected index if any.
    if (m_EventOne.Event_Needs() != NEED_HOUSE) {
        ev_one_house_edit.Set_Selected_Index(0);
    } else {
        ev_one_house_edit.Set_Selected_Index(HousesType(m_EventOne.m_IntegerValue));
    }

    if (m_EventTwo.Event_Needs() != NEED_HOUSE) {
        ev_two_house_edit.Set_Selected_Index(0);
    } else {
        ev_two_house_edit.Set_Selected_Index(HousesType(m_EventTwo.m_IntegerValue));
    }

    if (m_ActionOne.Action_Needs() != NEED_HOUSE) {
        act_one_house_edit.Set_Selected_Index(0);
    } else {
        act_one_house_edit.Set_Selected_Index(HousesType(m_ActionOne.m_IntegerValue));
    }

    if (m_ActionTwo.Action_Needs() != NEED_HOUSE) {
        act_two_house_edit.Set_Selected_Index(0);
    } else {
        act_two_house_edit.Set_Selected_Index(HousesType(m_ActionTwo.m_IntegerValue));
    }

    // *** Editors for Specials

    // Setup value editor for action one.
    char act_one_spec_buf[35] = { 0 };
    DropListClass act_one_spec_edit(
        113, act_one_spec_buf, sizeof(act_one_spec_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 120, 95, 40, up_btn, dn_btn);

    // Setup value editor for action two.
    char act_two_spec_buf[35] = { 0 };
    DropListClass act_two_spec_edit(
        114, act_two_spec_buf, sizeof(act_two_spec_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 142, 95, 40, up_btn, dn_btn);

    // Populate the lists.
    for (SpecialWeaponType i = SPECIAL_FIRST; i < SPECIAL_COUNT; ++i) {
        act_one_spec_edit.Add_Item(SuperClass::Name_From(i));
        act_two_spec_edit.Add_Item(SuperClass::Name_From(i));
    }

    if (m_ActionOne.m_IntegerValue >= SPECIAL_COUNT) {
        act_one_spec_edit.Set_Selected_Index(0);
    } else {
        act_one_spec_edit.Set_Selected_Index(SpecialWeaponType(m_EventOne.m_IntegerValue));
    }

    if (m_ActionTwo.m_IntegerValue >= SPECIAL_COUNT) {
        act_two_spec_edit.Set_Selected_Index(0);
    } else {
        act_two_spec_edit.Set_Selected_Index(SpecialWeaponType(m_EventTwo.m_IntegerValue));
    }

    // *** Editors for Quarries
    // Setup value editor for action one.
    char act_one_quarry_buf[35] = { 0 };
    DropListClass act_one_quarry_edit(
        113, act_one_quarry_buf, sizeof(act_one_quarry_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 120, 95, 40, up_btn, dn_btn);

    // Setup value editor for action two.
    char act_two_quarry_buf[35] = { 0 };
    DropListClass act_two_quarry_edit(
        114, act_two_quarry_buf, sizeof(act_two_quarry_buf), TPF_EDITOR | TPF_NOSHADOW, 225, 142, 95, 40, up_btn, dn_btn);

    // Populate the lists.
    for (QuarryType i = QUARRY_FIRST; i < QUARRY_COUNT; ++i) {
        act_one_quarry_edit.Add_Item(Name_From_Quarry(i));
        act_two_quarry_edit.Add_Item(Name_From_Quarry(i));
    }

    if (m_ActionOne.m_IntegerValue >= QUARRY_COUNT) {
        act_one_quarry_edit.Set_Selected_Index(0);
    } else {
        act_one_quarry_edit.Set_Selected_Index(QuarryType(m_EventOne.m_IntegerValue));
    }

    if (m_ActionTwo.m_IntegerValue >= QUARRY_COUNT) {
        act_two_quarry_edit.Set_Selected_Index(0);
    } else {
        act_two_quarry_edit.Set_Selected_Index(QuarryType(m_EventTwo.m_IntegerValue));
    }

    // *** Set trigger owner
    // Edit box to enter trigger owner house
    char owner_buf[5] = { 0 };
    EditClass owner_edit(
        104, owner_buf, sizeof(owner_buf), TPF_EDITOR | TPF_NOSHADOW, 40, 30, 40, 9, EDIT_TEXT | EDIT_NUMS | EDIT_SYMS);
    strlcpy(owner_buf, m_Name, sizeof(owner_buf));

    // Drop list to select owner house.
    char owner_list_buf[35] = { 0 };
    DropListClass owner_list_edit(143,
        owner_list_buf,
        sizeof(owner_list_buf),
        TPF_EDITOR | TPF_NOSHADOW,
        owner_edit.Get_Width() + owner_edit.Get_XPos() + 20,
        owner_edit.Get_YPos(),
        95,
        40,
        up_btn,
        dn_btn);

    // Populate the lists.
    for (HousesType i = HOUSES_FIRST; i < HOUSES_COUNT; ++i) {
        owner_list_edit.Add_Item(HouseTypeClass::As_Reference(i).Get_Name());
    }

    if (m_House == HOUSES_NONE) {
        m_House = HOUSES_GOODGUY;
    }

    owner_list_edit.Set_Selected_Index(m_House);

    // *** Set trigger persistance
    // Drop list to select persistance.
    char persist_list_buf[35] = { 0 };
    DropListClass persist_list_edit(144,
        persist_list_buf,
        sizeof(persist_list_buf),
        TPF_EDITOR | TPF_NOSHADOW,
        owner_list_edit.Get_Width() + owner_list_edit.Get_XPos() + 20,
        owner_list_edit.Get_YPos(),
        105,
        40,
        up_btn,
        dn_btn);

    // Populate the lists.
    for (PersistanceType i = STATE_FIRST; i < STATE_COUNT; ++i) {
        persist_list_edit.Add_Item(_perstext[i]);
    }

    persist_list_edit.Set_Selected_Index(m_State);

    // *** Setup buttons and intial gadget linkage state.
    TextButtonClass button_event(148,
        TXT_DEBUG_JUST_EVENT,
        TPF_EDITOR | TPF_NOSHADOW | TPF_CENTER,
        event_one_list.Get_XPos(),
        event_one_list.Get_YPos() + 11,
        100,
        9);
    TextButtonClass button_action(147,
        TXT_DEBUG_JUST_ACTION,
        TPF_EDITOR | TPF_NOSHADOW | TPF_CENTER,
        act_one_list.Get_XPos(),
        act_one_list.Get_YPos() + 11,
        100,
        9);
    TextButtonClass button_ok(146, TXT_DEBUG_JUST_ACTION, TPF_EDITOR | TPF_NOSHADOW | TPF_CENTER, 35, 190, 45, 9);
    TextButtonClass button_cancel(145, TXT_DEBUG_JUST_ACTION, TPF_EDITOR | TPF_NOSHADOW | TPF_CENTER, 340, 190, 45, 9);
    button_cancel.Add_Tail(button_ok);
    event_one_list.Add_Tail(button_ok);
    act_one_list.Add_Tail(button_ok);
    button_event.Add_Tail(button_ok);
    button_action.Add_Tail(button_ok);
    owner_edit.Add_Tail(button_ok);
    persist_list_edit.Add_Tail(button_ok);
    owner_list_edit.Add_Tail(button_ok);
    bool process = true;
    bool to_draw = true;

    while (process) {
        Call_Back();

        if (to_draw && g_logicPage->Lock()) {
            g_mouse->Hide_Mouse();
            Dialog_Box(0, 0, 420, 220);
            Draw_Caption(TXT_DEBUG_TRIGGER_EDITOR, 0, 0, 420);
            Fancy_Text_Print("Trigger Event:",
                event_one_list.Get_XPos(),
                event_one_list.Get_YPos() - 7,
                GadgetClass::Get_Color_Scheme(),
                0,
                TPF_NOSHADOW | TPF_EDITOR);
            Fancy_Text_Print("Action to Perform:",
                act_one_list.Get_XPos(),
                act_one_list.Get_YPos() - 7,
                GadgetClass::Get_Color_Scheme(),
                0,
                TPF_NOSHADOW | TPF_EDITOR);
            Fancy_Text_Print("House:",
                owner_list_edit.Get_XPos(),
                owner_list_edit.Get_YPos() - 7,
                GadgetClass::Get_Color_Scheme(),
                0,
                TPF_NOSHADOW | TPF_EDITOR);
            Fancy_Text_Print("Name:",
                owner_edit.Get_XPos(),
                owner_edit.Get_YPos() - 7,
                GadgetClass::Get_Color_Scheme(),
                0,
                TPF_NOSHADOW | TPF_EDITOR);
            Fancy_Text_Print("Persistence:",
                persist_list_edit.Get_XPos(),
                persist_list_edit.Get_YPos() - 7,
                GadgetClass::Get_Color_Scheme(),
                0,
                TPF_NOSHADOW | TPF_EDITOR);

            // Draw linkage indicator for linked event state
            if (m_EventLinkage == EVLINK_LINKED) {
                g_logicPage->Draw_Line(event_one_list.Get_XPos() - 1,
                    event_one_list.Get_YPos() + 3,
                    event_one_list.Get_XPos() - 4,
                    event_one_list.Get_YPos() + 3,
                    COLOR_WHITE);
                g_logicPage->Draw_Line(event_one_list.Get_XPos() - 4,
                    event_one_list.Get_YPos() + 3,
                    act_one_list.Get_XPos() - 4,
                    act_one_list.Get_YPos() + 3,
                    COLOR_WHITE);
                g_logicPage->Draw_Line(act_one_list.Get_XPos() - 1,
                    act_one_list.Get_YPos() + 3,
                    act_one_list.Get_XPos() - 4,
                    act_one_list.Get_YPos() + 3,
                    COLOR_WHITE);
                g_logicPage->Draw_Line(event_two_list.Get_XPos() - 1,
                    event_two_list.Get_YPos() + 3,
                    event_two_list.Get_XPos() - 10,
                    event_two_list.Get_YPos() + 3,
                    COLOR_WHITE);
                g_logicPage->Draw_Line(event_two_list.Get_XPos() - 10,
                    event_two_list.Get_YPos() + 3,
                    act_two_list.Get_XPos() - 10,
                    act_two_list.Get_YPos() + 3,
                    COLOR_WHITE);
                g_logicPage->Draw_Line(act_two_list.Get_XPos() - 1,
                    act_two_list.Get_YPos() + 3,
                    act_two_list.Get_XPos() - 10,
                    act_two_list.Get_YPos() + 3,
                    COLOR_WHITE);
            }

            // Setup the event linkage state.
            event_two_list.Remove();

            switch (m_EventLinkage) {
                case EVLINK_SINGLE:
                    button_event.Set_Text(TXT_DEBUG_JUST_EVENT, false);
                    break;
                case EVLINK_AND:
                    button_event.Set_Text(TXT_DEBUG_TRIGGER_AND, false);
                    event_two_list.Add(button_ok);
                    break;
                case EVLINK_OR:
                    button_event.Set_Text(TXT_DEBUG_TRIGGER_OR, false);
                    event_two_list.Add(button_ok);
                    break;
                case EVLINK_LINKED:
                    button_event.Set_Text(TXT_DEBUG_TRIGGER_LINKED, false);
                    event_two_list.Add(button_ok);
                    break;
                default:
                    break;
            }

            // Setup Event 1 state
            ev_one_house_edit.Remove();
            ev_one_wp_edit.Remove();
            ev_one_num_edit.Remove();
            ev_one_build_edit.Remove();
            ev_one_inf_edit.Remove();
            ev_one_air_edit.Remove();
            ev_one_unit_edit.Remove();
            ev_one_team_edit.Remove();

            // Add controls for the relevant need.
            switch (TEventClass::Event_Needs(event_one_list.Current_Item()->Get_Event())) {
                case NEED_INFANTRY:
                    ev_one_inf_edit.Add(button_ok);
                    break;
                case NEED_UNIT:
                    ev_one_unit_edit.Add(button_ok);
                    break;
                case NEED_AIRCRAFT:
                    ev_one_air_edit.Add(button_ok);
                    break;
                case NEED_BUILDING:
                    ev_one_build_edit.Add(button_ok);
                    break;
                case NEED_WAYPOINT:
                    ev_one_wp_edit.Add(button_ok);
                    break;
                case NEED_NUMBER:
                case NEED_14:
                    ev_one_num_edit.Add(button_ok);
                    break;
                case NEED_TEAM:
                    ev_one_team_edit.Add(button_ok);
                    break;
                case NEED_HOUSE:
                    ev_one_house_edit.Add(button_ok);
                    break;
                default:
                    break;
            }

            // Setup Event 2 state
            ev_two_house_edit.Remove();
            ev_two_wp_edit.Remove();
            ev_two_num_edit.Remove();
            ev_two_build_edit.Remove();
            ev_two_inf_edit.Remove();
            ev_two_air_edit.Remove();
            ev_two_unit_edit.Remove();
            ev_two_team_edit.Remove();

            // Check if the event 2 list is currently active in the gadget linked list.
            if (button_ok.Extract_Gadget(101) != nullptr) {
                // Add controls for the relevant need.
                switch (TEventClass::Event_Needs(event_two_list.Current_Item()->Get_Event())) {
                    case NEED_INFANTRY:
                        ev_two_inf_edit.Add(button_ok);
                        break;
                    case NEED_UNIT:
                        ev_two_unit_edit.Add(button_ok);
                        break;
                    case NEED_AIRCRAFT:
                        ev_two_air_edit.Add(button_ok);
                        break;
                    case NEED_BUILDING:
                        ev_two_build_edit.Add(button_ok);
                        break;
                    case NEED_WAYPOINT:
                        ev_two_wp_edit.Add(button_ok);
                        break;
                    case NEED_NUMBER:
                    case NEED_14:
                        ev_two_num_edit.Add(button_ok);
                        break;
                    case NEED_TEAM:
                        ev_two_team_edit.Add(button_ok);
                        break;
                    case NEED_HOUSE:
                        ev_two_house_edit.Add(button_ok);
                        break;
                    default:
                        break;
                }
            }

            // Set up action linkage state.
            button_action.Remove();
            act_two_list.Remove();

            if (m_EventLinkage == EVLINK_LINKED) {
                act_two_list.Add(button_ok);
            } else {
                button_action.Add(button_ok);

                if (m_ActionLinkage == ACTLINK_AND) {
                    button_action.Set_Text(TXT_DEBUG_TRIGGER_AND);
                    act_two_list.Add(button_ok);
                } else {
                    button_action.Set_Text(TXT_DEBUG_JUST_ACTION);
                }
            }

            // Setup action 1 state.
            act_one_quarry_edit.Remove();
            act_one_spec_edit.Remove();
            act_one_house_edit.Remove();
            act_one_bool_edit.Remove();
            act_one_trig_edit.Remove();
            act_one_wp_edit.Remove();
            act_one_num_edit.Remove();
            act_one_team_edit.Remove();
            act_one_theme_edit.Remove();
            act_one_voc_edit.Remove();
            act_one_movie_edit.Remove();
            act_one_vox_edit.Remove();

            // Add controls for the relevant need.
            switch (TActionClass::Action_Needs(act_one_list.Current_Item()->Get_Action())) {
                case NEED_THEME:
                    act_one_theme_edit.Add(button_ok);
                    break;
                case NEED_MOVIE:
                    act_one_movie_edit.Add(button_ok);
                    break;
                case NEED_SOUND:
                    act_one_voc_edit.Add(button_ok);
                    break;
                case NEED_SPEECH:
                    act_one_vox_edit.Add(button_ok);
                    break;
                case NEED_WAYPOINT:
                    act_one_wp_edit.Add(button_ok);
                    break;
                case NEED_NUMBER:
                    act_one_num_edit.Add(button_ok);
                    break;
                case NEED_TRIGGER:
                    act_one_trig_edit.Add(button_ok);
                    break;
                case NEED_TEAM:
                    act_one_team_edit.Add(button_ok);
                    break;
                case NEED_HOUSE:
                    act_one_house_edit.Add(button_ok);
                    break;
                case NEED_QUARRY:
                    act_one_quarry_edit.Add(button_ok);
                    break;
                case NEED_BOOL:
                    act_one_bool_edit.Add(button_ok);
                    break;
                case NEED_SPECIAL:
                    act_one_spec_edit.Add(button_ok);
                    break;
                default:
                    break;
            }

            // Setup action 2 state.
            act_two_quarry_edit.Remove();
            act_two_spec_edit.Remove();
            act_two_house_edit.Remove();
            act_two_bool_edit.Remove();
            act_two_trig_edit.Remove();
            act_two_wp_edit.Remove();
            act_two_num_edit.Remove();
            act_two_team_edit.Remove();
            act_two_theme_edit.Remove();
            act_two_voc_edit.Remove();
            act_two_movie_edit.Remove();
            act_two_vox_edit.Remove();

            // Check if the action 2 list is currently active in the gadget linked list.
            if (button_ok.Extract_Gadget(103) != nullptr) {
                // Add controls for the relevant need.
                switch (TActionClass::Action_Needs(act_two_list.Current_Item()->Get_Action())) {
                    case NEED_THEME:
                        act_two_theme_edit.Add(button_ok);
                        break;
                    case NEED_MOVIE:
                        act_two_movie_edit.Add(button_ok);
                        break;
                    case NEED_SOUND:
                        act_two_voc_edit.Add(button_ok);
                        break;
                    case NEED_SPEECH:
                        act_two_vox_edit.Add(button_ok);
                        break;
                    case NEED_WAYPOINT:
                        act_two_wp_edit.Add(button_ok);
                        break;
                    case NEED_NUMBER:
                        act_two_num_edit.Add(button_ok);
                        break;
                    case NEED_TRIGGER:
                        act_two_trig_edit.Add(button_ok);
                        break;
                    case NEED_TEAM:
                        act_two_team_edit.Add(button_ok);
                        break;
                    case NEED_HOUSE:
                        act_two_house_edit.Add(button_ok);
                        break;
                    case NEED_QUARRY:
                        act_two_quarry_edit.Add(button_ok);
                        break;
                    case NEED_BOOL:
                        act_two_bool_edit.Add(button_ok);
                        break;
                    case NEED_SPECIAL:
                        act_two_spec_edit.Add(button_ok);
                        break;
                    default:
                        break;
                }
            }

            // Collapse all the lists.
            act_one_spec_edit.Collapse();
            act_two_spec_edit.Collapse();
            act_one_quarry_edit.Collapse();
            act_two_quarry_edit.Collapse();
            ev_one_house_edit.Collapse();
            ev_two_house_edit.Collapse();
            act_one_house_edit.Collapse();
            act_two_house_edit.Collapse();
            ev_one_team_edit.Collapse();
            ev_two_team_edit.Collapse();
            act_one_team_edit.Collapse();
            act_two_team_edit.Collapse();
            ev_one_build_edit.Collapse();
            ev_two_build_edit.Collapse();
            ev_one_unit_edit.Collapse();
            ev_two_unit_edit.Collapse();
            ev_one_inf_edit.Collapse();
            ev_two_inf_edit.Collapse();
            ev_one_air_edit.Collapse();
            ev_two_air_edit.Collapse();
            act_one_trig_edit.Collapse();
            act_two_trig_edit.Collapse();
            act_one_list.Collapse();
            act_two_list.Collapse();
            event_one_list.Collapse();
            event_two_list.Collapse();
            owner_list_edit.Collapse();
            persist_list_edit.Collapse();
            act_one_bool_edit.Collapse();
            act_two_bool_edit.Collapse();
            act_one_theme_edit.Collapse();
            act_two_theme_edit.Collapse();
            act_one_voc_edit.Collapse();
            act_two_voc_edit.Collapse();
            act_one_movie_edit.Collapse();
            act_two_movie_edit.Collapse();
            act_one_vox_edit.Collapse();
            act_two_vox_edit.Collapse();
            button_ok.Flag_List_To_Redraw();
            g_mouse->Show_Mouse();
            to_draw = false;
            g_logicPage->Unlock();
        }

        switch (button_ok.Input()) {
        }
    }

    switch (m_EventOne.Event_Needs()) {
        default:
            break;
        case NEED_INFANTRY:
            m_EventOne.m_IntegerValue = ev_two_inf_edit.Current_Index();
            break;
        case NEED_UNIT:
            m_EventOne.m_IntegerValue = ev_one_unit_edit.Current_Index();
            break;
        case NEED_AIRCRAFT:
            m_EventOne.m_IntegerValue = ev_one_air_edit.Current_Index();
            break;
        case NEED_BUILDING:
            m_EventOne.m_IntegerValue = ev_one_build_edit.Current_Index();
            break;
        case NEED_WAYPOINT:
            /////////////////
            break;
        case NEED_TEAM:
            m_EventOne.m_IntegerValue = TeamTypeClass::From_Name(ev_one_team_edit.Current_Item())->Get_Heap_ID();
            break;
        case NEED_NUMBER:
        case NEED_14:
            m_EventOne.m_IntegerValue = atoi(ev_one_num_edit.Get_Text());
            break;
        case NEED_HOUSE:
            m_EventOne.m_IntegerValue = ev_one_build_edit.Current_Index();
            break;
    }

    return false;
}

/**
 * Editor function to generate trigger description for display.
 */
const char *TriggerTypeClass::Description()
{
    static char _buffer[128];
    char num_buf[30];
    char persist_type;
    char ev_link;
    char act_link;
    const char *need;

    switch (m_State) {
        case STATE_SEMI_PERSISTANT:
            persist_type = 'S';
        case STATE_PERSISTANT:
            persist_type = 'P';
            break;
        default:
            persist_type = 'V';
            break;
    }

    switch (m_EventLinkage) {
        case EVLINK_AND:
            ev_link = '&';
            break;
        case EVLINK_OR:
            ev_link = '|';
            break;
        case EVLINK_LINKED:
            ev_link = '=';
            break;
        default:
            ev_link = '.';
            break;
    }

    switch (m_ActionLinkage) {
        case ACTLINK_AND:
            act_link = '&';
            break;
        default:
            act_link = '.';
            break;
    }

    switch (m_EventOne.Event_Needs()) {
        case NEED_INFANTRY:
            need = Text_String(InfantryTypeClass::As_Reference(InfantryType(m_EventOne.m_IntegerValue)).Full_Name());
            break;
        case NEED_UNIT:
            need = Text_String(UnitTypeClass::As_Reference(UnitType(m_EventOne.m_IntegerValue)).Full_Name());
            break;
        case NEED_AIRCRAFT:
            need = Text_String(AircraftTypeClass::As_Reference(AircraftType(m_EventOne.m_IntegerValue)).Full_Name());
            break;
        case NEED_BUILDING:
            need = Text_String(BuildingTypeClass::As_Reference(BuildingType(m_EventOne.m_IntegerValue)).Full_Name());
            break;
        case NEED_WAYPOINT:
            if (m_EventOne.m_IntegerValue > 26) {
                sprintf(num_buf, "%c%c", m_EventOne.m_IntegerValue / 26 + '@', m_EventOne.m_IntegerValue % 26 + 'A');
            } else {
                sprintf(num_buf, "%c", m_EventOne.m_IntegerValue + 'A');
            }
            need = num_buf;
            break;
        case NEED_NUMBER:
            sprintf(num_buf, "%d", m_EventOne.m_IntegerValue);
            need = num_buf;
            break;
        default:
            need = "";
            break;
    }

    sprintf(_buffer,
        "%4.4s\t %s %c%c%c  %s%s",
        m_Name,
        HouseTypeClass::As_Reference(m_House).Get_Name(),
        persist_type,
        ev_link,
        act_link,
        TEventClass::Name_From_Event(m_EventOne.m_Type),
        need);

    return _buffer;
}

/**
 * Editor function to draw trigger entry in a list.
 */
void TriggerTypeClass::Draw_It(int index, int x, int y, int x_max, int y_max, BOOL selected, TextPrintType style)
{
    static int _tabs[] = { 13, 40 };
    RemapControlType *remapper = GadgetClass::Get_Color_Scheme();

    if ((style & TPF_FONTS) == TPF_6PT_GRAD || (style & TPF_FONTS) == TPF_EDITOR) {
        if (selected) {
            style |= TPF_USE_BRIGHT;
            g_logicPage->Fill_Rect(x, y, ((x + x_max) - 1), ((y + y_max) - 1), remapper->WindowPalette[0]);
        } else if (!(style & TPF_USE_GRAD_PAL)) {
            style |= TPF_USE_MEDIUM;
        }
    } else {
        remapper = (selected ? &ColorRemaps[REMAP_10] : &ColorRemaps[REMAP_5]);
    }

    Conquer_Clip_Text_Print(Description(), x, y, remapper, COLOR_TBLACK, style, x_max, _tabs);
}

/**
 * Initialises the TriggerTypes heap.
 */
void TriggerTypeClass::Init()
{
    g_TriggerTypes.Free_All();
}

/**
 * Reads the [Trigs] section from an ini file and generates TriggerTypes from it.
 */
void TriggerTypeClass::Read_INI(GameINIClass &ini)
{
    int entries = ini.Entry_Count("Trigs");

    // Iterate all entries in Trigs and create trigger types from them.
    for (int i = 0; i < entries; ++i) {
        char buff[128];
        const char *entry = ini.Get_Entry("Trigs", i);
        TriggerTypeClass *tt = new TriggerTypeClass;
        ini.Get_String("Trigs", entry, nullptr, buff, sizeof(buff));
        tt->Fill_In(entry, buff);
    }

    // Hack to ensure strings can be converted to heap ID after parsing all triggers.
    if (g_iniFormat < 2) {
        for (int i = 0; i < g_TriggerTypes.Count(); ++i) {
            // Recover string pointer.
            char *name = reinterpret_cast<char *>(g_TriggerTypes[i].m_ActionOne.m_TriggerType.Get_ID());

            if (name != nullptr) {
                g_TriggerTypes[i].m_ActionOne.m_TriggerType = From_Name(name);
                free(name);
            }

            // Recover string pointer.
            name = reinterpret_cast<char *>(g_TriggerTypes[i].m_ActionTwo.m_TriggerType.Get_ID());

            if (name != nullptr) {
                g_TriggerTypes[i].m_ActionTwo.m_TriggerType = From_Name(name);
                free(name);
            }
        }
    }
}

/**
 * Writes the [Trigs] section to an ini file from the TriggerTypes heap.
 */
void TriggerTypeClass::Write_INI(GameINIClass &ini)
{
    // Clear older section name in case we imported an old map format?
    ini.Clear("Triggers");
    ini.Clear("Trigs");

    for (int i = 0; i < g_TriggerTypes.Count(); ++i) {
        char buff[256];
        g_TriggerTypes[i].Build_INI_Entry(buff);
        ini.Put_String("Trigs", g_TriggerTypes[i].Get_Name(), buff);
    }
}

/**
 * Gets a TriggerTypeClass instance from its name.
 */
TriggerTypeClass *TriggerTypeClass::From_Name(const char *name)
{
    if (strcasecmp(name, "<none>") == 0 || strcasecmp(name, "none") == 0) {
        return nullptr;
    }

    if (name != nullptr) {
        for (int index = 0; index < g_TriggerTypes.Count(); ++index) {
            if (strcasecmp(name, Name_From((TriggerType)index)) == 0) {
                return &g_TriggerTypes[index];
            }
        }
    }

    return nullptr;
}

/**
 * Gets a trigger name from its heap ID.
 */
const char *TriggerTypeClass::Name_From(TriggerType trigger)
{
    return (trigger != TRIGGER_NONE) && (trigger < g_TriggerTypes.Count()) ? As_Reference(trigger).Get_Name() : "<none>";
}

/**
 * Gets a TriggerTypeClass name from an instance.
 */
const char *TriggerTypeClass::Name_From(TriggerTypeClass *trigger)
{
    if (trigger != nullptr) {
        return trigger->Get_Name();
    }
    return nullptr;
}

/**
 * Gets a TriggerTypeClass reference from its heap ID.
 */
TriggerTypeClass &TriggerTypeClass::As_Reference(TriggerType type)
{
    DEBUG_ASSERT(type != TRIGGER_NONE);
    DEBUG_ASSERT(type < g_TriggerTypes.Count());

    return g_TriggerTypes[type];
}

/**
 * Gets a TriggerTypeClass pointer from its heap ID.
 */
TriggerTypeClass *TriggerTypeClass::As_Pointer(TriggerType type)
{
    return type != TRIGGER_NONE && type < g_TriggerTypes.Count() ? &g_TriggerTypes[type] : nullptr;
}

/**
 * Fills in a TriggerTypeClass from the ini entry string.
 */
void TriggerTypeClass::Fill_In(const char *name, char *options)
{
    strlcpy(m_Name, name, sizeof(m_Name));
    m_State = PersistanceType(atoi(strtok(options, ",")));
    m_House = HousesType(atoi(strtok(nullptr, ",")));
    m_EventLinkage = EventLinkType(atoi(strtok(nullptr, ",")));
    m_ActionLinkage = ActionLinkType(atoi(strtok(nullptr, ",")));
    m_EventOne.Read_INI();
    m_EventTwo.Read_INI();
    m_ActionOne.Read_INI();
    m_ActionTwo.Read_INI();
}

/**
 * Generates a TriggerTypeClass ini entry string.
 */
void TriggerTypeClass::Build_INI_Entry(char *buffer)
{
    sprintf(buffer, "%d,%d,%d,%d,", m_State, m_House, m_EventLinkage, m_ActionLinkage);
    m_EventOne.Build_INI_Entry(&buffer[strlen(buffer)]);
    strcat(buffer, ",");
    m_EventTwo.Build_INI_Entry(&buffer[strlen(buffer)]);
    strcat(buffer, ",");
    m_ActionOne.Build_INI_Entry(&buffer[strlen(buffer)]);
    strcat(buffer, ",");
    m_ActionTwo.Build_INI_Entry(&buffer[strlen(buffer)]);
}
