#pragma once
#include "AbstractConnection.h"

class UDPConnection : public AbstractConnection
{
	public:
		UDPConnection(std::string serverIP);
		~UDPConnection();
		void send_msg(std::string msg) override;
		void receive_msg() override;
};

