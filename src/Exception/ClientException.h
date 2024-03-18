#pragma once
#include <iostream>
#include <exception>
#include "../common.h"

class ClientException : public std::exception 
{
    public:
        ClientException(const std::string& message = "Unknow error occured!") : m_message(message) { print_err(message); }

        const char* what() const noexcept override 
        {
            return m_message.c_str();
        }
    private:
        const std::string& m_message;
};