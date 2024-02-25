#include "AbstractConnection.h"


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

AbstractConnection::AbstractConnection(int family, int socketType, std::string serverIP)
{
#ifdef _WIN32
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup failed\n";
		exit(EXIT_FAILURE);
	}
#endif

	clientSocket = socket(family, socketType, 0);
	if (clientSocket <= 0)
	{
		std::cerr << "ERROR: socket\n";
		exit(EXIT_FAILURE);
	}

	serverAddress.sin_family = family;
	serverAddress.sin_port   = htons(serverPort);
	inet_pton(family, serverIP.c_str(), &serverAddress.sin_addr);
}

AbstractConnection::~AbstractConnection()
{
	close(clientSocket);
#ifdef _WIN32
     WSACleanup();
 #endif

	 std::cout << "Connection terminated successfully!\n";
}
