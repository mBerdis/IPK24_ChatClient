#pragma once
#include <iostream>
#include <exception>

class ClientException : public std::exception 
{
    public:
        ClientException(const std::string& message = "Unknow error occured!") : m_message(message) { std::cerr << "ERR: " << message << "\n"; }

        const char* what() const noexcept override 
        {
            return m_message.c_str();
        }
    private:
        const std::string& m_message;
};