#pragma once
#include <string>
#include "ConnectionSettings.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <cstdio>
//typedef int ssize_t;
#define poll WSAPoll
#define close closesocket
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>
#endif

enum ConnectionState
{
	OPEN,		// can send and receive
	CLOSED,		// terminating connection
	INIT		// not yet authorized
};

class AbstractConnection
{
	public:
		AbstractConnection(int family, int socketType, ConnectionSettings& conSettings);
		virtual ~AbstractConnection();

		virtual void msg(std::string msg) = 0;				// pure virtual, creates message and sends it using send_msg()
		virtual void receive_msg()				 = 0;		// pure virtual
		virtual void join_channel(std::string& channelID) = 0;				// pure virtual
		virtual void auth(std::string& username, std::string& secret) = 0;	// pure virtual

		int get_socket();
		void set_displayName(std::string name);
		void set_state(ConnectionState conState);

	protected:
		virtual void send_msg(std::string msg) = 0;			// pure virtual, internally used for sending messages to server

		ConnectionState state;
		int serverPort;
		int clientSocket;
		struct sockaddr_in serverAddress;
		std::string displayName;
#ifdef _WIN32
		WSADATA wsaData;
#endif
};

