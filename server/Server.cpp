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
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(sockaddr_in));
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
    int clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket < 0)
    {
        std::cerr << "Accept failed" << std::endl;
        return -1;
    }
    pollfd pfd;
    memset(&pfd, 0, sizeof(pollfd));
    pfd.fd = clientSocket;
    pfd.events = POLLIN;
    fds.push_back(pfd);
    clients.insert(std::make_pair(clientSocket, ClientConnection()));
    clients[clientSocket].fd = clientSocket;
	std::ostringstream ss;
	ss << "user" << clientSocket;
	clients[clientSocket].username = ss.str();
    std::string defaultNick = clients[clientSocket].username;
    std::string welcome = ":server 001 " + defaultNick + " :Welcome to IRC\r\n";
    clients[clientSocket].writeBuffer += welcome;
    std::cout << "New client connected: " << clientSocket << std::endl;
    return clientSocket;
}

static std::string trimCRLF(const std::string &s)
{
    size_t start = 0;
    size_t end = s.size();

    while (start < end && (s[start] == '\r' || s[start] == '\n'))
        start++;
    while (end > start && (s[end - 1] == '\r' || s[end - 1] == '\n'))
        end--;

    return s.substr(start, end - start);
}

static int joinChannel(std::map<int, ClientConnection> &clients,
    std::string &msg, int fd, std::vector<pollfd> &fds)
{
    int bytesReceived = msg.size();

    // Extract channel name
    std::string channel = trimCRLF(msg.substr(5));

    if (channel.empty() || channel == ":" || (channel[0] == ':' && channel.size() == 1)) 
    {
        return -1;
    }


    std::cout << "[JOIN] fd=" << fd 
          << " requested channel: '" << msg << "'\n";
    std::cout << "[JOIN] normalized as: '" << channel << "'\n";
    if (channel.empty() || channel[0] != '#')
        channel = "#" + channel;
    clients[fd].currentChannel = channel;
    std::string joinMsg =
        ":" + clients[fd].username + "!user@localhost JOIN :" + channel + "\r\n";
    for (std::map<int, ClientConnection>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        ClientConnection &c = it->second;
        if (c.currentChannel == channel)
        {
            c.writeBuffer += joinMsg;
            for (std::vector<pollfd>::iterator pIt = fds.begin(); pIt != fds.end(); ++pIt)
                if (pIt->fd == c.fd)
                    pIt->events |= POLLOUT;
        }
    }
    return bytesReceived;
}


static int  connectionIrssi(std::map<int, ClientConnection> &clients,
    std::string &msg, int fd, std::vector<pollfd> &fds)
{
    int bytesReceived = msg.size();
    if (msg.rfind("NICK ", 0) == 0)
    {
        std::string nick = msg.substr(5);
       while (!nick.empty() &&
       (nick[nick.size() - 1] == '\r' || nick[nick.size() - 1] == '\n'))
            nick.erase(nick.size() - 1);
        clients[fd].username = nick;
        std::string welcome = ":server 001 " + clients[fd].username + " :Welcome to IRC\r\n";
        clients[fd].writeBuffer += welcome;
		for (std::vector<pollfd>::iterator it = fds.begin(); it != fds.end(); ++it)
		{
			pollfd &p = *it;
			if (p.fd == fd)
				p.events |= POLLOUT;
		}
        return bytesReceived;
    }

    if (msg.rfind("USER ", 0) == 0)
{
    size_t size = msg.find(":");
    if (size != std::string::npos)
        clients[fd].name = msg.substr(size + 1);
    return bytesReceived;
}
    if (msg.find("CAP LS") == 0)
    {
        std::string reply = ":server CAP * LS :\r\n";
        clients[fd].writeBuffer += reply;
		for (std::vector<pollfd>::iterator it = fds.begin(); it != fds.end(); ++it)
		{
			pollfd &p = *it;
			if (p.fd == fd)
				p.events |= POLLOUT;
		}
        return bytesReceived;
    }
    return 0;
}

static void broadcastingMessage(std::map<int, ClientConnection> &clients,
                                const std::string &content,
                                int fd,
                                std::vector<pollfd> &fds)
{
    ClientConnection &sender = clients[fd];
    std::string ircMsg =
        ":" + sender.username + "!user@localhost PRIVMSG " +
        sender.currentChannel + " :" + content + "\r\n";

    std::map<int, ClientConnection>::iterator it = clients.begin();
    for (; it != clients.end(); ++it)
    {
        ClientConnection &client = it->second;

        if (client.currentChannel == sender.currentChannel &&
            client.fd != fd)
        {
            client.writeBuffer += ircMsg;
            std::vector<pollfd>::iterator pit = fds.begin();
            for (; pit != fds.end(); ++pit)
            {
                if (pit->fd == client.fd)
                {
                    pit->events |= POLLOUT;
                }
            }
        }
        std::cout << "[BROADCAST] sender=" << sender.username
          << " fd=" << fd
          << " channel='" << sender.currentChannel
          << "' msg='" << content << "'\n";
    }

    std::cout << "Broadcast OK: " << ircMsg;
}



int Server::handleClientMessage(size_t index)
{
    int fd = fds[index].fd;
    char buffer[2048];
    int bytesReceived = recv(fd, buffer, sizeof(buffer), 0);
    if (bytesReceived <= 0)
    {
        close(fd);
        clients.erase(fd);
        fds.erase(fds.begin() + index);
        return -1;
    }
    std::string msg(buffer, bytesReceived);
    msg = trimCRLF(msg);
    if (connectionIrssi(clients, msg, fd, fds) == bytesReceived)
        return bytesReceived;
    if (msg.rfind("JOIN ", 0) == 0)
        return joinChannel(clients, msg, fd, fds);
    if (msg.rfind("PRIVMSG ", 0) == 0) 
    {
        size_t colon = msg.find(" :");
        if (colon != std::string::npos)
        {
            std::string content = trimCRLF(msg.substr(colon + 2)); // text only, was part of broadcasting error
            broadcastingMessage(clients, content, fd, fds);
        }
    }
        // Log raw incoming data
    std::cout << "\n--- RAW MESSAGE RECEIVED FROM FD " 
            << fd << " ---\n";
    for (int i = 0; i < bytesReceived; i++)
    {
        unsigned char c = buffer[i];
        if (c == '\r') std::cout << "\\r";
        else if (c == '\n') std::cout << "\\n";
        else std::cout << c;
    }
    std::cout << "\n-------------------------------------\n\n";
    return bytesReceived;
}

void    Server::run()
{
    // Add listening socket to poll list
    pollfd listenPoll;
    memset(&listenPoll, 0, sizeof(pollfd));
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
                acceptNewClient();
            else if (fds[i].revents & POLLIN)
                handleClientMessage(i);
            else if (fds[i].revents & POLLOUT)
            {
                int fd = fds[i].fd;
				ClientConnection &client = clients[fd];
                if (!client.writeBuffer.empty())
                {
                    std::cout << "Sending to " << fd << ": " << client.writeBuffer;
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
