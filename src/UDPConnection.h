#pragma once
#include "AbstractConnection.h"

class UDPConnection : public AbstractConnection
{
	public:
		UDPConnection(ConnectionSettings& conSettings);
		~UDPConnection();
		void send_msg(std::string msg) override;
		void receive_msg() override;

	private:
		int udpRetry;
		int udpTimeout;
};

