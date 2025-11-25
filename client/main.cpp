// C++ program to illustrate the client application in the
// socket programming

#include "../includes/Client.hpp"
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main()
{
    // creating socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    // specifying address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // sending connection request
    connect(clientSocket, (struct sockaddr*)&serverAddress,
            sizeof(serverAddress));

    // sending data
    std::string line;
    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, line);

        if (line == "exit" || std::cin.eof())
            break;

        send(clientSocket, line.c_str(), line.size(), 0);
    }

    close(clientSocket);
    return 0;
}