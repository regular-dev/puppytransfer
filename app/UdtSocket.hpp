#ifndef UDTSOCKET_H
#define UDTSOCKET_H

#include "udt/udt.h"

#include <string>

typedef int universal_socket_t;

class UdtListener;

class UdtSocket
{

public :
    UdtSocket();
    UdtSocket(UDTSOCKET &s);
    ~UdtSocket();

    int listen(uint16_t port);
    int bind(universal_socket_t existing_udp_socket); // doesn't work well
    int connect(const std::string &ip, uint16_t port_);
    inline bool connected() const { return m_is_connected; }

    int send(const char *data, size_t size_data);
    int recv(char *buf, size_t size_buf);

    void setNonBlocking(bool flag);
    bool NonBlockingMode() const;

private :
    UDTSOCKET m_socket;
    bool m_is_connected = false;

    std::string m_remote_addr;
    uint16_t m_remote_port;

    friend class UdtListener;
};

#endif
