#ifndef UDPSOCK_H
#define UDPSOCK_H

#include <unistd.h>
#include <string>
#include <errno.h>

#include "IpAddress.hpp"

class UdpSocket
{
public :
    UdpSocket();
    UdpSocket(const std::string &arg_ip, uint16_t port);
    ~UdpSocket();

    int sendTo(const void *data, std::size_t siz,
               const std::string &arg_ip, uint16_t port);
    int send(const void *data, std::size_t);

    int bind(const std::string &arg_ip, uint16_t port);

    int recvFrom(void *data, size_t size_buf, std::string &arg_ip, uint16_t &port);

    int connect(const std::string &arg_ip, uint16_t port);
    void disconnect();

protected:
    int m_fd;
    IpAddress m_addr;
    uint16_t m_port;
};

#endif
