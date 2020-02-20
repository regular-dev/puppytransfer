#include "IpAddress.hpp"

IpAddress::IpAddress() :
    m_address(0)
{

}

IpAddress::IpAddress(const char *address) :
    m_address(IpToInt(address))
{
}

IpAddress::IpAddress(const std::string &address) :
    m_address(IpToInt(address))
{
}

IpAddress::IpAddress(uint32_t address) :
    m_address(htonl(address))
{
}

IpAddress::IpAddress(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3) :
    m_address(htonl((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3))
{
}

uint32_t IpAddress::IpToInt(const std::string &address)
{
    if (address == "255.255.255.255") {
        // The broadcast address needs to be handled explicitely,
        // because it is also the value returned by inet_addr on error
        return INADDR_BROADCAST;
    } else {
        // Try to convert the address as a byte representation ("xxx.xxx.xxx.xxx")
        uint32_t ip = inet_addr(address.c_str());
        if (ip != INADDR_NONE)
            return ip;

        // Not a valid address, try to convert it as a host name
        addrinfo hints;
        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        addrinfo *result = NULL;
        if (getaddrinfo(address.c_str(), NULL, &hints, &result) == 0) {
            if (result) {
                ip = reinterpret_cast<sockaddr_in *>(result->ai_addr)->sin_addr.s_addr;
                freeaddrinfo(result);
                return ip;
            }
        }
        // Not a valid address nor a host name
        return 0;
    }

}

const IpAddress IpAddress::None;
const IpAddress IpAddress::LocalHost(127, 0, 0, 1);
const IpAddress IpAddress::Broadcast(255, 255, 255, 255);
const IpAddress IpAddress::Any(0, 0, 0, 0);

std::string IpAddress::toString() const
{
    in_addr address;
    address.s_addr = m_address;

    return inet_ntoa(address);
}

uint32_t IpAddress::toInteger() const
{
    return ntohl(m_address);
}

bool operator !=(const IpAddress &left, const IpAddress &right)
{
    return !(left == right);
}

bool operator <=(const IpAddress &left, const IpAddress &right)
{
    return !(right < left);
}


bool operator >=(const IpAddress &left, const IpAddress &right)
{
    return !(left < right);
}

bool operator ==(const IpAddress &left, const IpAddress &right)
{
    return left.toInteger() == right.toInteger();
}

bool operator<(const IpAddress &left, const IpAddress &right)
{
    return left.toInteger() < right.toInteger();
}


bool operator>(const IpAddress &left, const IpAddress &right)
{
    return right.toInteger() < left.toInteger();
}
