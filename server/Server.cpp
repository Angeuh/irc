#include "../includes/Server.hpp"

bool Server::Signal = false; 

Server::Server()
{
    Signal = false;
}

Server::~Server()
{
    close(serverSocket);
    close(epfd);
    
    std::cout << "Server Closed" << std::endl;
}

Server::Server(int port, std::string pass) :
	password(pass)
{
    Signal = false;
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
    if (listen(serverSocket, 128) < 0)
    {
        throw std::runtime_error("Listen failed");
    }
    addToEpoll(serverSocket, EPOLLIN);
    std::cout << "Server listening on port " << port << std::endl;
}
void Server::SignalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "Signal Received!" << std::endl;
	Signal = true;
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

void	Server::joinCmd( Message &msg, ClientConnection &user )
{
	(void) msg;
	(void) user;
    // std::string channel = trimCRLF(msg.substr(5));
    // std::cout << "Requested to join channel: '" << channel << "'\n";
    // if (channel.empty() || channel == ":" || (channel[0] == ':' && channel.size() == 1)) 
    //     return -1;
    // /* if(channel.get_isInviteOnly() == true)
    // {
    //     // ERR_INVITEONLYCHAN err : 473
    //     //           "<channel> :Cannot join channel (+i)"
    //     std::cerr << "<channel> :Cannot join channel " << channel[0] << std::endl;
    //     return -1;
    // } */
    // std::cout << "[JOIN] fd=" << fd
    //       << " requested channel: '" << msg << "'\n";
    // std::cout << "[JOIN] normalized as: '" << channel << "'\n";
    // if (channel.empty() || channel[0] != '#')
    //     channel = "#" + channel;
    // clients[fd].currentChannel = channel;
    // std::string joinMsg =
    //     ":" + clients[fd].username + "!user@localhost JOIN :" + channel + "\r\n";
    // for (std::map<int, ClientConnection>::iterator it = clients.begin(); it != clients.end(); ++it)
    // {
    //     ClientConnection &c = it->second;
    //     if (c.currentChannel == channel)
    //     {
    //         c.writeBuffer += joinMsg;
    //         this->modifyEpoll(c.fd, EPOLLIN | EPOLLOUT);
    //     }
    // }
	// if (channels.find(channel) == channels.end())
	// 	channels[channel] = Channel(channel, fd);
	// else
	// 	channels[channel].insertUser(fd);
}

// format : KICK <channel, ...> <nick, ...> [<reason>]
// either multiple channels or multiple users
// reason broadcasted to all users
void	Server::kickCmd( Message &msg, ClientConnection &user )
{
	(void) msg;
	(void) user;
}

// format : INVITE <nickname> <channel>
void	Server::inviteCmd( Message &msg, ClientConnection &user )
{
	(void) msg;
	(void) user;
}

// format : TOPIC [param]
void	Server::topicCmd( Message &msg, ClientConnection &user )
{
	Channel	channel; //to do

	if (msg.howManyParam == 0) {
		sendRPL(user, RPL::rplTopic(user.username, channel.getName(), msg.params[1].value));
	// } else if (channel.isOperator(user) == false) {
		// sendRPL(user, RPL::errChanOpPrivsNeeded(user.username, channel.getName()));
	} else if (msg.params[1].value.empty()) {
		channel.getTopic() = "";
		sendRPL(user, RPL::rplTopic(user.username, channel.getName(), msg.params[1].value));
	} else {
		channel.getTopic() = msg.params[1].value;
		// broadcastingMessage(clients, msg.params[0].value, "TOPIC", fd);
	}
}

void	Server::modeCmd( Message &msg, ClientConnection &user )
{
	(void) msg;
	(void) user;
}

void Server::broadcastingMessage(std::map<int, ClientConnection> &clients,
                                const std::string &content,
								const std::string &command,
                                int fd)
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
            if (ircMsg.size() > 512) 
            {
                ircMsg = ircMsg.substr(0, 512);
            }
            this->modifyEpoll(client.fd, EPOLLIN | EPOLLOUT);
        }
        std::cout << "[BROADCAST] sender=" << sender.username
          << " fd=" << fd
          << " channel='" << sender.currentChannel
          << "' msg='" << content << "'\n";
    }

    std::cout << "Broadcast OK: " << ircMsg;
}

