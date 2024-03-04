#pragma once
#include <string>

#define DEFAULT_SERVER_PORT 4567
#define DEFAULT_UDP_TIMEOUT 250
#define DEFAULT_UDP_RETRY 3

enum class ProtocolType 
{
    UNDEFINED,
    TCP,
    UDP
};

struct ConnectionSettings 
{
    std::string serverAdress;
    ProtocolType protType;
    uint16_t serverPort;
    uint16_t udpTimeout;
    uint8_t  udpRetry;

    ConnectionSettings()    // setting default values when creating instance
    {
        protType    = ProtocolType::UNDEFINED;
        serverPort  = DEFAULT_SERVER_PORT;
        udpTimeout  = DEFAULT_UDP_TIMEOUT;
        udpRetry    = DEFAULT_UDP_RETRY;
    }
};
