#include <string>
#include <unistd.h>
#include <cstring>
#include <poll.h>
#include <iostream>
#include <memory>
#include <csignal>
#include <netdb.h>
#include <sstream>
#include <regex>

#include "ConnectionSettings.h"
#include "AbstractConnection.h"
#include "UDPConnection.h"
#include "TCPConnection.h"
#include "Exception/ClientException.h"
#include "Exception/ConnectionException.h"
#include "common.h"

// Regular expressions

// #TODO remove when done testing (added . to join discord channels)
const std::regex GENERAL_REGEX("[A-Za-z0-9-.]+");     // Username, ChannelID, Secret; A-z0-9-

//const std::regex GENERAL_REGEX("[A-Za-z0-9-]+");     // Username, ChannelID, Secret; A-z0-9-
const std::regex PRINTABLE_REGEX("^[[:print:]]*$");  // MessageContent; Printable characters with space (0x20-7E)
const std::regex WHITESPACE_REGEX("[[:space:]]");    // for DisplayName; Printable characters (0x21-7E)

// max parameter lenghts
const uint8_t G_MAX_LEN = 20;   // general max lenght; Username, ChannelID, DisplayName
const uint8_t S_MAX_LEN = 128;  // secret  max lenght; Secret
const uint16_t M_MAX_LEN = 1400;// message max lenght; MessageContent

static bool match_dName(const std::string displayName)
{
    // Check if the input contains whitespace
    if (std::regex_search(displayName, WHITESPACE_REGEX))
        return false;

    // Check if the input contains only printable characters
    return std::regex_match(displayName, PRINTABLE_REGEX);
}

static std::string parse_ipAddress(std::string str)
{
    // Structure to hold the results of getaddrinfo
    struct addrinfo hints, * res;

    // Initialize hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family     = AF_INET;      // Using IPv4
    hints.ai_socktype   = SOCK_STREAM;  // Use TCP

    // Call getaddrinfo to get address info for the hostname
    int result = getaddrinfo(str.c_str(), nullptr, &hints, &res);
    if (result != 0) 
    {
        print_err("getaddrinfo failed!");
        exit(EXIT_FAILURE);
    }

    char address[INET_ADDRSTRLEN];
    struct sockaddr_in* ipv4 = reinterpret_cast<struct sockaddr_in*>(res->ai_addr);
    inet_ntop(AF_INET, &(ipv4->sin_addr), address, INET_ADDRSTRLEN);

    // Free memory allocated by getaddrinfo
    freeaddrinfo(res);

    return address;
}

static int parse_args_to_setting(int argc, char* argv[], struct ConnectionSettings& settings)
{
    // "Edge cases of argument processing will not be a part of evaluation 
    // (e.g., providing argument more than once, providing invalid argument value, providing unknown arguments). 
    // Application behaviour is expected to be undefined in such cases." - from asssignment

    int opt;        // Parse command-line options using getopt

    while ((opt = getopt(argc, argv, "t:s:p:d:r:h")) != -1) {
        switch (opt) {
        case 't':
            if (std::strcmp(optarg, "tcp") == 0)
                settings.protType = ProtocolType::TCP;
            else if (std::strcmp(optarg, "udp") == 0)
                settings.protType = ProtocolType::UDP;
            else
            {
                print_err("Invalid argument for -t. Use 'tcp' or 'udp'.");
                exit(EXIT_FAILURE);
            }
            break;
        case 's':
            settings.serverAdress = parse_ipAddress(optarg);
            break;

        case 'p':
            settings.serverPort = std::stoi(optarg);
            break;

        case 'd':
            settings.udpTimeout = std::stoi(optarg);
            break;

        case 'r':
            settings.udpRetry = std::stoi(optarg);
            break;

        case 'h':
            std::cerr << "Usage: " << argv[0] << " -t protocol -s servAdd [-p servPort] [-d udpTimeout] [-r udpRetry] [-h]" << std::endl;
            return 1;
            break;

        default:
            std::cerr << "Usage: " << argv[0] << " -t protocol -s servAdd [-p servPort] [-d udpTimeout] [-r udpRetry] [-h]" << std::endl;
            print_err("Invalid option!");
            exit(EXIT_FAILURE);
        }
    }

    if (settings.serverAdress.empty())
    {
        print_err("Invalid argument for -s. Provide server address.");
        exit(EXIT_FAILURE);
    }

    if (settings.protType == ProtocolType::UNDEFINED) 
    {
        print_err("Argument -t undefined! Use 'tcp' or 'udp'.");
        exit(EXIT_FAILURE);
    }

    return 0;
}

