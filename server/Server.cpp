#include "../includes/Server.hpp"

Server::Server()
{
    std::cout << "Default Server constructor " << std::endl;
}

Server::~Server()
{
    close(serverSocket);
    close(epfd);
    std::cout << "Server Closed" << std::endl;
}

Server::Server(int port)
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        throw std::runtime_error("Socket creation failed");
    }
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(sockaddr_in));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    epfd = epoll_create(1024);
    if (epfd < 0)
        throw std::runtime_error("epoll_create failed");
    if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        throw std::runtime_error("Bind failed");
    }
    if (listen(serverSocket, 5) < 0)
    {
        throw std::runtime_error("Listen failed");
    }
    addToEpoll(serverSocket, EPOLLIN);
    std::cout << "Server listening on port " << port << std::endl;
}

char const *Server::PollError::what() const throw()
{
    return "Poll failed";
}

int Server::acceptNewClient()
{
    int clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket < 0)
    {
        std::cerr << "Accept failed" << std::endl;
        return -1;
    }
    addToEpoll(clientSocket, EPOLLIN);
    clients.insert(std::make_pair(clientSocket, ClientConnection()));
    clients[clientSocket].fd = clientSocket;
	std::ostringstream ss;
	ss << "user" << clientSocket;
	clients[clientSocket].username = ss.str();
    std::string defaultNick = clients[clientSocket].username;
    //clients[clientSocket].fd = inet_ntoa(clients.sin_addr);
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

    return (s.substr(start, end - start));
}

int Server::joinChannel(std::map<int, ClientConnection> &clients,
    std::string &msg, int fd,
	std::map<std::string, Channel> &channels)
{
    int bytesReceived = msg.size();


    // Extract channel name
    std::string channel = trimCRLF(msg.substr(5));
    std::cout << "Requested to join channel: '" << channel << "'\n";
    if (channel.empty() || channel == ":" || (channel[0] == ':' && channel.size() == 1)) 
        return -1;
    /* if(channel.get_isInviteOnly() == true)
    {
        // ERR_INVITEONLYCHAN err : 473
        //           "<channel> :Cannot join channel (+i)"
        std::cerr << "<channel> :Cannot join channel " << channel[0] << std::endl;
        return -1;
    } */
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
            this->modifyEpoll(fd, EPOLLIN | EPOLLOUT);
        }
    }
	if (channels.find(channel) == channels.end())
		channels[channel] = Channel(channel, fd);
	else
		channels[channel].insertUser(fd);
    return bytesReceived;
}


int  Server::connectionIrssi(std::map<int, ClientConnection> &clients,
    std::string &msg, int fd)
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
		this->modifyEpoll(fd, EPOLLIN | EPOLLOUT);
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
		this->modifyEpoll(fd, EPOLLIN | EPOLLOUT);
        return bytesReceived;
    }
    return 0;
}

static int  operatorCommand(std::map<int, ClientConnection> &clients,
    Message &msg, int fd, std::map<std::string, Channel> &channels)
{
	Channel	&channel = channels[clients[fd].currentChannel];

	if (msg.command.value == "KICK")
	{
		channel.kickCmd(msg);
		return (SUCCESS);
	}
	if (msg.command.value == "INVITE")
	{
		channel.inviteCmd(msg, clients, fd);
		return (SUCCESS);
	}
	if (msg.command.value == "TOPIC")
	{
		channel.topicCmd(msg, clients, fd);
		return (SUCCESS);
	}
	if (msg.command.value == "MODE")
	{
		channel.modeCmd(msg);
		return (SUCCESS);
	}
	return (FAILURE);
}

void Server::broadcastingMessage(std::map<int, ClientConnection> &clients,
                                const std::string &content,
								const std::string &command,
                                int fd
							)
{
	bool skipSender = (command == "PRIVMSG");
    ClientConnection &sender = clients[fd];
    std::string ircMsg =
        ":" + sender.username + "!user@localhost " + command + " " +
        sender.currentChannel + " :" + content + "\r\n";

    std::map<int, ClientConnection>::iterator it = clients.begin();
    for (; it != clients.end(); ++it)
    {
        ClientConnection &client = it->second;

        if (client.currentChannel == sender.currentChannel &&
            (!skipSender || client.fd != fd))
        {
            client.writeBuffer += ircMsg;
            this->modifyEpoll(client.fd, EPOLLIN | EPOLLOUT);
        }
        std::cout << "[BROADCAST] sender=" << sender.username
          << " fd=" << fd
          << " channel='" << sender.currentChannel
          << "' msg='" << content << "'\n";
    }

    std::cout << "Broadcast OK: " << ircMsg;
}

int Server::handleClientMessage(int fd)
{
    char buffer[2048];
    int bytesReceived = recv(fd, buffer, sizeof(buffer), 0);
    if (bytesReceived <= 0)
    {
        removeFromEpoll(fd);
        close(fd);
        clients.erase(fd);
        return -1;
    }
    std::string msg(buffer, bytesReceived);
	Message		parsedMsg(buffer, bytesReceived);
    msg = trimCRLF(msg);
    std::cout << "Raw message without \\r somehow ? : " << msg << std::endl;
    if (connectionIrssi(clients, msg, fd) == bytesReceived)
        return bytesReceived;
    if (msg.rfind("JOIN ", 0) == 0) {
        return joinChannel(clients, msg, fd, channels);
	}
	if (operatorCommand(clients, parsedMsg, fd, channels) == SUCCESS)
		return bytesReceived;
    if (msg.rfind("PRIVMSG ", 0) == 0)
    {
        size_t colon = msg.find(" :");
        if (colon != std::string::npos)
        {
            std::string content = trimCRLF(msg.substr(colon + 2));
            if (content.length() > 510)
                content = content.substr(0, 512);
            broadcastingMessage(clients, content, "PRIVMSG", fd);
        }
    }
    // Log raw incoming data, for test only, but i'll keep it for push, always useful
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

void Server::run()
{
    const int MAX_EVENTS = 64;
    struct epoll_event events[MAX_EVENTS];

    while (true)
    {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);

        if (n < 0)
            throw PollError();

        for (int i = 0; i < n; i++)
        {
            int fd = events[i].data.fd;
            if (fd == serverSocket)
            {
                acceptNewClient();
                continue;
            }
            if (events[i].events & EPOLLIN)
            {
                handleClientMessage(fd);
            }
            if (events[i].events & EPOLLOUT)
            {
                ClientConnection &client = clients[fd];

                if (!client.writeBuffer.empty())
                {
                    int sent = send(
                        fd,
                        client.writeBuffer.c_str(),
                        client.writeBuffer.size(),
                        0);

                    if (sent > 0)
                        client.writeBuffer.erase(0, sent);

                    if (client.writeBuffer.empty())
                        this->modifyEpoll(fd, EPOLLIN);
                }
            }
        }
    }
}

