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
    {
        close(serverSocket); 
        throw std::runtime_error("epoll_create failed");
    }
    if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        close(epfd);
        close(serverSocket);
        throw std::runtime_error("Bind failed");
    }
    if (listen(serverSocket, 128) < 0)
    {
        close(epfd);
        close(serverSocket);
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
            this->modifyEpoll(c.fd, EPOLLIN | EPOLLOUT);
        }
    }
	if (channels.find(channel) == channels.end())
		channels[channel] = Channel(channel, fd);
	else
		channels[channel].insertUser(fd);
    return bytesReceived;
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
            /* if (ircMsg.size() > 512) 
            {
                ircMsg = ircMsg.substr(0, 512);
            } */
            this->modifyEpoll(client.fd, EPOLLIN | EPOLLOUT);
        }
        std::cout << "[BROADCAST] sender=" << sender.username
          << " fd=" << fd
          << " channel='" << sender.currentChannel
          << "' msg='" << content << "'\n";
    }

    std::cout << "Broadcast OK: " << ircMsg;
}

//    Numeric Replies:

//            ERR_NONICKNAMEGIVEN             ERR_ERRONEUSNICKNAME
//            ERR_NICKNAMEINUSE               ERR_NICKCOLLISION
//            ERR_UNAVAILRESOURCE             ERR_RESTRICTED

    //    431    ERR_NONICKNAMEGIVEN
    //           ":No nickname given"

    //    432    ERR_ERRONEUSNICKNAME
    //           "<nick> :Erroneous nickname"

    //      - Returned after receiving a NICK message which contains
    //        characters which do not fall in the defined set.  See
    //        section 2.3.1 for details on valid nicknames.

    //    433    ERR_NICKNAMEINUSE
    //           "<nick> :Nickname is already in use"

    //      - Returned when a NICK message is processed that results
    //        in an attempt to change to a  currently existing nickname
    // 436    ERR_NICKCOLLISION
    //           "<nick> :Nickname collision KILL from <user>@<host>"

    //      - Returned by a server to a client when it detects a
    //        nickname collision (registered of a NICK that
    //        already exists by another server).

    //    437    ERR_UNAVAILRESOURCE
    //           "<nick/channel> :Nick/channel is temporarily unavailable"

    //      - Returned by a server to a user trying to join a channel
    //        currently blocked by the channel delay mechanism.

    //      - Returned by a server to a user trying to change nickname
    //        when the desired nickname is blocked by the nick delay
    //        mechanism.

    //    441    ERR_USERNOTINCHANNEL
    //           "<nick> <channel> :They aren't on that channel"

    //      - Returned by the server to indicate that the target
    //        user of the command is not on the given channel.
static int	isNicknameAvailable( std::map<int, ClientConnection> &clients, std::string &nick )
{

    for (std::map<int, ClientConnection>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->second.username == nick && it->second.isRegistered == true)
            return (FAILURE);
    }
	return (SUCCESS);
}

static bool isSpecial(char c) 
{
    std::string special = "[]\\`_^{}|";
    return special.find(c) != std::string::npos;
}

static int	verifNickname( std::string &nick )
{
    std::string special = "[]\\`_^{}|";
	if (nick.size() > 9 || nick.size() < 1)
        return (FAILURE);

    char first = nick[0];
    if (!std::isalpha(first) && !isSpecial(first))
        return (FAILURE);

    for (size_t i = 1; i < nick.length(); i++) 
    {
        char c = nick[i];

        if (!std::isalnum(c) &&!isSpecial(c) && c != '-') 
            return (FAILURE);
    }
	return (SUCCESS);
}

static std::string itoa(int n)
{
    std::ostringstream oss;
    oss << n;
    return oss.str();
}

std::string Server::generateFreeNick(const std::string &base)
{
    std::string nick = base;

    if (isNicknameAvailable(this->clients, nick) == SUCCESS)
        return nick;

    nick = base + "_";
    if (isNicknameAvailable(this->clients, nick) == SUCCESS)
        return nick;
    for (int i = 1; i < 1000; i++)
    {
        nick = base + itoa(i);
        if (isNicknameAvailable(this->clients, nick) == SUCCESS)
            return nick;
    }

    return "";
}

void	Server::handleRegistration( Message &msg, int fd )
{
	switch (msg.command) {
	case NICK:
    {
        std::string wanted = msg.params[0].value;

        if (msg.howManyParam == 0)
        {
            RPL::sendRPL(clients[fd],
                        RPL::errNoNickNameGiven(),
                        *this);
        }
        else if (verifNickname(wanted) == FAILURE)
        {
            RPL::sendRPL(clients[fd],
                        RPL::errErroneusNickname(),
                        *this);
        }
        else if (isNicknameAvailable(clients, wanted) == FAILURE)
        {
            wanted = generateFreeNick(wanted);
            if (wanted.empty())
            {
                RPL::sendRPL(clients[fd],
                            RPL::errNickNameInUse(),
                            *this);
                break;
            }
            RPL::sendRPL(clients[fd],
                        RPL::errNickNameInUse(),
                        *this);
        }
        clients[fd].username = wanted;
        clients[fd].hasNick = true;
        break;
    }
	case USER:
		if (msg.howManyParam == 0)
			RPL::sendRPL(this->clients[fd], RPL::errNeedMoreParams("USER"), *this);
		else {
			this->clients[fd].name = msg.params.back().value;
			this->clients[fd].hasUser = true;
		}
		break;
	case PASS:
		if (msg.howManyParam == 0)
			RPL::sendRPL(this->clients[fd], RPL::errNeedMoreParams("PASS"), *this);
		else if (this->clients[fd].isRegistered)
			RPL::sendRPL(this->clients[fd], RPL::errAlreadyRegistred(), *this);
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
		RPL::sendRPL(this->clients[fd], RPL::rplWelcome(this->clients[fd].username), *this);
	}
}

void	Server::handleClientMessage(Message &msg, int fd)
{
	Channel		channel = this->channels[clients[fd].currentChannel];

	switch (msg.command) {
	case JOIN:
		joinChannel(this->clients, msg.rawMessage, fd, this->channels);
        break;
	case TOPIC:
		channel.topicCmd(msg, this->clients, fd, *this);
        break;
	// case MODE:
	// 	channel.modeCmd(msg);
	// case INVITE:
	// 	channel.inviteCmd(msg, this->clients, fd, this->fds);
	// case KICK:
	// 	channel.kickCmd(msg);
	case PRIVMSG:
		broadcastingMessage(this->clients, msg.params[0].value, "PRIVMSG", fd);
        break;
	}
}

void Server::callRecv(int fd)
{
	char	buffer[4096];
    int 	bytesReceived = recv(fd, buffer, sizeof(buffer), 0);
	size_t	pos;

    if (bytesReceived <= 0)
    {
        removeFromEpoll(fd);
        close(fd);
        clients.erase(fd);
        return ;
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

void    Server::closeFd()
{

    for (std::map<int, ClientConnection>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        close(it->first);
    }
    clients.clear();
    close(serverSocket);
    close(epfd);

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
                break;
            closeFd();
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
                callRecv(fd);
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
    std::cout << "Shutting down server..." << std::endl;
    closeFd();
    std::cout << "Server Closed" << std::endl;
}

