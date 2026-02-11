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
        throw std::runtime_error("Socket creation failed");
    }
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(sockaddr_in));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        throw std::runtime_error("Bind failed");
    }
    if (listen(serverSocket, 5) < 0)
    {
        throw std::runtime_error("Listen failed");
    }
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
    pollfd pfd;
    memset(&pfd, 0, sizeof(pollfd));
    pfd.fd = clientSocket;
    pfd.events = POLLIN;
    fds.push_back(pfd);
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

static int joinChannel(std::map<int, ClientConnection> &clients,
    std::string &msg, int fd, std::vector<pollfd> &fds,
	std::map<std::string, Channel> &channels)
{
    int bytesReceived = msg.size();

    // Extract channel name
    std::string channel = trimCRLF(msg.substr(5));
    std::cout << "Requested to join channel: '" << channel << "'\n";
    if (channel.empty() || channel == ":" || (channel[0] == ':' && channel.size() == 1)) 
        return -1;
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
	if (channels.find(channel) == channels.end())
		channels[channel] = Channel(channel, fd);
	else
		channels[channel].insertUser(fd);
    return bytesReceived;
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

static int  connectionIrssi(std::map<int, ClientConnection> &clients,
    Message &msg, int fd, std::vector<pollfd> &fds)
{
    if (msg.command.value == "NICK")
    {
		if (msg.howManyParam == 0)
			RPL::sendRPL(clients[fd], RPL::errNoNickNameGiven(), fds);
		else if (verifNickname(msg.params[0].value) == FAILURE) {
			RPL::sendRPL(clients[fd], RPL::errErroneusNickname(), fds);
		} else if (isNicknameAvailable(clients, msg.params[0].value) == FAILURE) {
			RPL::sendRPL(clients[fd], RPL::errNickNameInUse(), fds);
		} else
			clients[fd].username = msg.params[0].value;
    } else if (msg.command.value == "USER") {
		if (msg.howManyParam == 0)
			RPL::sendRPL(clients[fd], RPL::errNeedMoreParams("USER"), fds);
		else {
			clients[fd].name = msg.params.back().value;
			RPL::sendRPL(clients[fd], RPL::rplWelcome(clients[fd].username), fds);
		}
	} else if (msg.rawMessage.find("CAP LS") == 0) {
		RPL::sendRPL(clients[fd], ":localhost CAP * LS:\r\n", fds);
    } else
		return (FAILURE);
	return (SUCCESS);
}

static int  operatorCommand(std::map<int, ClientConnection> &clients,
    Message &msg, int fd, std::map<std::string, Channel> &channels,
	std::vector<pollfd> &fds)
{
	Channel		channel = channels[clients[fd].currentChannel];

	if (msg.command.value == "KICK") {
		channel.kickCmd(msg);
	} else if (msg.command.value == "INVITE") {
		channel.inviteCmd(msg, clients, fd, fds);
	} else if (msg.command.value == "TOPIC") {
		channel.topicCmd(msg, clients, fd, fds);
	} else if (msg.command.value == "MODE") {
		channel.modeCmd(msg);
	} else
		return (FAILURE);
	return (SUCCESS);
}

void broadcastingMessage(std::map<int, ClientConnection> &clients,
                                const std::string &content,
								const std::string &command,
                                int fd,
                                std::vector<pollfd> &fds
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
            std::vector<pollfd>::iterator pit = fds.begin();
            for (; pit != fds.end(); ++pit)
            {
                if (pit->fd == client.fd)
                    pit->events |= POLLOUT;
            }
        }
        std::cout << "[BROADCAST] sender=" << sender.username
          << " fd=" << fd
          << " channel='" << sender.currentChannel
          << "' msg='" << content << "'\n";
    }

    std::cout << "Broadcast OK: " << ircMsg;
}

void	Server::handleClientMessage( Message &msg, int fd )
{
	if (connectionIrssi(this->clients, msg, fd, this->fds) == SUCCESS)
        return ;
    if (msg.command.value == "JOIN") {
        joinChannel(this->clients, msg.rawMessage, fd, this->fds, this->channels);
	}
	if (operatorCommand(this->clients, msg, fd, this->channels, this->fds) == SUCCESS)
		return ;
    if (msg.command.value == "PRIVMSG") {
		broadcastingMessage(this->clients, msg.params[0].value, "PRIVMSG", fd, fds);
	}
}

void Server::callRecv(int fd, int index)
{
	char	buffer[4096];
    int 	bytesReceived = recv(fd, buffer, sizeof(buffer), 0);
    if (bytesReceived <= 0)
    {
        close(fd);
        this->clients.erase(fd);
        this->fds.erase(this->fds.begin() + index);
        return ;
    }
	this->clients[fd].readBuffer += std::string(buffer, bytesReceived);
	size_t	pos;

	pos = this->clients[fd].readBuffer.find("\r\n");
	while (pos != std::string::npos)
	{
		std::string	line = this->clients[fd].readBuffer.substr(0, pos + 2);
		this->clients[fd].readBuffer.erase(0, pos + 2);
		Message	msg(line);
		std::cout << msg << std::endl;
		handleClientMessage(msg, fd);
		pos = this->clients[fd].readBuffer.find("\r\n");
	}
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
            throw PollError();
        }

        for (size_t i = 0; i < fds.size(); i++)
        {
            if (fds[i].fd == serverSocket && (fds[i].revents & POLLIN))
                acceptNewClient();
            else if (fds[i].revents & POLLIN)
                callRecv(fds[i].fd, i);
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
