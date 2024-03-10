#pragma once
#include "AbstractConnection.h"

class UDPConnection : public AbstractConnection
{
	public:
		UDPConnection(ConnectionSettings& conSettings);
		~UDPConnection();

		void msg(std::string msg) override;
		MessageType receive_msg() override;
		void join_channel(std::string& channelID) override;
		void auth(std::string& username, std::string& secret) override;
		void send_error(std::string msg) override;

	protected:
		void send_msg(std::string msg) override;
		void send_confirm(uint16_t receivedID);
		MessageType process_msg(std::string& msg) override;

	private:
		uint32_t udpRetry;	// 1 initial + x retries
		uint32_t udpTimeout;
};

