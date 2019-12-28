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

#ifndef IPXGCONN_H
#define IPXGCONN_H

#include "always.h"
#include "ipxconn.h"

class IPXGlobalConnClass : public IPXConnClass
{
public:
    IPXGlobalConnClass(int squeue_size, int rqueue_size, int buff_size, unsigned short a5);
    virtual ~IPXGlobalConnClass() {}

    virtual int Service_Receive_Queue() final;
    virtual int Send(char *src, int src_len, void *a4, int a5) final;
    virtual int Send_Packet(void *buf, int buf_len, IPXAddressClass *ip_address, int a5) final;
    virtual int Receive_Packet(void *,int,IPXAddressClass *) final;
    virtual int Get_Packet(void *,int *,IPXAddressClass *,unsigned short *) final;

    void Set_Bridge(char *);


private:
    unsigned short m_Unknown1;
    char m_Net[4];
    char m_Node[6];
    BOOL m_Unknown2;
    IPXAddressClass m_IPAddresses[4];
    int m_Unknown3[4];
    int m_Unknown4;
};

#endif //IPXGCONN_H
