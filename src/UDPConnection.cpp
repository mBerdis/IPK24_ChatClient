#include "UDPConnection.h"
#include <iostream>

UDPConnection::UDPConnection(ConnectionSettings& conSettings): 
    AbstractConnection(AF_INET, SOCK_DGRAM, conSettings), 
    udpRetry{conSettings.udpRetry},
    udpTimeout{conSettings.udpTimeout}
{
    // init connetion with the server
    return;
}

UDPConnection::~UDPConnection()
{
    //TODO: terminate connection!
    return;
}

void UDPConnection::send_msg(std::string msg)
{ 
    int bytesSent = sendto(clientSocket, msg.c_str(), msg.length(), 0, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (bytesSent < 0) {
        std::cerr << "Error sending message\n";
    }
    return;
}

void UDPConnection::msg(std::string msg)
{
}

void UDPConnection::receive_msg()
{
    return;
}

void UDPConnection::join_channel(std::string& channelID)
{
}

void UDPConnection::auth(std::string& username, std::string& secret)
{
}
