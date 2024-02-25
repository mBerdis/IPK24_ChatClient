#pragma once
#include <string>

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

class AbstractConnection
{
	public:
		AbstractConnection(int family, int socketType, std::string serverIP);
		~AbstractConnection();
		virtual void send_msg(std::string msg)	= 0;		// pure virtual
		virtual void receive_msg()				= 0;		// pure virtual

	protected:
		const int serverPort = 12345;
		int clientSocket;
		struct sockaddr_in serverAddress;
#ifdef _WIN32
		WSADATA wsaData;
#endif
};

