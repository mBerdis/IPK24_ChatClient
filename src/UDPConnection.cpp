#include <iostream>
#include <cstring>
#include <string>
#include <stdio.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <cstdio>
typedef int ssize_t;
#define poll WSAPoll
#define close closesocket
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>
#endif


#include "UDPConnection.h"

UDPConnection::UDPConnection(std::string serverIP): AbstractConnection(AF_INET, SOCK_DGRAM, serverIP)
{
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

void UDPConnection::receive_msg()
{
    return;
}
