#pragma once
#include "ClientException.h"

class AuthException : public ClientException
{
    public:
        AuthException(const std::string& message = "Auth failed.") : ClientException(message) {}
};