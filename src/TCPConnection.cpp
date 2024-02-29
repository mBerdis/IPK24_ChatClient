#include "TCPConnection.h"
#include <iostream>
#include <cerrno>
#include <cstring>

TCPConnection::TCPConnection(ConnectionSettings& conSettings): AbstractConnection(AF_INET, SOCK_STREAM, conSettings)
{
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) != 0)
    {
        std::cout << "Server IP Address: " << inet_ntoa(serverAddress.sin_addr) << std::endl;

        std::cerr << "Error connecting to the server: " << strerror(errno) << std::endl;
        std::cerr << "Error code: " << errno << std::endl;
        //#TODO: throw exception
    }

    std::string authMsg = "AUTH {Username} USING {Secret}\r\n";
    send_msg(authMsg);

    // init connetion with the server
	return;
}

TCPConnection::~TCPConnection()
{
    //TODO: terminate connection!
    return;
}

void TCPConnection::send_msg(std::string msg)
{
    int bytesSent = send(clientSocket, msg.c_str(), msg.length(), 0);
    if (bytesSent < 0) {
        std::cerr << "Error sending message\n";
    }
    return;
}

void TCPConnection::receive_msg()
{
    return;
}