// - Pour les RPL: ":"serverName + " " + RPLnum + " " + RPLmsg + "\r\n";
// - Pour les ERR: ":"serverName + " " + ERRnum + " " + command + " " + ERRmsg + "\r\n";
// - Pour les reponses informatives: ":"nickname"!~"+username"@"serverName + " " + command + " :" + variable + "\r\n";
void Server::sendRPL(ClientConnection &client, const std::string &content)
{
    client.writeBuffer += content;
    this->modifyEpoll(client.fd, EPOLLIN | EPOLLOUT);
    std::cout << "[RPL/ERR] sender=" << client.username
        << " channel='" << client.currentChannel
        << "' msg='" << content << "'\n";
    std::cout << "RPL/ERR OK: " << content << std::endl;
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

void	Server::handleRegistration( Message &msg, ClientConnection &user )
{
	switch (msg.command) {
	case NICK:
		if (msg.howManyParam == 0)
			sendRPL(user, RPL::errNoNickNameGiven());
		else if (verifNickname(msg.params[0].value) == FAILURE) {
			sendRPL(user, RPL::errErroneusNickname());
		} else if (isNicknameAvailable(this->clients, msg.params[0].value) == FAILURE) {
			sendRPL(user, RPL::errNickNameInUse());
		} else {
			user.username = msg.params[0].value;
			user.hasNick = true;
		}
		break;
	case USER:
		if (msg.howManyParam == 0)
			sendRPL(user, RPL::errNeedMoreParams("USER"));
		else {
			user.name = msg.params.back().value;
			user.hasUser = true;
		}
		break;
	case PASS:
		if (msg.howManyParam == 0)
			sendRPL(user, RPL::errNeedMoreParams("PASS"));
		else if (user.isRegistered)
			sendRPL(user, RPL::errAlreadyRegistred());
		else {
			user.connectionPass = msg.params[0].value;
			user.hasPass = true;
		}
		break;
	}
	if (user.hasNick && user.hasUser && user.hasPass)
	{
		std::cout << "REGISTRATION OK :" << std::endl;
		std::cout << "Client nickname : " << user.username << std::endl;
		std::cout << "Client username : " << user.name << std::endl;
		std::cout << "Client password : " << user.connectionPass << std::endl;
		user.isRegistered = true;
		sendRPL(user, RPL::rplWelcome(user.username));
	}
}

void	Server::handleClientMessage( Message &msg, ClientConnection &user )
{
	switch (msg.command) {
	case JOIN:
		joinCmd(msg, user);
	case TOPIC:
		topicCmd(msg, user);
	case MODE:
		modeCmd(msg, user);
	case INVITE:
		inviteCmd(msg, user);
	case KICK:
		kickCmd(msg, user);
	// case PRIVMSG:
		// broadcastingMessage(this->clients, msg.params[0].value, "PRIVMSG", fd);
	}
}

void Server::callRecv(int fd)
{
	ClientConnection	user = this->clients[fd];
	char				buffer[4096];
    int 				bytesReceived = recv(fd, buffer, sizeof(buffer), 0);
	size_t				pos;

    if (bytesReceived <= 0)
    {
        removeFromEpoll(fd);
        close(fd);
        clients.erase(fd);
        return ;
    }
	user.readBuffer += std::string(buffer, bytesReceived);
	pos = user.readBuffer.find("\r\n");
	while (pos != std::string::npos)
	{
		std::string	line = user.readBuffer.substr(0, pos + 2);
		user.readBuffer.erase(0, pos + 2);
		Message	msg(line);
		std::cout << msg << std::endl;
		if (user.isRegistered == true)
			handleClientMessage(msg, user);
		else 
			handleRegistration(msg, user);
		pos = user.readBuffer.find("\r\n");
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
        {
            if (errno == EINTR)
                return;
            throw PollError();
        }

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
                int fd_epoll = events[i].data.fd;
                callRecv(fd_epoll);
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

