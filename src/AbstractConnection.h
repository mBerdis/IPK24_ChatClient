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

class AbstractConnection
{
	public:
		AbstractConnection(int family, int socketType, ConnectionSettings& conSettings);
		~AbstractConnection();

		virtual void send_msg(std::string msg)	 = 0;		// pure virtual
		virtual void receive_msg()				 = 0;		// pure virtual
		virtual void join_channel(std::string& channelID) = 0;				// pure virtual
		virtual void auth(std::string& username, std::string& secret) = 0;	// pure virtual

		int get_socket();
		void set_displayName(std::string name);

	protected:
		int serverPort;
		int clientSocket;
		struct sockaddr_in serverAddress;
		std::string displayName;
#ifdef _WIN32
		WSADATA wsaData;
#endif
};

