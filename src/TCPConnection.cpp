#include "TCPConnection.h"
#include <iostream>
#include <cerrno>
#include <cstring>
#include <sstream>
#include "Exception/ConnectionException.h"

TCPConnection::TCPConnection(ConnectionSettings& conSettings): AbstractConnection(AF_INET, SOCK_STREAM, conSettings)
{
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) != 0)
    {
        std::stringstream ss;
        ss << "Error connecting to the server: " << strerror(errno) << ". ERR_CODE: " << errno;
        throw ConnectionException(ss.str());
    }      
}

TCPConnection::~TCPConnection()
{
    if (state != INIT)
    {
        send_msg("BYE\r\n");
    }
 
    std::cout << "Later\n" << std::flush;
}

void TCPConnection::msg(std::string msg)
{
    if (state != OPEN)
    {
        std::cout << "Auth first!";
        return;
    }

    std::stringstream tcpMsg;
    tcpMsg << "MSG FROM " << displayName << " IS " << msg << "\r\n"; // MSG FROM {DisplayName} IS {MessageContent}\r\n
    send_msg(tcpMsg.str());
}

void TCPConnection::send_msg(std::string msg)
{
    int bytesSent = send(clientSocket, msg.c_str(), msg.length(), 0);
    if (bytesSent < 0) {
        std::cerr << "Error sending message\n";
    }
    return;
}

MessageType TCPConnection::process_msg(std::string& msg)
{
    std::istringstream iss(msg);
    std::string firstWord;
    iss >> firstWord;       // will read until first whitespace

    if (firstWord == "REPLY")
    {
        std::string confirmation, keyword, reason;
        iss >> confirmation;
        iss >> keyword;
        std::getline(iss >> std::ws, reason); // read the rest of the line, skipping leading whitespace

        if (keyword != "IS")
        {
            // TODO: not sure if this is right
            return INTERNAL_ERR;
        }

        if (confirmation == "OK")
        {
            std::cerr << "Success: " << reason << "\n";
            return OK;
        }
        else if (confirmation == "NOK")
        {
            std::cerr << "Failure: " << reason << "\n";
            return NOK;
        }
        else 
        {
            // TODO: not sure if this is right
            return INTERNAL_ERR;
        }
    }
    else if (firstWord == "MSG")
    {
        std::string keyword, sender, displayName, message;
        iss >> keyword;
        iss >> sender;
        iss >> displayName;
        std::getline(iss >> std::ws, message); // read the rest of the line, skipping leading whitespace

        if (keyword != "FROM")
        {
            // TODO: not sure if this is right
            return INTERNAL_ERR;
        }

        std::cout << displayName << ": " << message << "\n";
        return MSG;
    }
    else if (firstWord == "ERR")
    {
        std::string keyword, displayName, message;
        iss >> keyword;
        iss >> displayName;
        std::getline(iss >> std::ws, message); // read the rest of the line, skipping leading whitespace

        if (keyword != "FROM")
        {
            // TODO: not sure if this is right
            return INTERNAL_ERR;
        }

        std::cout << "ERR FROM " << displayName << ": " << message << "\n";
        return ERR;
    }
    else
    {
        std::cerr << "Unrecognized message from server.";
        return INTERNAL_ERR;
    }
}

MessageType TCPConnection::receive_msg()
{
    char buffer[1600];
    
    // Receive data from the server
    int bytes_read = recv(clientSocket, buffer, 1600, 0);
    if (bytes_read < 0) {
        std::cout << "nothing received!\n";
        return INTERNAL_ERR; // Return early if an error occurred
    }

    // Append received data to the response string
    std::string response;
    response.append(buffer, bytes_read);

    // Print the received message
    std::cout << "DEBUG: Received message from server: " << response << "\n";

    return process_msg(response);
}

void TCPConnection::join_channel(std::string& channelID)
{
    if (state != OPEN)
    {
        std::cout << "Auth first!";
        return;
    }

    std::stringstream tcpMsg;
    tcpMsg << "JOIN " << channelID << " AS " << displayName << "\r\n"; // JOIN {ChannelID} AS {DisplayName}\r\n
    send_msg(tcpMsg.str());
}

void TCPConnection::auth(std::string& username, std::string& secret)
{
    std::stringstream tcpMsg;
    // AUTH {Username} AS {DisplayName} USING {Secret}\r\n
    tcpMsg << "AUTH " << username << " AS " << displayName << " USING " << secret << "\r\n";
    send_msg(tcpMsg.str());
    //TODO: wait for confirmation 
    
    //TODO: add timeout?
    while (true)
    {
        switch (receive_msg())
        {
            case ERR:
                return;
            case NOK:
                return;
            case OK:
                set_state(OPEN);
                return;
            default:
                break;
        }
    }
}
