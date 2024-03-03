#pragma once
#include "ClientException.h"

class TimeoutException : public ClientException 
{
    public:
        TimeoutException(const std::string& message = "Connection timeout!") : ClientException(message) {}
};