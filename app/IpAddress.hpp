/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Code used from SFML - Simple and Fast Multimedia Library *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#ifndef IPADDRESS_H
#define IPADDRESS_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <string>
#include <cstring>

class IpAddress
{
public :
    IpAddress();
    IpAddress(const std::string &address);
    IpAddress(const char *address);
    explicit IpAddress(uint32_t address);
    IpAddress(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3);

    std::string toString() const;
    uint32_t toInteger() const;
    static uint32_t IpToInt(const std::string &ip);

    static const IpAddress None;
    static const IpAddress LocalHost;
    static const IpAddress Broadcast;
    static const IpAddress Any;

private :
    uint32_t m_address; ///< Address stored as an unsigned 32 bits integer
};

bool operator ==(const IpAddress &left, const IpAddress &right);
bool operator !=(const IpAddress &left, const IpAddress &right);

bool operator <(const IpAddress &left, const IpAddress &right);
bool operator >(const IpAddress &left, const IpAddress &right);

bool operator <=(const IpAddress &left, const IpAddress &right);
bool operator >=(const IpAddress &left, const IpAddress &right);

#endif
