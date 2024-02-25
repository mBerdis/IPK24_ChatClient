#include <string>

#include "AbstractConnection.h"
#include "UDPConnection.h"

int main() {
    //TODO: TCP
    //     connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    //     send(clientSocket, message, strlen(message), 0);

    UDPConnection con("127.0.0.1");
    con.send_msg("Hello World!");

    return 0;
}