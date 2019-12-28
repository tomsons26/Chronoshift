#include "ipxconn.h"

unsigned short IPXConnClass::s_Socket;
// int IPXConnClass::s_ConnectionNum;
// ECB *IPXConnClass::s_ListenECB;
// IPXHEADER *IPXConnClass::s_ListenHeader;
// char *IPXConnClass::s_ListenBuf;
// ECB *IPXConnClass::s_SendECB;
// IPXHEADER *IPXConnClass::s_SendHeader;
// char *IPXConnClass::s_SendBuf;
// long IPXConnClass::s_Handler;
BOOL IPXConnClass::s_Configured;
BOOL IPXConnClass::s_SocketOpen;
BOOL IPXConnClass::s_Listening;
// int IPXConnClass::s_PacketLen;

IPXConnClass::IPXConnClass(int squeue_size, int rqueue_size, int buff_size, uint16_t type, IPXAddressClass *ip_address,
    int connection_id, char *connection_name, int exbuffsize) :
    ConnectionClass(squeue_size, rqueue_size, buff_size, type, 2, -1, 60, exbuffsize),
    m_IPAddress()
{
    if (ip_address != nullptr) {
        // cannot assign right expression to class object
        // m_IPAddress = ip_address;

        memcpy(&m_IPAddress, ip_address, sizeof(m_IPAddress));
    }

    m_ConnectionID = connection_id;
    strcpy(m_ConnectionName, connection_name);
    char net[4];
    char node[6];
    m_IPAddress.Get_Address(net, node);
    memcpy(m_SomeNode, node, sizeof(node));
    m_UseSomeNode = false;
}

/**
 * Initialise the class instance.
 *
 */
void IPXConnClass::Init()
{
    ConnectionClass::Init();
}

/**
 *
 *
 */
void IPXConnClass::Configure(unsigned short socket, int connection_num, ECB *listen_ecb, ECB *send_ecb,
    IPXHEADER *listen_header, IPXHEADER *send_header, char *listen_buf, char *send_buf, long handler, int packet_len)
{
    // s_ConnectionNum = connection_num;
    // s_ListenHeader = listen_header;
    // s_SendHeader = send_header;
    // s_ListenBuf = listen_buf;
    // s_SendBuf = send_buf;
    s_Socket = socket;
    // s_Handler = handler;
    // s_ListenECB = listen_ecb;
    // s_PacketLen = packet_len;
    // s_SendECB = send_ecb;
    s_Configured = true;
}

/**
 * Starts listening for connections.
 *
 */
BOOL IPXConnClass::Start_Listening()
{
    if (!IPXConnClass::Open_Socket(s_Socket)) {
        return false;
    }
    if (g_PacketTransport->Start_Listening()) {
        s_Listening = true;
        return true;
    }
    IPXConnClass::Close_Socket(s_Socket);
    return false;
}

/**
 * Stops listening for connections.
 *
 */
BOOL IPXConnClass::Stop_Listening()
{
    if (g_PacketTransport != nullptr) {
        g_PacketTransport->Stop_Listening();
    }
    s_Listening = false;
    return true;
}

/**
 *
 *
 */
BOOL IPXConnClass::Send(void *src, int src_len, void *unknown1, int unknown2)
{
    return IPXConnClass::Send_To(src, src_len, &m_IPAddress, m_UseSomeNode ? m_SomeNode : nullptr);
}

/**
 * Opens a socket on a given port.
 *
 */
BOOL IPXConnClass::Open_Socket(unsigned port)
{
    return s_SocketOpen = g_PacketTransport->Open_Socket(port);
}

/**
 * Closes the socket if open.
 *
 */
void IPXConnClass::Close_Socket(unsigned port)
{
    g_PacketTransport->Close_Socket();
    s_SocketOpen = false;
}

/**
 * Writes data to the sockets interface for a specific destination.
 *
 */
BOOL IPXConnClass::Send_To(void *src, int src_len, IPXAddressClass *address, void *unknown2)
{
    g_PacketTransport->Write_To(src, src_len, address);
    return true;
}

/**
 * Writes data to the sockets interface for broadcast.
 *
 */
BOOL IPXConnClass::Broadcast(void *src, int src_len)
{
    g_PacketTransport->Broadcast(src, src_len);
    return true;
}
