#ifndef UDTLISTENER_HPP
#define UDTLISTENER_HPP

#include "UdtSocket.hpp"

class UdtListener
{

public :
    UdtListener();
    UdtListener(uint16_t port);
    ~UdtListener();

    int listen(uint16_t port);
    UdtSocket accept();

private :
    UDTSOCKET m_socket;
    uint16_t m_port;

};

#endif
