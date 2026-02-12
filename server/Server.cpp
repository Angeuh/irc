#include "../includes/Server.hpp"

Server::Server() {}

Server::~Server()
{
    close(serverSocket);
    close(epfd);
    std::cout << "Server Closed" << std::endl;
}

Server::Server(int port, std::string pass) :
	password(pass)
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
    std::cout << "New client connected: fd " << clientSocket << std::endl;
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
static int	isNicknameAvailable( std::map<int, ClientConnection> &clients, std::string &nick )
{
	//to do
	(void) clients;
	(void) nick;
	return (SUCCESS);
}

static int	verifNickname( std::string &nick )
{
	//to do
	(void) nick;
	return (SUCCESS);
}

void	Server::handleRegistration( Message &msg, int fd )
{
	switch (msg.command) {
	case NICK:
		if (msg.howManyParam == 0)
			RPL::sendRPL(this->clients[fd], RPL::errNoNickNameGiven(), this->fds);
		else if (verifNickname(msg.params[0].value) == FAILURE) {
			RPL::sendRPL(this->clients[fd], RPL::errErroneusNickname(), this->fds);
		} else if (isNicknameAvailable(this->clients, msg.params[0].value) == FAILURE) {
			RPL::sendRPL(this->clients[fd], RPL::errNickNameInUse(), this->fds);
		} else {
			this->clients[fd].username = msg.params[0].value;
			this->clients[fd].hasNick = true;
		}
		break;
	case USER:
		if (msg.howManyParam == 0)
			RPL::sendRPL(this->clients[fd], RPL::errNeedMoreParams("USER"), this->fds);
		else {
			this->clients[fd].name = msg.params.back().value;
			this->clients[fd].hasUser = true;
		}
		break;
	case PASS:
		if (msg.howManyParam == 0)
			RPL::sendRPL(this->clients[fd], RPL::errNeedMoreParams("PASS"), this->fds);
		else if (this->clients[fd].isRegistered)
			RPL::sendRPL(this->clients[fd], RPL::errAlreadyRegistred(), this->fds);
		else {
			this->clients[fd].connectionPass = msg.params[0].value;
			this->clients[fd].hasPass = true;
		}
		break;
	}
	if (this->clients[fd].hasNick && this->clients[fd].hasUser && this->clients[fd].hasPass)
	{
		std::cout << "REGISTRATION OK :" << std::endl;
		std::cout << "Client nickname : " << this->clients[fd].username << std::endl;
		std::cout << "Client username : " << this->clients[fd].name << std::endl;
		std::cout << "Client password : " << this->clients[fd].connectionPass << std::endl;
		this->clients[fd].isRegistered = true;
		RPL::sendRPL(this->clients[fd], RPL::rplWelcome(this->clients[fd].username), this->fds);
	}
}

int Server::handleClientMessage(int fd)
{
    char buffer[2048];
    int fd = fds[index].fd;
    char buffer[2048];
    int bytesReceived = recv(fd, buffer, sizeof(buffer), 0);
    if (bytesReceived <= 0)
    {
        removeFromEpoll(fd);
        close(fd);
        clients.erase(fd);
        return -1;
    }
	this->clients[fd].readBuffer += std::string(buffer, bytesReceived);
	pos = this->clients[fd].readBuffer.find("\r\n");
	while (pos != std::string::npos)
	{
		std::string	line = this->clients[fd].readBuffer.substr(0, pos + 2);
		this->clients[fd].readBuffer.erase(0, pos + 2);
		Message	msg(line);
		std::cout << msg << std::endl;
		if (clients[fd].isRegistered == true)
			handleClientMessage(msg, fd);
		else 
			handleRegistration(msg, fd);
		pos = this->clients[fd].readBuffer.find("\r\n");
	}
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
                callRecv(fds[i].fd, fd);
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

