#include "../includes/Server.hpp"

Server::Server()
{
    std::cout << "Default Server constructor " << std::endl;
}

Server::~Server()
{
    close(serverSocket);
    std::cout << "Server Closed" << std::endl;
}

Server::Server(int port)
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        std::cerr << "Socket creation failed" << std::endl;
        return;
    }
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cerr << "Bind failed" << std::endl;
        return;
    }
    if (listen(serverSocket, 5) < 0)
    {
        std::cerr << "Listen failed" << std::endl;
        return;
    }
    std::cout << "Server listening on port " << port << std::endl;
}

int Server::acceptNewClient()
{
    int clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket < 0)
    {
        std::cerr << "Accept failed" << std::endl;
        return -1;
    }
    pollfd pfd{};
    pfd.fd = clientSocket;
    pfd.events = POLLIN;
    fds.push_back(pfd);
    std::cout << "New client connected: " << clientSocket << std::endl;
    return clientSocket;
}

int Server::handleClientMessage(size_t index)
{
    char buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    int bytesReceived = recv(fds[index].fd, buffer, sizeof(buffer), 0);
    if (bytesReceived <= 0)
    {
        std::cout << "Client " << fds[index].fd << " disconnected." <<
        std::endl;
        close(fds[index].fd);
        fds.erase(fds.begin() + index);
        return -1;
    }
    std::cout << "Client " << fds[index].fd << ": " << buffer << std::endl;
    return bytesReceived;
}

void    Server::run()
{
    // Add listening socket to poll list
    pollfd listenPoll{};
    listenPoll.fd = serverSocket;
    listenPoll.events = POLLIN;
    fds.push_back(listenPoll);

    while (true)
    {
        int ret = poll(fds.data(), fds.size(), -1);
        if (ret < 0)
        {
            std::cerr << "Poll failed" << std::endl;
            continue;
        }

        for (size_t i = 0; i < fds.size(); i++)
        {
            if (fds[i].fd == serverSocket && (fds[i].revents & POLLIN))
            {
                acceptNewClient();
            }
            else if (fds[i].revents & POLLIN)
            {
                handleClientMessage(i);
            }
        }
    }
}