// Function to handle interrupt signal
void signalHandler(int signum) 
{
    signal_received = 1;
}

void process_user_input(const std::string& line, std::unique_ptr<AbstractConnection>& conPtr)
{
    std::istringstream iss(line);

    std::string firstWord;
    iss >> firstWord;       // will read until first whitespace

    if (firstWord == "/help")
    {
        std::cout << "Available commands: " << "\n";
        std::cout << "/auth {Username} {Secret} {DisplayName}" << "\n";
        std::cout << "/join {ChannelID}" << "\n";
        std::cout << "/rename {DisplayName}" << "\n";
        std::cout << "/help {DisplayName}" << "\n";
        std::cout << "else will be considered as a message you want to send." << "\n";
    }
    else if (firstWord == "/rename")
    {
        std::string displayName;
        iss >> displayName;

        if (!iss.eof() || displayName == "" || !match_dName(displayName) || displayName.length() > G_MAX_LEN)
        {
            print_err("Invalid rename command argument!");
            return;
        }

        conPtr->set_displayName(displayName);
    }
    else if (firstWord == "/join")
    {
        std::string channelID;
        iss >> channelID;

        if (!iss.eof() || channelID == "" || !std::regex_match(channelID, GENERAL_REGEX) || channelID.length() > G_MAX_LEN)
        {
            print_err("Invalid join command argument!");
            return;
        }

        conPtr->join_channel(channelID);
    }
    else if (firstWord == "/auth")
    {
        std::string username, secret, displayName;
        iss >> username;
        iss >> secret;
        iss >> displayName;
        
        // checking all parameter specifications
        if (!iss.eof() || username == "" || secret == "" || displayName == "" ||
            !std::regex_match(username, GENERAL_REGEX) || username.length() > G_MAX_LEN ||
            !std::regex_match(secret,   GENERAL_REGEX) || secret.length()   > S_MAX_LEN ||
            !match_dName(displayName) || displayName.length() > G_MAX_LEN)
        {
            print_err("Invalid auth command arguments!");
            return;
        }

        conPtr->set_displayName(displayName);
        conPtr->auth(username, secret);
    }
    else 
    {
        // check if it was supposted to be a command
        if (firstWord[0] == '/')
        {
            print_err("Invalid command detected!");
            return;
        }

        // else it is a message

        if (!std::regex_match(line, PRINTABLE_REGEX))
        {
            print_err("Invalid message content!");
            return;
        }

        // split line into messages of max M_MAX_LEN lenght and send them 
        for (size_t i = 0; i <= line.length(); i += M_MAX_LEN)
        {
            conPtr->msg(line.substr(i, M_MAX_LEN));
        }
    }
}

int main(int argc, char* argv[]) 
{
    // Register signal handler for interrupt signal (Ctrl+C)
    signal(SIGINT, signalHandler);

    ConnectionSettings settings;
    if (parse_args_to_setting(argc, argv, settings) != 0)
        return 0;   // help option detected, exit

    // Create connection
    std::unique_ptr<AbstractConnection> conPtr;
    try
    {
        if (settings.protType == ProtocolType::TCP)
            conPtr = std::make_unique<TCPConnection>(settings); // Create TCP connection
        else
            conPtr = std::make_unique<UDPConnection>(settings); // Create UDP connection
    }
    catch (ConnectionException&)
    {
        return 404; // TODO: return code
    }

    // Set up pollfd array
    const int nfds = 2; // Keyboard input + socket
    struct pollfd fds[nfds];

    // Add keyboard input (stdin)
    fds[0].fd = fileno(stdin);
    fds[0].events = POLLIN;

    // Add socket
    fds[1].fd = conPtr->get_socket();
    fds[1].events = POLLIN;

    // Main loop
    while (!signal_received)
    {
        // wait indefinitely until an event occurs
        if (poll(fds, nfds, -1) <= 0)  // check if poll was interrupted
            break;

        // Check if there's input from the keyboard
        if (fds[0].revents & POLLIN) 
        {
            std::string line;
            std::getline(std::cin, line);

            // check if reached EOF and read nothing
            if (std::cin.eof() && line.empty())
                return 0;   // EOF reached. Exiting

            try
            {
                process_user_input(line, conPtr);
            }
            catch (const ClientException&)
            {
                return 50;      // TODO: return code
            }

            continue;
        }

        // Check if there's data to read from the socket
        if (fds[1].revents & POLLIN)
        {
            switch (conPtr->receive_msg())
            {
                case ERR: return 50;    // TODO: return code
                case BYE: return 0;
                default: break;
            }        
        }   
    }

    // connection termination is handled by destructors
    return 0;
}