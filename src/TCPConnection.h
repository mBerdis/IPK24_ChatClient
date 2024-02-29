#pragma once
#include "AbstractConnection.h"

class TCPConnection : public AbstractConnection
{
	public:
		TCPConnection(ConnectionSettings& conSettings);
		~TCPConnection();
		void send_msg(std::string msg) override;
		void receive_msg() override;
};

