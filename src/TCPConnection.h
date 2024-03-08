#pragma once
#include "AbstractConnection.h"

class TCPConnection : public AbstractConnection
{
	public:
		TCPConnection(ConnectionSettings& conSettings);
		~TCPConnection() override;

		void msg(std::string msg) override;
		MessageType receive_msg() override;
		void join_channel(std::string& channelID) override;
		void auth(std::string& username, std::string& secret) override;
		void send_error(std::string msg) override;

	protected:
		void send_msg(std::string msg) override;
		MessageType process_msg(std::string& msg) override;
};

