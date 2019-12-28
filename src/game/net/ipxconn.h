/**
 * @file
 *
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

#pragma once

#ifndef IPXCONN_H
#define IPXCONN_H

#include "always.h"
#include "connection.h"
#include "ipx.h"

struct ECB;

class IPXConnClass : public ConnectionClass
{
public:
    IPXConnClass(int squeue_size, int rqueue_size, int buff_size, uint16_t type, IPXAddressClass *ip_address,
        int connection_id, char *connection_name, int exbuffsize);
    virtual ~IPXConnClass() {}

    virtual void Init() override;
    virtual int Send(uint8_t *data, int32_t datalen, void *ack_data, int32_t ack_datalen) override;

    static void Configure(unsigned short socket, int connection_num, ECB *listen_ecb, ECB *send_ecb,
        IPXHEADER *listen_header, IPXHEADER *send_header, char *listen_buf, char *send_buf, long handler, int packet_len);
    static BOOL Start_Listening();
    static BOOL Stop_Listening();
    static BOOL Open_Socket(unsigned port);
    static void Close_Socket(unsigned port);
    static BOOL Send_To(void *src, int src_len, IPXAddressClass *address, void *unknown2);
    static BOOL Broadcast(void *src, int src_len);

private:
    IPXAddressClass m_IPAddress;
    char m_SomeNode[6];
    BOOL m_UseSomeNode;
    int m_ConnectionID;
    char m_ConnectionName[40];

public:
    static unsigned short s_Socket;
    //static int s_ConnectionNum;
    //static ECB *s_ListenECB;
    //static IPXHEADER *s_ListenHeader;
    //static char *s_ListenBuf;
    //static ECB *s_SendECB;
    //static IPXHEADER *s_SendHeader;
    //static char *s_SendBuf;
    //static long s_Handler;
    static BOOL s_Configured;
    static BOOL s_SocketOpen;
    static BOOL s_Listening;
    //static int s_PacketLen;
};
#endif // IPXCONN_H
