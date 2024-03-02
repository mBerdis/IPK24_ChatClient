#pragma once
#include "AbstractConnection.h"

class UDPConnection : public AbstractConnection
{
	public:
		UDPConnection(ConnectionSettings& conSettings);
		~UDPConnection();
		void send_msg(std::string msg) override;
		void receive_msg() override;
		void join_channel(std::string& channelID) override;
		void auth(std::string& username, std::string& secret) override;

	private:
		int udpRetry;
		int udpTimeout;
};

