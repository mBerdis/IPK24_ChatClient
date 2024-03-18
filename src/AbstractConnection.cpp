#include "AbstractConnection.h"


#include <iostream>
#include <cstring>
#include <string>
#include <stdio.h>
#include "Exception/ClientException.h"

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

	// setup server address
	serverAddress.sin_family = family;
	serverAddress.sin_port   = htons(serverPort);
	inet_pton(family, conSettings.serverAdress.c_str(), &serverAddress.sin_addr);

	// setup poll fds
	fds[0].fd = get_socket();
	fds[0].events = POLLIN;
}

AbstractConnection::~AbstractConnection()
{
	shutdown(clientSocket, SHUT_RDWR);
	close(clientSocket);
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

MessageType AbstractConnection::await_message(const MessageType waitingFor, uint32_t timeout)
{
	while (!signal_received)
	{
		// wait timeout ms until an event occurs
		if (poll(fds, 1, timeout) <= 0)  // poll was interrupted
			break;

		// Check if there's data to read from the socket
		if (fds[0].revents & POLLIN)
		{
			MessageType result = receive_msg();
			
			if (result == ERR)
				throw ClientException();

			if (result == waitingFor)	// got what we were waiting for
				return result;			// this will be used for awaiting CONFIRM

			if (waitingFor == REPLY && (result == OK || result == NOK))
				return result;	// waited for REPLY, got OK/NOK

			// else continue, await next message
		}
	}

	return INTERNAL_ERR;
}
