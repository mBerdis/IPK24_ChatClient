#pragma once
#include "ClientException.h"

class ConnectionException : public ClientException
{
public:
    ConnectionException(const std::string& message = "Connection error!") : ClientException(message) {}
};