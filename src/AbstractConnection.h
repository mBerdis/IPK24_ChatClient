#pragma once
#include <string>
#include "ConnectionSettings.h"
#include "common.h"

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
#include <csignal>

#define BUFFER_SIZE   1600	// buffer for reading message from socket 1500 + reserve
#define REPLY_TIMEOUT 5000	// connection will wait 5 sec for reply msg
extern volatile sig_atomic_t signal_received;

enum MessageType: uint8_t
{
	CONFIRM = 0x00,
	REPLY	= 0x01,
	AUTH	= 0x02,
	JOIN	= 0x03,
	MSG		= 0x04,
	ERR		= 0xFE,
	BYE		= 0xFF,
	// flow control, not actuall message types
	OK		= 0xF1,
	NOK		= 0xF2,
	INTERNAL_ERR = 0xF3	// used as value for returning out of function that failed
};

enum ConnectionState
{
	OPEN,		// can send and receive
	CLOSED,		// terminating connection
	INIT,		// not yet authorized
	TRY_AUTH	// trying to auth
};

class AbstractConnection
{
	public:
		AbstractConnection(int family, int socketType, ConnectionSettings& conSettings);
		virtual ~AbstractConnection();

		virtual void msg(std::string msg) = 0;				// pure virtual, creates message and sends it using send_msg()
		virtual MessageType receive_msg() = 0;				// pure virtual
		virtual void join_channel(std::string& channelID) = 0;				// pure virtual
		virtual void auth(std::string& username, std::string& secret) = 0;	// pure virtual
		virtual void send_error(std::string msg) = 0;						// pure virtual

		int get_socket();
		void set_displayName(std::string name);
		void set_state(ConnectionState conState);

	protected:
		virtual void send_msg(std::string msg) = 0;				// pure virtual, internally used for sending messages to server
		virtual MessageType process_msg(std::string& msg) = 0;	// pure virtual, internally used for processing messages from server
		
		/*
		* Function returns MessageType because of REPLY's OK/NOK message
		* In case of OK, caller might want to do something
		* in case of AUTH caller will set state to OPEN
		*/
		MessageType await_message(const MessageType waitingFor, uint32_t timeout);

		ConnectionState state;
		int serverPort;
		int clientSocket;
		struct sockaddr_in serverAddress;
		std::string displayName;
		struct pollfd fds[1];
#ifdef _WIN32
		WSADATA wsaData;
#endif
};