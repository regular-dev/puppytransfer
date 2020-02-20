#include "UdtSocket.hpp"

#include <cstring>

#if defined(WIN32) || defined(__WIN32) || defined(__WIN32__)
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

UdtSocket::UdtSocket()
{
    m_socket = UDT::socket(AF_INET, SOCK_STREAM, 0);

    //bool rendezvous = true;
    //UDT::setsockopt(m_socket, 0, UDT_RENDEZVOUS, &rendezvous, sizeof(bool));
}

UdtSocket::~UdtSocket()
{
    UDT::close(m_socket);
}

UdtSocket::UdtSocket(UDTSOCKET &sock)
{
    m_socket = sock;
}

int UdtSocket::listen(uint16_t port)
{
    sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(my_addr.sin_zero), '\0', 8);

    return UDT::bind(m_socket, (sockaddr *)&my_addr, sizeof(my_addr));
}

int UdtSocket::bind(universal_socket_t existing_udp_socket)
{
    return UDT::bind2(m_socket, existing_udp_socket);
}

int UdtSocket::connect(const std::string &ip, uint16_t port)
{
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr);
    std::memset(&(serv_addr.sin_zero), '\0', 8);

    const auto status = UDT::connect(m_socket, (sockaddr *)&serv_addr, sizeof(serv_addr));
    if (UDT::ERROR == status) {
        m_is_connected = false;
        // TODO : handle error connect. log_debug or something ?
    } else {
        m_is_connected = true;

        m_remote_addr = ip;
        m_remote_port = port;
    }

    return status;
}

int UdtSocket::send(const char *data, size_t size_data)
{
    const auto status = UDT::send(m_socket, data, size_data, 0);

    if (UDT::ERROR == status)
        m_is_connected = false;

    return status;
}

int UdtSocket::recv(char *buf, size_t size_buf)
{
    const auto status = UDT::recv(m_socket, buf, size_buf, 0);

    if (UDT::ERROR == status)
        m_is_connected = false;

    return status;
}

void UdtSocket::setNonBlocking(bool flag)
{
    flag = !flag;
    UDT::setsockopt(m_socket, 0, UDT_RCVSYN, &flag, sizeof(bool));
    UDT::setsockopt(m_socket, 0, UDT_SNDSYN, &flag, sizeof(bool));
}

bool UdtSocket::NonBlockingMode() const
{
    return true; // TODO : combine rcv snd syn block mode
}
