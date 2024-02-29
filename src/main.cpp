#include <string>
#include <unistd.h>
#include <cstring>
#include <iostream>

#include "ConnectionSettings.h"
#include "AbstractConnection.h"
#include "UDPConnection.h"
#include "TCPConnection.h"

static int parse_args_to_setting(int argc, char* argv[], struct ConnectionSettings& settings)
{
    int opt;        // Parse command-line options using getopt

    while ((opt = getopt(argc, argv, "t:s:p:d:r:h")) != -1) {
        switch (opt) {
        case 't':
            if (std::strcmp(optarg, "tcp") == 0)
                settings.protType = ProtocolType::TCP;
            else if (std::strcmp(optarg, "udp") == 0)
                settings.protType = ProtocolType::UDP;
            else
                throw std::invalid_argument("Invalid argument for -t. Use 'tcp' or 'udp'.");
            break;
        case 's':
            settings.serverAdress = optarg;
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
            throw std::invalid_argument("Invalid option!");
        }
    }

    if (settings.serverAdress.empty())
        throw std::invalid_argument("Invalid argument for -s. Provide server address.");

    if (settings.protType == ProtocolType::UNDEFINED)
        throw std::invalid_argument("Argument -t undefined! Use 'tcp' or 'udp'.");

    return 0;
}

int main(int argc, char* argv[]) 
{
    ConnectionSettings settings;
    if (parse_args_to_setting(argc, argv, settings) != 0)
        return 0;   // add option detected, exit

    std::cout << settings.serverAdress << ":" << settings.serverPort << "\n";
    std::cout << (int) settings.udpRetry << " " << settings.udpTimeout << "\n";

    if (settings.protType == ProtocolType::TCP)
    {
        TCPConnection con(settings);
        con.send_msg("Hello World!");
    }

    return 0;
}