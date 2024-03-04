#include "AbstractConnection.h"


#include <iostream>
#include <cstring>
#include <string>
#include <stdio.h>

// Global flag to indicate if interrupt signal was received
volatile sig_atomic_t signal_received = 0;

AbstractConnection::AbstractConnection(int family, int socketType, ConnectionSettings& conSettings): 
	state{ INIT },
	serverPort{conSettings.serverPort}
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
	inet_pton(family, conSettings.serverAdress.c_str(), &serverAddress.sin_addr);
}

AbstractConnection::~AbstractConnection()
{
	close(clientSocket);
	std::cout << "Connection terminated successfully!\n" << std::flush;
}

int AbstractConnection::get_socket()
{
	return clientSocket;
}

void AbstractConnection::set_displayName(std::string name)
{
	displayName = name;
}

void AbstractConnection::set_state(ConnectionState conState)
{
	state = conState;
}
