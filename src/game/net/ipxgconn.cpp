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

#include "ipxgconn.h"
#include "commbuff.h"

IPXGlobalConnClass::IPXGlobalConnClass(int squeue_size, int rqueue_size, int buff_size, unsigned short a5) :
    IPXConnClass(squeue_size, rqueue_size, buff_size + 2, 4661, nullptr, 0, "", 10)
{
    m_Unknown1 = a5;
    m_Unknown2 = false;
    for (int i = 0; i < 4; ++i) {
        m_Unknown3[i] = -1;
    }
    m_Unknown4 = 0;
}

/**
 *
 *
 */
int IPXGlobalConnClass::Send_Packet(void *buf, int buf_len, IPXAddressClass *ip_address, int a5)
{
    IPXAddressClass ipx;

    PacketHeader *buff_header = reinterpret_cast<PacketHeader *>(m_PacketBuffer);
    buff_header->Type = m_PacketType;
    buff_header->Command = !a5 || ip_address == nullptr;
    buff_header->Number = m_CommBuffer->m_SendTotal;
    //*&v10->ipxconn.conn.PacketBuffer->field_7[0] = m_Unknown1;
    //(unsigned short)buff_header[1] = m_Unknown1;
    memcpy(&buff_header[1], &m_Unknown1, sizeof(m_Unknown1));

    if (ip_address != nullptr) {
        // cannot assign right expression to class object
        // ipx = ip_address;
        memcpy(&ipx, ip_address, sizeof(IPXAddressClass));
    }
    memcpy(&buff_header[1].Command, buf, buf_len);
    return m_CommBuffer->Queue_Send(m_PacketBuffer, buf_len + 9, &ipx, sizeof(ipx));
}

int IPXGlobalConnClass::Receive_Packet(void *buf, int buf_len, IPXAddressClass *address)
{
    PacketHeader *buff_header = reinterpret_cast<PacketHeader *>(m_PacketBuffer);
    if (buff_header->Type != m_PacketType) {
        return 0;
    }

    switch (buff_header->Command) {
        case 0: {
            int i = 0;
            bool v19 = false;
            while (i < m_CommBuffer->m_ReceiveTotal) {
                // conversion to common class type is impossible
                if (/*address == m_IPAddresses[i] &&*/ m_Unknown3[i] == buff_header->Number) {
                    v19 = true;
                    break;
                }
                ++i;
                if (i >= 4) {
                    break;
                }
            }

            char v18[9];
            *(uint16_t *)v18 = m_PacketType;
            v18[2] = 2;
            *(int32_t *)&v18[3] = buff_header->Number;
            *(unsigned short *)&v18[7] = m_Unknown1;
            Send(v18, sizeof(v18), address, sizeof(IPXAddressClass));

            if (v19 == false) {
                m_CommBuffer->Queue_Receive(buf, buf_len, address, sizeof(IPXAddressClass));

                // v9 = address;
                // v10 = &this->ipaddresses[this->ipaddressindex];
                // v11 = this;
                // v10->address_0 = address->address_0;
                // v9 = (v9 + 4);
                // v10 = (v10 + 4);
                // v10->address_0 = v9->address_0;
                // LOWORD(v10->address_4) = v9->address_4;

                m_Unknown3[m_Unknown4] = buff_header->Number;
                m_Unknown4++;
                if (m_Unknown4 >= 4) {
                    m_Unknown4 = 0;
                    return 1;
                }
            }
            break;
        }
        case 1: {
            m_CommBuffer->Queue_Receive(buf, buf_len, address, sizeof(IPXAddressClass));
            return 1;
        }
        case 2: {
            for (int i = 0; i < m_CommBuffer->m_SendCount; ++i) {
                CommBufferClass::SendQueueType *entry = m_CommBuffer->Get_Send(i);
                PacketHeader *entry_header = reinterpret_cast<PacketHeader *>(entry->m_Buffer);
                if (buff_header->Number == entry_header->Number && entry_header->Command == 0) {
                    entry->m_Flags |= 2;
                    return 1;
                }
            }
            return 1;
        }
        default:
            return 1;
    }
    return 1;
}

int IPXGlobalConnClass::Get_Packet(void *a2, int *a3, IPXAddressClass *address, unsigned short *a5)
{
    CommBufferClass::ReceiveQueueType *qentry = m_CommBuffer->Get_Receive(0);
    if ( m_CommBuffer->m_ReceiveCount && qentry != nullptr && !(qentry->m_Flags & 2) )
    {
        int len = qentry->m_BufLen - 16;
        unsigned char *buffer = qentry->m_Buffer;
        qentry->m_Flags |= 2u;
        if ( len > 0 ) {
            memcpy(a2, buffer + 16, len);
        }
        *a3 = len;
        *a5 = *(buffer + 7);
        //v9 = qentry->ExtraBuffer;
        //*&a4->address[0] = *&v9->address[0];
        //*&a4->address[4] = *&v9->address[4];
        //*&a4->byte8 = *&v9->byte8;
        memcpy(address, qentry->m_ExtraBuffer, sizeof(IPXAddressClass));
        return true;
    }
    return false;
}

/**
 *
 *
 */
int IPXGlobalConnClass::Send(char *src, int src_len, void *a4, int a5)
{
    IPXAddressClass *ipx = reinterpret_cast<IPXAddressClass *>(a4);
    if (ipx->Is_Broadcast()) {
        return IPXConnClass::Broadcast(src, src_len);
    }

    void *node;
    if (m_Unknown2 && !memcmp(a4, m_Net, sizeof(m_Net))) {
        node = m_Node;
    } else {
        node = 0;
    }
    return IPXConnClass::Send_To(src, src_len, ipx, node);
}

/**
 *
 *
 */
BOOL IPXGlobalConnClass::Service_Receive_Queue()
{
    for (int i = 0; i < m_CommBuffer->m_ReceiveCount; ++i) {
        if (m_CommBuffer->Get_Receive(i)->m_Flags & 2) {
            m_CommBuffer->UnQueue_Receive(nullptr, nullptr, i--, nullptr, nullptr);
        }
    }
    return true;
}

/**
 *
 *
 */
void IPXGlobalConnClass::Set_Bridge(char *a2)
{
    if ( s_Configured ) {
        m_Unknown2 = false;
    }
}
