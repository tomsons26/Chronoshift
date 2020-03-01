/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 * @author tomsons26
 *
 * @brief Message queue related functions.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "queue.h"
#include "eventhandler.h"
#include "gameevent.h"
#include "globals.h"
#include "house.h"
#include "mouse.h"
#include "session.h"
#include "target.h"

#ifndef GAME_DLL
TQueueClass<GameEventClass, OUTGOING_SIZE> g_OutgoingEvents; //was OutList
TQueueClass<GameEventClass, SCHEDULED_SIZE> g_ScheduledEvents; //was DoList
#endif

BOOL Queue_Options()
{
    GameEventClass ev(GameEventClass::EVENT_OPTIONS);
    return g_OutgoingEvents.Add(ev);
}

BOOL Queue_Exit()
{
    GameEventClass ev(GameEventClass::EVENT_EXIT);
    return g_OutgoingEvents.Add(ev);
}

void Queue_Record()
{
#ifdef GAME_DLL
    void (*func)() = reinterpret_cast<void (*)()>(0x0052BD20);
    return func();
#else
    int event_index = 0;
    for (int index = 0; index < g_ScheduledEvents.Get_Count(); ++index) {
        GameEventClass &ev = g_ScheduledEvents.Fetch_From_Head(index);
        if (g_GameFrame == ev.Get_Event_Frame() && !ev.Is_Executed()) {
            ++event_index;
        }
    }
    g_Session.Recording_File().Write(&event_index, sizeof(event_index));

    for (int index = 0; index < g_ScheduledEvents.Get_Count(); ++index) {
        GameEventClass &ev = g_ScheduledEvents.Fetch_From_Head(index);
        if (g_GameFrame == ev.Get_Event_Frame() && !ev.Is_Executed()) {
            g_Session.Recording_File().Write(&ev, sizeof(GameEventClass));
            --event_index;
        }
    }
#endif
}

void Queue_Playback()
{
#ifdef GAME_DLL
    void (*func)() = reinterpret_cast<void (*)()>(0x0052BDEC);
    return func();
#else
    static int _mx, _my;

    GameEventClass event;
    if (g_Keyboard->Check() && g_Keyboard->Get() == KN_ESC || g_Session.Attraction_Allowed()) {
        captainslog_debug("Queue_Playback() - Interupted by escape press");
        g_GameActive = 0;
        return;
    } else if (g_Session.Attraction_Allowed() && g_GameFrame > 0
        && (_mx != g_Mouse->Get_Mouse_X() || _my != g_Mouse->Get_Mouse_Y())) {
        captainslog_debug("Queue_Playback() - Interupted by mouse move");
        g_GameActive = 0;
    } else {
        _mx = g_Mouse->Get_Mouse_X();
        _my = g_Mouse->Get_Mouse_Y();
        Compute_Game_CRC();
        ////////g_CRC[g_GameFrame & 0x1F] = g_GameCRC;

        if (g_GameFrame >= g_Session.Trap_Print_CRC()) {
            Print_CRCs(0);
            captainslog_debug("Queue_Playback() - Reached CRC print frame");
            Emergency_Exit(0);
        }

        if (g_GameFrame || g_Session.Game_To_Play() == GAME_CAMPAIGN) {
            int sendrate = g_Session.Frame_Send_Rate();
            int rate = sendrate * ((g_GameFrame + sendrate - 1) / sendrate);

            if (g_Session.Game_To_Play() == GAME_CAMPAIGN || g_Session.Game_To_Play() == GAME_SKIRMISH
                || g_Session.Packet_Protocol() != COMPROTO_TWO || g_GameFrame == rate) {
                size_t numevents = 0;
                bool read = true;

                if (g_Session.Recording_File().Read(&numevents, sizeof(numevents)) != sizeof(numevents)) {
                    captainslog_debug("Queue_Playback() - Reading event count failed");
                    read = false;
                } else {
                    for (int i = 0; i < numevents; ++i) {
                        if (g_Session.Recording_File().Read(&event, sizeof(GameEventClass)) != sizeof(GameEventClass)) {
                            captainslog_debug("Queue_Playback() - Reading events failed");
                            read = false;
                            break;
                        }
                        event.Set_Executed(false); // i think...
                        g_ScheduledEvents.Add(event);
                    }
                }

                if (read == false) {
                    g_GameActive = 0;
                } else {
                    int players = 0;
                    HousesType house;
                    if (g_Session.Game_To_Play() == GAME_CAMPAIGN) {
                        players = 1;
                        house = g_PlayerPtr->What_Type();
                    } else {
                        players = g_Session.MPlayer_Max();
                        house = HOUSES_MULTI_FIRST;
                    }

                    if (Execute_Scheduled_Events(players, house)) {
                        Clean_Scheduled_Events();
                    } else {
                        captainslog_debug("Queue_Playback() - Execute_ScheduledEvents() failed");
                        g_GameActive = false;
                    }
                }
            }
        }
    }
#endif
}

