#include "UDPConnection.h"
#include <iostream>
#include <cerrno>
#include <cstring>
#include <sstream>
#include "Exception/ConnectionException.h"

std::string get_msgID()
{
    std::stringstream str;
    uint16_t networkMessageID = htons(messageID);   // convert it to network byte order
    unsigned char byte1 = (networkMessageID >> 8) & 0xFF;   // get first byte
    unsigned char byte2 = networkMessageID & 0xFF;          // get second byte

    str << byte1 << byte2;
    return str.str();
}

UDPConnection::UDPConnection(ConnectionSettings& conSettings): 
    AbstractConnection(AF_INET, SOCK_DGRAM, conSettings), 
    udpRetry{conSettings.udpRetry},
    udpTimeout{conSettings.udpTimeout}
{}

UDPConnection::~UDPConnection()
{
    if (state == INIT)
        return;
    
    std::stringstream udpMsg;
    udpMsg << BYE << get_msgID();

    // send_msg can throw exception if sending failed, but we will exit anyway
    try
    {
        send_msg(udpMsg.str());
    }
    catch (const ClientException&)
    {
        return;
    }
}

void UDPConnection::send_msg(std::string msg)
{ 
    messageID++;    // increment counter for the next messageID

    // try sending msg, max (1 + udpRetry) times
    for (size_t i = 0; i <= udpRetry; i++)
    {
        if (signal_received)    // signal could be given while in inner while cycle
            return;

        int bytesSent = sendto(clientSocket, msg.c_str(), msg.length(), 0, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
        if (bytesSent < 0)
            throw ClientException("Error sending message!");

        while (!signal_received)
        {
            // wait udpTimeout ms until an event occurs
            if (poll(fds, 1, udpTimeout) <= 0)  // poll was interrupted
                break;

            // Check if there's data to read from the socket
            if (fds[0].revents & POLLIN)
            {
                switch (receive_msg())
                {
                case ERR:     throw ClientException();
                case CONFIRM: return;
                default:      break;
                }
            }
        }
    }

    throw ClientException("Error sending message! Maximum udpRetry reached.");
}

MessageType UDPConnection::process_msg(std::string& msg)
{
    // TODO: 
    return MSG;
}

void UDPConnection::msg(std::string msg)
{
    if (state != OPEN)
    {
        std::cerr << "ERR: Auth first!\n" << std::flush;
        return;
    }

    std::stringstream udpMsg;
    udpMsg << MSG << get_msgID() << displayName << '\0' << msg << '\0';
    send_msg(udpMsg.str());
}

MessageType UDPConnection::receive_msg()
{
    return MSG;
}

void UDPConnection::join_channel(std::string& channelID)
{
    // TODO: 
}

void UDPConnection::auth(std::string& username, std::string& secret)
{
    // TODO: 
    set_state(OPEN);
}

void UDPConnection::send_error(std::string msg)
{
    if (state != OPEN)
    {
        std::cerr << "ERR: Auth first!\n" << std::flush;
        return;
    }

    std::stringstream udpMsg;
    udpMsg << ERR << get_msgID() << displayName << '\0' << msg << '\0';
    send_msg(udpMsg.str());
}
