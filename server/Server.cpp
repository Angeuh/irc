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

//send content to all users in channel (except for user if PRIVMSG)
void Server::broadcastingMessage( ClientConnection &user, const std::string &command, const std::string &content )
{
	bool	skipUser = (command == "PRIVMSG");
	for (std::map<int, ClientConnection>::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
    {
		ClientConnection &client = it->second;
		if (client.currentChannel == user.currentChannel && (!skipUser || client.username != user.username)) {
			std::cout << "[BROADCAST] send to " << client.username << std::endl;
			sendMessage(client, content);
		}
    }
}

// - Pour les RPL: ":"serverName + " " + RPLnum + " " + RPLmsg + "\r\n";
// - Pour les ERR: ":"serverName + " " + ERRnum + " " + command + " " + ERRmsg + "\r\n";
// - Pour les reponses informatives: ":"nickname"!~"+username"@"serverName + " " + command + " :" + variable + "\r\n";

//send content to client
void Server::sendMessage( ClientConnection &client, const std::string &content )
{
    client.writeBuffer += content;
    this->modifyEpoll(client.fd, EPOLLIN | EPOLLOUT);
    std::cout << "[SENDMESSAGE] " << content << std::endl;
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

void	Server::welcomeToChannel( Channel &channel, ClientConnection &user )
{
	broadcastingMessage(user, "JOIN", RPL::ircMessageNoContent(user.username, "JOIN", channel.getName()));
	if (channel.hasTopic)
		sendMessage(user, RPL::rplTopic(user.username, channel.getName(), channel.getTopic()));
	else
		sendMessage(user, RPL::rplNoTopic(user.username, channel.getName()));
	sendMessage(user, RPL::rplNameReply(user.username, channel));
	sendMessage(user, RPL::rplEndOfNames(user.username, channel.getName()));
}

//à faire : séparer en deux, channel existant et non existant (je fais demain)
void	Server::joinOneChannel( ClientConnection &user, std::string &channelName, std::string &key, bool hasKey )
{
    if (channelName.empty()) {
		// sendMessage(user, RPL::errBadChanMask()); // À vérifier
		return ;
	}
	if (channelName[0] != '#')
		channelName = "#" + channelName;

	std::map<std::string, Channel>::iterator	it = this->channels.find(channelName);
	if (it == this->channels.end()) {
		this->channels[channelName] = Channel(channelName, user);
		welcomeToChannel(this->channels[channelName], user);
		user.activeChannels[channelName] = &this->channels[channelName];
		return ;
	}
	else {
		Channel	&channel = it->second;
		if (channel.isInviteOnly()) {
			sendMessage(user, RPL::errInviteOnlyChan(user.username, channel.getName()));
			return ;
		} else if (channel.isFull()) {
			sendMessage(user, RPL::errChannelIsFull(user.username, channel.getName()));
			return ;
		} else if (channel.hasKey && ((hasKey && key != channel.getKey()) || (!hasKey))) {
			sendMessage(user, RPL::errBadChannelKey(user.username, channel.getName()));
			return ;
		} else {
			channel.insertUser(user);
			welcomeToChannel(channel, user);
			user.activeChannels[channelName] = &channel;
		}
	}		
}

// void	Server::quitOneChannel( std::set<std::string> &userChannels, ClientConnection &user )
// {

// }

void	Server::quitAllChannels( ClientConnection &user )
{
	for (std::map<std::string, Channel *>::iterator it = user.activeChannels.begin(); it != user.activeChannels.end(); it++)
		it->second->removeUser(user);
	user.activeChannels.clear();
}

// join <channel,channel,channel...> <key,key,key...>
// channel[i] -> key[i] each join generates its own success or error
void	Server::joinCmd( Message &msg, ClientConnection &user )
{
	std::vector<std::string>	channels;
	std::vector<std::string>	keys;

	std::cout << "[JOIN]" << std::endl;
	if (msg.params.size() == 0)
		sendMessage(user, RPL::errNeedMoreParams("USER"));
	else if (msg.params.size() >= 1)
		channels = split(msg.params[0].value);
	if (msg.params.size() >= 2)
		keys = split(msg.params[1].value);
	for (unsigned long i = 0; i < channels.size(); i++)
	{
		if (channels[i] == "0") {
			quitAllChannels(user);
			return ;
		} else if (i < keys.size())
			joinOneChannel(user, channels[i], keys[i], true);
		else
			joinOneChannel(user, channels[i], channels[i], false);
	}
}

void Server::quitChannel(ClientConnection &user, std::string &channelName, std::string &reason) 
{
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it == channels.end())
	{
		sendMessage(user, RPL::errNoSuchChannel(channelName));
		return;
	}
	Channel *channel = &it->second;
	if (!channel->isOnChannel(user))
	{
		sendMessage(user, RPL::errNotOnChannel(user.username, channelName));
		return;
	}
	std::string content = channelName;
	if (!reason.empty())
		content += reason;
	else
	{
		content += ":LEAVING";
	}
	std::string partMsg = RPL::ircMessageContent(
		user.username,
		"PART",
		channelName,
		content
	);
	broadcastingMessage(user, "PART", partMsg);

	channel->removeUser(user);
	user.activeChannels.erase(channelName);
	if (channel->users.empty())
		channels.erase(it);
}


void Server::partCmd(Message &msg, ClientConnection &user)
{
	//need to make a list of channels
	std::vector<std::string> listChannels;
	if (msg.params.size() < 1)
	{
		sendMessage(user, RPL::errNeedMoreParams("PART"));
		return;
	}
	listChannels = split(msg.params[0].value);
	for(size_t j = 0; j < listChannels.size(); j++)
	{
		std::string reason = msg.params.size() > 1 ? msg.params[1].value : "";
		quitChannel(user, listChannels[j], reason);
	}
}

// format : KICK <channel, ...> <nick, ...> [<reason>]
// either multiple channels or multiple users
// reason broadcasted to all users
void Server::kickCmd(Message &msg, ClientConnection &user)
{
	if (msg.params.size() < 2)
	{
		sendMessage(user, RPL::errNeedMoreParams("KICK"));
		return;
	}
	const std::string &channelName = msg.params[0].value;
	const std::string &nick = msg.params[1].value;

	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it == channels.end())
	{
		sendMessage(user, RPL::errNoSuchChannel(channelName));
		return;
	}
	Channel &channel = it->second;
	if (!channel.isOnChannel(user))
	{
		sendMessage(user, RPL::errNotOnChannel(user.username, channelName));
		return;
	}
	if (!channel.isOperator(user))
	{
		sendMessage(user, RPL::errChanOpPrivsNeeded(user.username, channelName));
		return;
	}
	ClientConnection *target = channel.getUserByNick(nick);
	if (!target)
	{
		sendMessage(user, RPL::errNotOnChannel(nick, channelName));
		return;
	}
	std::string reason;
	if (msg.params.size() >= 3)
		reason = msg.params[2].value;
	std::string content = nick;
	if (!reason.empty())
		content += " :" + reason;
	std::string kickMsg = RPL::ircMessageContent(
		user.username,
		"KICK",
		channelName,
		content
	);
	broadcastingMessage(user, "KICK", kickMsg);
	channel.removeUser(*target);
	target->activeChannels.erase(channelName);
}

