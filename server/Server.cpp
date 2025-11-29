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
    clients.emplace(clientSocket, ClientConnection{});
    clients[clientSocket].fd = clientSocket;
    clients[clientSocket].username = "user" + std::to_string(clientSocket);
    std::cout << "New client connected: " << clientSocket << std::endl;
    return clientSocket;
}

int Server::handleClientMessage(size_t index)
{
    int otherFd;
    int fd = fds[index].fd;
    char buffer[2048];
    int bytesReceived = recv(fd, buffer, sizeof(buffer), 0);
    buffer[bytesReceived] = '\0';
    if (bytesReceived <= 0)
    {
        close(fd);
        clients.erase(fd);
        fds.erase(fds.begin() + index);
        return -1;
    }
    std::string msg(buffer, bytesReceived);
    if (msg.rfind("NICK ", 0) == 0)
    {
        clients[fd].username = msg.substr(5);
        std::string welcome = ":server 001 " + clients[fd].username + " :Welcome to IRC\r\n";
        clients[fd].writeBuffer += welcome;
        for (pollfd &p : fds)
            if (p.fd == fd)
                p.events |= POLLOUT;
        return bytesReceived;
    }

    if (msg.rfind("USER ", 0) == 0) {
        clients[fd].username = msg.substr(msg.find(":") + 1);
        return bytesReceived;
    }
    if (msg.find("CAP LS") == 0)
    {
        std::string reply = ":server CAP * LS :\r\n";
        clients[fd].writeBuffer += reply;
        for (pollfd &p : fds)
            if (p.fd == fd)
                p.events |= POLLOUT;

        return bytesReceived;
    }
    ClientConnection &sender = clients[fd];
    std::string ircMsg =
        ":" + sender.username + "!user@localhost PRIVMSG #channel :" +
            msg + "\r\n";
    for (std::pair<const int, ClientConnection> &pair : clients)
    {
        std::cout << "test :" << ircMsg;
        otherFd = pair.first;
        if (ircMsg.back() != '\n')
            ircMsg += "\r\n";
        clients[otherFd].writeBuffer += ircMsg;
        for (pollfd &p : fds)
        {
            if (p.fd == otherFd)
                p.events |= POLLOUT;
        }
    }
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
            else if (fds[i].revents & POLLOUT)
            {
                int fd = fds[i].fd;
                auto &client = clients[fd];
                if (!client.writeBuffer.empty())
                {
                    int sent = send(fd, client.writeBuffer.c_str(), client.writeBuffer.size(), 0);

                    if (sent > 0)
                        client.writeBuffer.erase(0, sent);
                    if (client.writeBuffer.empty()) {
                        fds[i].events &= ~POLLOUT;
                    }
                }
            }
        }
    }
}