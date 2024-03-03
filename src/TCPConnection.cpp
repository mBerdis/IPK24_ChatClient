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

void TCPConnection::receive_msg() {
    char buffer[1600];
    std::string response;

    // Receive data from the server
    int bytes_rx = recv(clientSocket, buffer, 1600, 0);
    if (bytes_rx < 0) {
        perror("ERROR: recv");
        return; // Return early if an error occurred
    }

    // Append received data to the response string
    response.append(buffer, bytes_rx);

    // Print the received message
    std::cout << "Received message from server: " << response << std::endl;
}

void TCPConnection::join_channel(std::string& channelID)
{
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

    // if got confirmation
    set_state(OPEN);
}
