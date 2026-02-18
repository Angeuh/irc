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

//send content to all users in channel (except for user if PRIVMSG)
void Server::broadcastingMessage( ClientConnection &user, const std::string &command, const std::string &content )
{
	bool skipUser = (command == "PRIVMSG");
    for (std::map<int, ClientConnection>::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
    {
        ClientConnection &client = it->second;
        if (client.currentChannel == user.currentChannel && (!skipUser || client.username != user.username))
            sendMessage(client, content);
        std::cout << "[BROADCAST] receiver =" << client.username << std::endl;
    }
    std::cout << "Broadcast OK: " << content;
}

// - Pour les RPL: ":"serverName + " " + RPLnum + " " + RPLmsg + "\r\n";
// - Pour les ERR: ":"serverName + " " + ERRnum + " " + command + " " + ERRmsg + "\r\n";
// - Pour les reponses informatives: ":"nickname"!~"+username"@"serverName + " " + command + " :" + variable + "\r\n";

//send content to client
void Server::sendMessage(ClientConnection &client, const std::string &content)
{
    client.writeBuffer += content;
    this->modifyEpoll(client.fd, EPOLLIN | EPOLLOUT);
    std::cout << "[RPL/ERR] sender=" << client.username
        << " channel='" << client.currentChannel
        << "' msg='" << content << "'\n";
    std::cout << "RPL/ERR OK: " << content << std::endl;
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
	sendMessage(user, RPL::rplNamReply(channel));
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
		this->channels[channelName] = Channel(channelName, user.fd);
		welcomeToChannel(this->channels[channelName], user);
		return ;
	}
	else {
		Channel	channel = it->second;
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
			channel.insertUser(user.fd);
			welcomeToChannel(channel, user);
		}
	}		
}

static std::vector<std::string> split( const std::string& str ) {
    std::vector<std::string>	res;
    size_t 						start = 0;
    size_t						end = str.find(',');
    
    while (end != std::string::npos) {
        res.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(',', start);
    }
    res.push_back(str.substr(start));
    return (res);
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
	channels = split(msg.params[0].value);
	keys = split(msg.params[1].value);
    
	for (unsigned long i = 0; i < channels.size(); i++)
	{
		if (i < keys.size())
			joinOneChannel(user, channels[i], keys[i], true);
		else
			joinOneChannel(user, channels[i], channels[i], false);
	}
	
}

// format : KICK <channel, ...> <nick, ...> [<reason>]
// either multiple channels or multiple users
// reason broadcasted to all users
void	Server::kickCmd( Message &msg, ClientConnection &user )
{
	std::cout << "[KICK]" << std::endl;
	(void) msg;
	(void) user;
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
	Channel	channel; //to do

	std::cout << "[TOPIC]" << std::endl;
	if (msg.params.size() == 0) {
		sendMessage(user, RPL::rplTopic(user.username, channel.getName(), msg.params[1].value));
	} else if (channel.isOperator(user.fd) == false) {
		sendMessage(user, RPL::errChanOpPrivsNeeded(user.username, channel.getName()));
	} else if (msg.params[1].value.empty()) {
		channel.getTopic() = "";
		sendMessage(user, RPL::rplTopic(user.username, channel.getName(), msg.params[1].value));
	} else {
		channel.getTopic() = msg.params[1].value;
		broadcastingMessage(user, "TOPIC", RPL::ircMessageContent(user.username, "TOPIC", channel.getName(), msg.params[1].value));
	}
}

void	Server::modeCmd( Message &msg, ClientConnection &user )
{
	std::cout << "[MODE]" << std::endl;
	(void) msg;
	(void) user;
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

// nick <username> / user <username> <hostname> <servername> <realname> / pass <password>
void	Server::handleRegistration( Message &msg, ClientConnection &user )
{
	switch (msg.command) {
	case NICK:
		if (msg.params.size() == 0)
			sendMessage(user, RPL::errNoNickNameGiven());
		else if (verifNickname(msg.params[0].value) == FAILURE) {
			sendMessage(user, RPL::errErroneusNickname());
		} else if (isNicknameAvailable(this->clients, msg.params[0].value) == FAILURE) {
			sendMessage(user, RPL::errNickNameInUse(user.username));
		} else {
			user.username = msg.params[0].value;
			user.hasNick = true;
		}
		break;
	case USER:
		if (msg.params.size() != 4)
			sendMessage(user, RPL::errNeedMoreParams("USER"));
		else if (user.isRegistered)
			sendMessage(user, RPL::errAlreadyRegistred());
		else {
			user.name = msg.params.back().value;
			user.hasUser = true;
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
		sendMessage(user, RPL::rplWelcome(user.username));
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
	case PRIVMSG:
		broadcastingMessage(user, "PRIVMSG", RPL::ircMessageContent(user.username, "TOPIC", user.currentChannel, msg.params[0].value));
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

