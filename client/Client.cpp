#include "../includes/Client.hpp"

Client ::Client()
{
    std::cout << "Default Client constructor " << std::endl;
}

Client::~Client()
{
    close(sock);
    std::cout << "Client Disconnected" << std::endl;
}

Client::Client(const std::string &host, int port)
{
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        std::cerr << "Socket creation failed" << std::endl;
        return;
    }
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cerr << "Connection to server failed" << std::endl;
        return;
    }
    std::cout << "Connected to server " << host << " on port " << port << std::endl;

}

int    Client::sendMessage(const std::string &msd)
{
    if (send(sock, msd.c_str(), msd.size(), 0) < 0)
    {
        std::cerr << "Send failed" << std::endl;
        return -1;
    }
    return 0;
}

void    Client::run()
{
    std::string line;
    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, line);

        if (line == "exit" || std::cin.eof())
            break;

        if (sendMessage(line) < 0)
            break;
    }
}