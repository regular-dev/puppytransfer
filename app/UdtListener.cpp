#include "UdtListener.hpp"

#include <cstring>

#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

UdtListener::UdtListener()
{
    m_socket = UDT::socket(AF_INET, SOCK_STREAM, 0);

   // bool rendezvous = true;
    //UDT::setsockopt(m_socket, 0, UDT_RENDEZVOUS, &rendezvous, sizeof(bool));
}

UdtListener::UdtListener(uint16_t port)
{
    m_socket = UDT::socket(AF_INET, SOCK_STREAM, 0);
    listen(port);
}

UdtListener::~UdtListener()
{
    UDT::close(m_socket);
}

int UdtListener::listen(uint16_t port)
{
    sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(my_addr.sin_zero), '\0', 8);

    const auto res_bind = UDT::bind(m_socket, (sockaddr *)&my_addr, sizeof(my_addr));
    if (res_bind < 0)
        return res_bind;

    return UDT::listen(m_socket, 5);
}

UdtSocket UdtListener::accept()
{
    int namelen;
    sockaddr_in remote_addr;

    UDTSOCKET recver = UDT::accept(m_socket, (sockaddr*)&remote_addr, &namelen);

    UdtSocket client_socket(recver);
    client_socket.m_remote_addr = inet_ntoa(remote_addr.sin_addr);
    client_socket.m_remote_port = remote_addr.sin_port;

    return client_socket;
}
