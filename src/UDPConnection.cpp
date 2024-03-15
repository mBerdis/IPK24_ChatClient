#include "UDPConnection.h"
#include <iostream>
#include <cerrno>
#include <cstring>
#include <sstream>
#include "Exception/ConnectionException.h"

static uint16_t messageID = 0;

static std::string id_to_str(uint16_t id)
{
    std::stringstream str;
    uint16_t networkMessageID = htons(id);   // convert it to network byte order
    unsigned char byte1 = (networkMessageID >> 8) & 0xFF;   // get first byte
    unsigned char byte2 = networkMessageID & 0xFF;          // get second byte

    str << byte1 << byte2;
    return str.str();
}

static uint16_t read_msgID(std::istringstream& iss)
{
    char buffer[2];
    iss.read(buffer, sizeof(buffer));
    // interpret the buffer as a uint16_t
    uint16_t networkMessageID = (static_cast<uint16_t>(buffer[0]) << 8) | static_cast<uint16_t>(buffer[1]);

    return ntohs(networkMessageID);
}

static MessageType read_msgType(std::istringstream& iss)
{
    uint8_t typeValue;
    iss >> typeValue;

    // cast the value to MessageType
    return static_cast<MessageType>(typeValue);
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
    
    // SIGINT could be signalled which would stop sending bye message, unset it, we are ending anyway
    signal_received = 0;

    std::stringstream udpMsg;
    udpMsg << BYE << id_to_str(messageID);

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

/*
*   bypassing traditional send_msg function, because we want to send only one confirm message and also dont increment messageID
*/
void UDPConnection::send_confirm(uint16_t receivedID)
{
    std::stringstream udpMsg;
    udpMsg << CONFIRM << id_to_str(receivedID);
    std::string msg = udpMsg.str();

    int bytesSent = sendto(clientSocket, msg.c_str(), msg.length(), 0, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (bytesSent < 0)
        throw ClientException("Error sending message!");
}

MessageType UDPConnection::process_msg(std::string& msg)
{
    std::istringstream iss(msg);

    MessageType receivedType = read_msgType(iss);
    uint16_t    receivedID   = read_msgID(iss);

    if (receivedType != CONFIRM)
        send_confirm(receivedID);
    
    switch (receivedType)
    {
        case CONFIRM:
            // check if receivedID is the same as ID of the last message sent
            if (receivedID != messageID - 1)
                return INTERNAL_ERR;
            return CONFIRM;  // will signal correct confirmation

        case REPLY:
        {
            char result;
            iss >> result;

            uint16_t refID = read_msgID(iss);

            std::string content;
            std::getline(iss, content, '\0');  // read until the \0

            // check if receivedID is the same as ID of the last non-confirm message sent
            if (refID != messageID - 1)
                return INTERNAL_ERR;

            if (result == 1)
            {
                std::cerr << "Success: " << content << "\n";
                return OK;
            }   
            else if (result == 0)
            {
                std::cerr << "Failure: " << content << "\n";
                return NOK;
            }
            else 
            {
                send_error("Unexpected result value.");
                std::cerr << "ERR: Unexpected result value.\n";
                return ERR;
            }
        }

        case MSG:
        {
            std::string name, content;
            std::getline(iss, name,    '\0');  // read until the \0
            std::getline(iss, content, '\0');  // read until the \0
            std::cout << name << ": " << content << "\n";
            break;
        }

        case ERR:
        {
            std::string name, content;
            std::getline(iss, name,    '\0');  // read until the \0
            std::getline(iss, content, '\0');  // read until the \0
            std::cout << "ERR FROM " << name << ": " << content << "\n";
            break;
        }

        case BYE: set_state(INIT);    // will not send bye message when terminating
            break;

        default:
            send_error("Unrecognized message from server.");
            std::cerr << "ERR: Unrecognized message from server.\n";
            return ERR;
    }

    return receivedType;
}

void UDPConnection::msg(std::string msg)
{
    if (state != OPEN)
    {
        std::cerr << "ERR: Auth first!\n" << std::flush;
        std::cerr << "Current state: " << state << "\n" << std::flush;
        return;
    }

    std::stringstream udpMsg;
    udpMsg << MSG << id_to_str(messageID) << displayName << '\0' << msg << '\0';
    send_msg(udpMsg.str());
}

MessageType UDPConnection::receive_msg()
{
    char buffer[BUFFER_SIZE];

    // Receive data from the server
    socklen_t addrLenght = sizeof(serverAddress);
    int bytes_read = recvfrom(clientSocket, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&serverAddress, &addrLenght);
    if (bytes_read <= 0)
    {
        std::cout << "ERR: nothing received!\n";
        return ERR; // Return early if an error occurred
    }

    // Append received data to the response string
    std::string response;
    response.append(buffer, bytes_read);

    return process_msg(response);
}

void UDPConnection::join_channel(std::string& channelID)
{
    if (state != OPEN)
    {
        std::cerr << "ERR: Auth first!\n" << std::flush;
        return;
    }

    // send join message
    std::stringstream udpMsg;
    udpMsg << JOIN << id_to_str(messageID) << channelID << '\0' << displayName << '\0';
    send_msg(udpMsg.str());

    // wait for reply
    while (!signal_received)
    {
        // wait REPLY_TIMEOUT ms until an event occurs
        if (poll(fds, 1, REPLY_TIMEOUT) <= 0)  // poll was interrupted
            break;

        // Check if there's data to read from the socket
        if (fds[0].revents & POLLIN)
        {
            switch (receive_msg())
            {
                case ERR: throw ClientException();
                case NOK: return;
                case OK:  return;
                default:  break;
            }
        }
    }
}

void UDPConnection::auth(std::string& username, std::string& secret)
{
    if (state == OPEN)
    {
        std::cerr << "ERR: You are already authorized!\n" << std::flush;
        return;
    }

    set_state(TRY_AUTH);

    // send auth message
    std::stringstream udpMsg;
    udpMsg << AUTH << id_to_str(messageID) << username << '\0' << displayName << '\0' << secret << '\0';
    send_msg(udpMsg.str());

    // wait for reply
    while (!signal_received)
    {
        // wait REPLY_TIMEOUT ms until an event occurs
        if (poll(fds, 1, REPLY_TIMEOUT) <= 0)  // poll was interrupted
            break;

        // Check if there's data to read from the socket
        if (fds[0].revents & POLLIN)
        {
            switch (receive_msg())
            {
            case ERR: throw ClientException();
            case NOK: return;
            case OK:  set_state(OPEN); return;
            default:  break;
            }
        }
    }
}

void UDPConnection::send_error(std::string msg)
{
    if (state != OPEN)
    {
        std::cerr << "ERR: Auth first!\n" << std::flush;
        return;
    }

    std::stringstream udpMsg;
    udpMsg << ERR << id_to_str(messageID) << displayName << '\0' << msg << '\0';
    send_msg(udpMsg.str());
}
