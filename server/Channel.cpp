#include "../includes/Channel.hpp"

Channel::Channel() {}

// isInviteOnly(false) 
Channel::Channel(const std::string & n, int user) : name(n), topic("") {
	this->users.insert(user);
	this->operators.insert(user);
}

Channel::~Channel() {
	this->users.clear();
	this->operators.clear();
}

bool	Channel::getIsInviteOnly()
{
	return (this->isInviteOnly);
}

void	Channel::insertUser( int user )
{
	this->users.insert(user);
}

bool	Channel::isOperator( int user )
{
	return (this->operators.find(user) != this->operators.end());
}

int	Channel::isOnChannelNick( std::string nickname, std::map<int, ClientConnection> clients)
{
	for (std::set<int>::iterator it = users.begin(); it != users.end(); it++)
	{
		int userFd = *it;
		if (clients[userFd].username == nickname)
			return userFd;
	}

	return (-1);
}

bool	Channel::isOnChannel( int user )
{
	return (this->users.find(user) != this->users.end());
}

static bool channelExist(Server &server, std::string name)
{
	std::map<std::string, Channel> channels = server.getChannels();
	for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); it++)
	{
		if (it->first == name)
			return 1;
	}
	return (-1);
}

// format : KICK <channel, ...> <nick, ...> [<reason>]
// either multiple channels or multiple users

//    The server MUST NOT send KICK messages with multiple channels or
//    users to clients.  This is necessarily to maintain backward
//    compatibility with old client software.

// reason broadcasted to all users

// ERR_NEEDMOREPARAMS              ERR_NOSUCHCHANNEL
//            ERR_BADCHANMASK                 ERR_CHANOPRIVSNEEDED
//            ERR_USERNOTINCHANNEL            ERR_NOTONCHANNEL
int		Channel::kickCmd( Message &msg, std::map<int, ClientConnection> &clients,
							int fd, Server &server)
{
	if (msg.howManyParam < 3) {
		RPL::sendRPL(clients[fd], RPL::errNeedMoreParams(msg.params[1].value), server);
	} else if (this->isOperator(fd) == false) {   
		RPL::sendRPL(clients[fd], RPL::errChanOpPrivsNeeded(clients[fd].username, this->name), server);
	}
	else if (!isOnChannelNick(msg.params[3].value, clients)) {
		RPL::sendRPL(clients[fd], RPL::errUserNotInChannel(clients[fd].username, this->name), server);
	}
	else if (!channelExist(server, msg.params[2].value)) {
		RPL::sendRPL(clients[fd], RPL::errNoSuchChannel(msg.params[2].value), server);
	}

	int targetFd = isOnChannelNick(msg.params[1].value, clients);
    if (targetFd == -1 || !isOnChannel(targetFd))
	{
		RPL::sendRPL(clients[fd], RPL::errUserNotInChannel(clients[fd].username, this->name), server);
        return (-1);
	}

	// ?should broadcast the msg user kicked? to user and all on channel
	//broadcastingMessage()?

	//should end smt like this
	users.erase(targetFd);
    operators.erase(targetFd);
    clients[targetFd].currentChannel.clear();
	return (0);
}

// format : INVITE <nickname> <channel>
int		Channel::inviteCmd( Message &msg, std::map<int, ClientConnection> &clients,
	int fd)
{
	(void) msg;
	(void) clients;
	(void) fd;
	// if (this->isOperator(fd) == false && this->isInviteOnly == true) 
		// RPL::sendRPL(clients[fd], RPL::errChanOpPrivsNeeded(clients[fd].username, this->name), fds);
	// if (this->isInviteOnly)
	// 	std::cout << "ok\n";
	return (0);
}

// format : TOPIC [param]
int		Channel::topicCmd( Message &msg, std::map<int, ClientConnection> &clients,
	int fd, Server &server)
{
	if (msg.howManyParam == 0) {
		RPL::sendRPL(clients[fd], RPL::rplTopic(clients[fd].username, this->name, msg.params[1].value), server);
	} else if (this->isOperator(fd) == false) {
		RPL::sendRPL(clients[fd], RPL::errChanOpPrivsNeeded(clients[fd].username, this->name), server);
	} else if (msg.params[1].value.empty()) {
		this->topic = "";
		RPL::sendRPL(clients[fd], RPL::rplTopic(clients[fd].username, this->name, msg.params[1].value), server);
	} else {
		this->topic = msg.params[1].value;
		server.broadcastingMessage(clients, msg.params[0].value, "TOPIC", fd);
	}
	return (1);
}

int		Channel::modeCmd( Message &msg )
{
	(void) msg;
	return (1);
}
