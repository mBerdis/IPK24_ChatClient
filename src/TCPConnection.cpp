#include "TCPConnection.h"
#include <iostream>
#include <cerrno>
#include <cstring>
#include <sstream>
#include "Exception/ConnectionException.h"

TCPConnection::TCPConnection(ConnectionSettings& conSettings): AbstractConnection(AF_INET, SOCK_STREAM, conSettings)
{
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) != 0)
    {
        std::stringstream ss;
        ss << "Error connecting to the server: " << strerror(errno) << ". ERR_CODE: " << errno;
        throw ConnectionException(ss.str());
    }      
}

TCPConnection::~TCPConnection()
{
    if (state != INIT)
        send_msg("BYE\r\n");
}

void TCPConnection::msg(std::string msg)
{
    if (state != OPEN)
    {
        print_err("Auth first!");
        return;
    }

    std::stringstream tcpMsg;
    tcpMsg << "MSG FROM " << displayName << " IS " << msg << "\r\n"; // MSG FROM {DisplayName} IS {MessageContent}\r\n
    send_msg(tcpMsg.str());
}

void TCPConnection::send_msg(std::string msg)
{
    int bytesSent = send(clientSocket, msg.c_str(), msg.length(), 0);
    if (bytesSent < 0) 
        print_err("Error sending message!");

    return;
}

MessageType TCPConnection::process_msg(std::string& msg)
{
    std::istringstream iss(msg);
    std::string firstWord;
    iss >> firstWord;       // will read until first whitespace
    
    if (eq_kw(firstWord, "REPLY"))
    {
        std::string confirmation, keyword, reason;
        iss >> confirmation;    // OK or NOK
        iss >> keyword;         // IS
        std::getline(iss >> std::ws, reason); // read the rest of the line, skipping leading whitespace
        
        if (!eq_kw(keyword, "IS"))
        {
            send_error("Unrecognized message from server.");
            print_err("Unrecognized message from server!");
            return ERR;
        }
        
        if (eq_kw(confirmation, "OK"))
        {
            std::cerr << "Success: " << reason << "\n";
            return OK;
        }
        else if (eq_kw(confirmation, "NOK"))
        {
            std::cerr << "Failure: " << reason << "\n";
            return NOK;
        }
        else 
        {
            send_error("Unrecognized REPLY message from the server.");
            print_err("Unrecognized REPLY message from the server!");
            return ERR;
        }
    }
    else if (eq_kw(firstWord, "MSG"))
    {
        std::string keyword, name, keyword2, content;
        iss >> keyword;         // FROM
        iss >> name;            // name
        iss >> keyword2;        // IS
        std::getline(iss >> std::ws, content); // read the rest of the line, skipping leading whitespace
        
        if (!eq_kw(keyword, "FROM") || !eq_kw(keyword2, "IS") || name == "")
        {
            send_error("Unrecognized message from server.");
            print_err("Unrecognized message from server!");
            return ERR;
        }

        print_msg(name, content);
        return MSG;
    }
    else if (eq_kw(firstWord, "ERR"))
    {
        std::string keyword, name, keyword2, message;
        iss >> keyword;         // FROM
        iss >> name;            // name
        iss >> keyword2;        // IS
        std::getline(iss >> std::ws, message); // read the rest of the line, skipping leading whitespace

        if (!eq_kw(keyword, "FROM") || !eq_kw(keyword2, "IS") || name == "")
        {
            send_error("Unrecognized message from server.");
            print_err("Unrecognized message from server!");
            return ERR;
        }

        std::cerr << "ERR FROM " << name << ": " << message << "\n";
        return ERR;
    }
    else if (eq_kw(firstWord, "BYE"))
    {
        set_state(INIT); // changing to INIT state, because in this state destructor doesnt send BYE
        return BYE;
    }
    else
    {
        send_error("Unrecognized message from server.");
        print_err("Unrecognized message from server!");
        return ERR;
    }
}

MessageType TCPConnection::receive_msg()
{
    char buffer[BUFFER_SIZE];
    
    // Receive data from the server
    int bytes_read = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (bytes_read <= 0) {
        print_err("Nothing received!");
        return ERR; // Return early if an error occurred
    }

    // Append received data to the response string
    std::string response;
    response.append(buffer, bytes_read);

    return process_msg(response);
}

void TCPConnection::join_channel(std::string& channelID)
{
    if (state != OPEN)
    {
        print_err("Auth first!");
        return;
    }

    // send join message
    std::stringstream tcpMsg;
    tcpMsg << "JOIN " << channelID << " AS " << displayName << "\r\n"; // JOIN {ChannelID} AS {DisplayName}\r\n
    send_msg(tcpMsg.str());

    await_message(REPLY, REPLY_TIMEOUT);
}

void TCPConnection::auth(std::string& username, std::string& secret)
{
    if (state == OPEN)
    {
        print_err("You are already authorized!");
        return;
    }

    set_state(TRY_AUTH);

    // send auth message
    std::stringstream tcpMsg;

    // AUTH {Username} AS {DisplayName} USING {Secret}\r\n
    tcpMsg << "AUTH " << username << " AS " << displayName << " USING " << secret << "\r\n";
    send_msg(tcpMsg.str());

    if (await_message(REPLY, REPLY_TIMEOUT) == OK)
        set_state(OPEN);
}

void TCPConnection::send_error(std::string msg)
{
    if (state != OPEN)
    {
        print_err("Auth first!");
        return;
    }

    std::stringstream tcpMsg;
    tcpMsg << "ERR FROM " << displayName << " IS " << msg << "\r\n"; // ERR FROM {DisplayName} IS {MessageContent}\r\n
    send_msg(tcpMsg.str());
}