// format : INVITE <nickname> <channel>
void	Server::inviteCmd( Message &msg, ClientConnection &user )
{
	std::cout << "[INVITE]" << std::endl;
	(void) msg;
	(void) user;
}

// format : TOPIC [param]
void	Server::topicCmd( Message &msg, ClientConnection &user )
{
	std::cout << "[TOPIC]" << std::endl;
	if (msg.params.size() == 0) {
		sendMessage(user, RPL::errNeedMoreParams("JOIN"));
		return ;
	}
	
	Channel	&channel = *user.activeChannels[msg.params[0].value];
	if (msg.params.size() == 1) {
		sendMessage(user, RPL::rplTopic(user.username, channel.getName(), msg.params[1].value));
	} else if (channel.isOperator(user) == false) {
		sendMessage(user, RPL::errChanOpPrivsNeeded(user.username, channel.getName()));
	} else if (msg.params[1].value.empty()) {
		channel.getTopic() = "";
		sendMessage(user, RPL::rplTopic(user.username, channel.getName(), msg.params[1].value));
	} else {
		channel.setTopic(msg.params[1].value);
		broadcastingMessage(user, "TOPIC", RPL::ircMessageContent(user.username, "TOPIC", channel.getName(), msg.params[1].value));
	}
}

//differenciate channel mode and user mode
void	Server::modeCmd( Message &msg, ClientConnection &user )
{
	std::cout << "[MODE]" << std::endl;
	(void) msg;
	(void) user;
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

// nick <username> / user <username> <	> <servername> <realname> / pass <password>
void	Server::handleRegistration( Message &msg, ClientConnection &user )
{
	switch (msg.command) {
	case NICK:
		{
        std::string wanted = msg.params[0].value;

        if (msg.params.size() == 0)
			sendMessage(user, RPL::errNoNickNameGiven());
        else if (verifNickname(wanted) == FAILURE)
			sendMessage(user, RPL::errErroneusNickname(msg.params[1].value));
        else if (isNicknameAvailable(clients, wanted) == FAILURE)
        {
            wanted = generateFreeNick(wanted);
            if (wanted.empty())
            {
				sendMessage(user, RPL::errNickNameInUse(wanted));
                break;
            }
            sendMessage(user, RPL::errNickNameInUse(wanted));
        }
        user.username = wanted;
       	user.hasNick = true;
        break;
    }
	case USER:
		if (msg.params.size() != 4)
			sendMessage(user, RPL::errNeedMoreParams("USER"));
		else if (user.isRegistered)
			sendMessage(user, RPL::errAlreadyRegistred());
		else {
			user.name = msg.params.back().value;
			user.hasUser = true;
			std::cout << "[USER validated] " << user.name << std::endl;
		}
		break;
	case PASS:
		if (msg.params.size() == 0)
			sendMessage(user, RPL::errNeedMoreParams("PASS"));
		else if (user.isRegistered)
			sendMessage(user, RPL::errAlreadyRegistred());
		else {
			user.connectionPass = msg.params[0].value;
			user.hasPass = true;
			std::cout << "[PASS validated] " << user.connectionPass << std::endl;
		}
		break;
	}
	if (user.hasNick && user.hasUser && user.hasPass)
	{
		std::cout << std::endl << "REGISTRATION OK :" << std::endl;
		std::cout << "Client nickname : " << user.username << std::endl;
		std::cout << "Client username : " << user.name << std::endl;
		std::cout << "Client password : " << user.connectionPass << std::endl;
		std::cout << "Client fd : " << user.fd << std::endl;
		user.isRegistered = true;
		sendMessage(user, RPL::rplWelcome(user.username));
	}
}

void	Server::handleClientMessage( Message &msg, ClientConnection &user )
{
	switch (msg.command) {
	case JOIN:
		joinCmd(msg, user);
		break;
	case TOPIC:
		topicCmd(msg, user);
		break;
	case MODE:
		modeCmd(msg, user);
		break;
	case INVITE:
		inviteCmd(msg, user);
		break;
	case PART:
		partCmd(msg, user);
		break;
	case KICK:
		kickCmd(msg, user);
		break;
	case PRIVMSG:
		broadcastingMessage(user, "PRIVMSG", RPL::ircMessageContent(user.username, "PRIVMSG", msg.params[0].value, msg.params[1].value));
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
		if (this->clients[fd].isRegistered == true)
			handleClientMessage(msg, this->clients[fd]);
		else 
			handleRegistration(msg, this->clients[fd]);
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

