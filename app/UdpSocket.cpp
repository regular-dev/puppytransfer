#include "UdpSocket.hpp"

UdpSocket::UdpSocket()
{
    m_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

int UdpSocket::connect(const std::string &arg_ip, uint16_t port)
{
    IpAddress ipaddr(arg_ip);
    m_addr = ipaddr;
    m_port = port;

    return 1;
}

UdpSocket::UdpSocket(const std::string &arg_ip, uint16_t port)
{
    connect(arg_ip, port);
}

int UdpSocket::bind(const std::string &arg_ip, uint16_t port)
{
    IpAddress ipaddr(arg_ip);
    struct sockaddr_in s_addr_bind;
    memset((char *)&s_addr_bind, 0, sizeof(s_addr_bind));

    s_addr_bind.sin_family = AF_INET;
    s_addr_bind.sin_port = htons(port);
    s_addr_bind.sin_addr.s_addr = inet_addr(arg_ip.c_str());

    return ::bind(m_fd, (struct sockaddr *)&s_addr_bind, sizeof(s_addr_bind));
}

int UdpSocket::sendTo(const void *data, std::size_t siz, const std::string &arg_ip,
                      uint16_t port)
{
    struct sockaddr_in sa_snd;
    memset((char *)&sa_snd, 0, sizeof(sa_snd));

    sa_snd.sin_family = AF_INET;
    sa_snd.sin_port = htons(port);
    inet_aton(arg_ip.c_str(), &sa_snd.sin_addr);

    return sendto(m_fd, data, siz, 0, (struct sockaddr *)&sa_snd, sizeof(sa_snd));
}

int UdpSocket::recvFrom(void *data, size_t buf_siz, std::string &arg_ip, uint16_t &port)
{
    struct sockaddr_in sa_rcv;
    std::memset((char *)&sa_rcv, 0, sizeof(sa_rcv));
    uint32_t siz_recv = sizeof(sa_rcv);

    const int res = recvfrom(m_fd, data, buf_siz, 0, (struct sockaddr *)&sa_rcv, &siz_recv);

    if (res < 0)
        return res;

    arg_ip = inet_ntoa(sa_rcv.sin_addr);
    port = ntohs(sa_rcv.sin_port);

    return res;
}

UdpSocket::~UdpSocket()
{
    close(m_fd);
}
