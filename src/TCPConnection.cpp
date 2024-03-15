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
        send_msg("BYE\r\n");
}

void TCPConnection::msg(std::string msg)
{
    if (state != OPEN)
    {
        std::cerr << "ERR: Auth first!\n" << std::flush;
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
        std::cerr << "ERR: Error sending message\n";
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
        iss >> confirmation;    // OK or NOK
        iss >> keyword;         // IS
        std::getline(iss >> std::ws, reason); // read the rest of the line, skipping leading whitespace

        if (keyword != "IS")
        {
            send_error("Unrecognized message from server.");
            std::cerr << "ERR: Unrecognized message from server.\n";
            return ERR;
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
            send_error("Unrecognized REPLY message from the server.");
            std::cerr << "ERR: Unrecognized REPLY message from the server.\n";
            return ERR;
        }
    }
    else if (firstWord == "MSG")
    {
        std::string keyword, displayName, keyword2, message;
        iss >> keyword;         // FROM
        iss >> displayName;     // name
        iss >> keyword2;        // IS
        std::getline(iss >> std::ws, message); // read the rest of the line, skipping leading whitespace

        if (keyword != "FROM" && keyword2 != "IS")
        {
            send_error("Unrecognized message from server.");
            std::cerr << "ERR: Unrecognized message from server.\n";
            return ERR;
        }

        std::cout << displayName << ": " << message << "\n";
        return MSG;
    }
    else if (firstWord == "ERR")
    {
        std::string keyword, displayName, keyword2, message;
        iss >> keyword;         // FROM
        iss >> displayName;     // name
        iss >> keyword2;        // IS
        std::getline(iss >> std::ws, message); // read the rest of the line, skipping leading whitespace

        if (keyword != "FROM" || keyword2 != "IS")
        {
            send_error("Unrecognized message from server.");
            std::cerr << "ERR: Unrecognized message from server.\n";
            return ERR;
        }

        std::cout << "ERR FROM " << displayName << ": " << message << "\n";
        return ERR;
    }
    else if (firstWord == "BYE")
    {
        set_state(INIT); // changing to INIT state, because in this state destructor doesnt send BYE
        return BYE;
    }
    else
    {
        send_error("Unrecognized message from server.");
        std::cerr << "ERR: Unrecognized message from server.\n";
        return ERR;
    }
}

MessageType TCPConnection::receive_msg()
{
    char buffer[BUFFER_SIZE];
    
    // Receive data from the server
    int bytes_read = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (bytes_read <= 0) {
        std::cout << "ERR: nothing received!\n";
        return ERR; // Return early if an error occurred
    }

    // Append received data to the response string
    std::string response;
    response.append(buffer, bytes_read);

    return process_msg(response);
}

void TCPConnection::join_channel(std::string& channelID)
{
    if (state != OPEN)
    {
        std::cerr << "ERR: Auth first!\n" << std::flush;
        return;
    }

    // send join message
    std::stringstream tcpMsg;
    tcpMsg << "JOIN " << channelID << " AS " << displayName << "\r\n"; // JOIN {ChannelID} AS {DisplayName}\r\n
    send_msg(tcpMsg.str());

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

void TCPConnection::auth(std::string& username, std::string& secret)
{
    if (state == OPEN)
    {
        std::cerr << "ERR: You are already authorized!\n" << std::flush;
        return;
    }

    set_state(TRY_AUTH);

    // send auth message
    std::stringstream tcpMsg;

    // AUTH {Username} AS {DisplayName} USING {Secret}\r\n
    tcpMsg << "AUTH " << username << " AS " << displayName << " USING " << secret << "\r\n";
    send_msg(tcpMsg.str());

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

void TCPConnection::send_error(std::string msg)
{
    if (state != OPEN)
    {
        std::cerr << "ERR: Auth first!\n" << std::flush;
        return;
    }

    std::stringstream tcpMsg;
    tcpMsg << "ERR FROM " << displayName << " IS " << msg << "\r\n"; // ERR FROM {DisplayName} IS {MessageContent}\r\n
    send_msg(tcpMsg.str());
}