void Queue_AI()
{
    if (g_Session.Playback_Game()) {
        Queue_Playback();
    } else {
        switch (g_Session.Game_To_Play()) {
            case GAME_CAMPAIGN:
            case GAME_SKIRMISH:
                Queue_AI_Normal();
                break;
            case 1:
            case 2:
            case GAME_IPX:
            case GAME_INTERNET:
            case 6:
            case 7:
                Queue_AI_Multiplayer();
                break;
            default:
                break;
        };
    }
}

void Queue_AI_Normal()
{
#ifdef GAME_DLL
    void (*func)() = reinterpret_cast<void (*)()>(0x00528F20);
    return func();
#else
    while (g_OutgoingEvents.Get_Count() > 0) {
        GameEventClass &ev = g_OutgoingEvents.Fetch_Head();
        ev.Set_Executed(false); // i think.....
        g_ScheduledEvents.Add(ev);
        g_OutgoingEvents.Remove_Head();
    }
    if (g_Session.Record_Game()) {
        Queue_Record();
    }
    if (Execute_Scheduled_Events(1, g_PlayerPtr->What_Type())) {
        Clean_Scheduled_Events();
    } else {
        captainslog_debug("Queue_AI_Normal() - Execute_ScheduledEvents() failed");
        g_GameActive = false;
    }
#endif
}

void Queue_AI_Network()
{
#ifdef GAME_DLL
#else
#endif
}

void Queue_AI_Multiplayer()
{
#ifdef GAME_DLL
    void (*func)() = reinterpret_cast<void (*)()>(0x00529020);
    return func();
#else
#endif
}

BOOL Queue_Mission(TargetClass whom, MissionType mission, target_t target, target_t dest)
{
    GameEventClass ev(whom, mission, target, dest);
    return g_OutgoingEvents.Add(ev);
}

BOOL Queue_Mission_Formation(
    TargetClass whom, MissionType mission, target_t target, target_t dest, SpeedType form_speed, MPHType form_max_speed)
{
    GameEventClass ev(whom, mission, target, dest, form_speed, form_max_speed);
    return g_OutgoingEvents.Add(ev);
}

BOOL Execute_Scheduled_Events(int house_count, HousesType houses, ConnectionManagerClass *conn_mgr,
    TCountDownTimerClass<FrameTimerClass> *a4, signed int *a5, unsigned short *a6, unsigned short *a7)
{
#ifdef GAME_DLL
    BOOL(*func)
    (int,
        HousesType,
        ConnectionManagerClass *,
        TCountDownTimerClass<FrameTimerClass> *,
        signed int *,
        unsigned short *,
        unsigned short *) = reinterpret_cast<BOOL (*)(int,
        HousesType,
        ConnectionManagerClass *,
        TCountDownTimerClass<FrameTimerClass> *,
        signed int *,
        unsigned short *,
        unsigned short *)>(0x0052B69C);
    return func(house_count, houses, conn_mgr, a4, a5, a6, a7);
#else
    return false;
#endif
}

void Clean_Scheduled_Events(ConnectionManagerClass *conn_mgr)
{
#ifdef GAME_DLL
    void (*func)(ConnectionManagerClass *) = reinterpret_cast<void (*)(ConnectionManagerClass *)>(0x0052BC94);
    return func(conn_mgr);
#else
    while (g_ScheduledEvents.Get_Count() > 0) {
        g_Keyboard->Check();
        if ( conn_mgr != nullptr ) {
            //Update_Queue_Mono(conn_mgr, 7);
        }

        GameEventClass &ev = g_ScheduledEvents.Fetch_Head();
        if (!ev.Is_Executed() && ev.Get_Event_Frame() >= g_GameFrame) {
            break;
        }
        g_ScheduledEvents.Remove_Head();
    }
#endif
}

void Compute_Game_CRC()
{
#ifdef GAME_DLL
    void (*func)() = reinterpret_cast<void (*)()>(0x0052C044);
    func();
#endif
}

void Print_CRCs(GameEventClass *event)
{
#ifdef GAME_DLL
    void (*func)(GameEventClass *) = reinterpret_cast<void (*)(GameEventClass *)>(0x0052C2B8);
    func(event);
#endif
}
