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

bool	Channel::isOnChannelNick( std::string nickname, std::map<int, clinetConnection> clients)
{
	for (std::set<int>::iterator it = users.begin(); users.end(); it++)
	{
		int userFd = it;
		if (clients[userFd].username == nickname)
			return userFd;
	}

	return (false);
}

bool	Channel::isOnChannel( int user )
{
	return (this->users.find(user) != this->users.end());
}

static bool channelExist(std::map<std::string, Channel>	channels, std::string name)
{
	for (std::set<int>::iterator it = channel.begin(); channel.end(); it++)
	{
		if (it.name = name)
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
	static std::string	errNoSuchChannel( const std::string & );
int		Channel::kickCmd( Message &msg, std::map<int, ClientConnection> &,
							int, std::map<std::string, Channel>	channels;)
{
	if (msg.howManyParam < 3) {
		RPL::sendRPL(clients[fd], RPL::errNeedMoreParams(msg.params[1].value), server);
	} else if (this->isOperator(fd) == false) {   
		RPL::sendRPL(clients[fd], RPL::errChanOpPrivsNeeded(clients[fd].username, this->name), server);
	}
	else if (!isOnChannelNick(msg.param.value[3], clients)) {
		RPL::sendRPL(clients[fd], RPL::errUserNotInChannel(clients[fd].username, this->name), server);
	}
	else if (!channelExist(channels, msg.params.value[2])) {
		RPL::sendRPL(clients[fd], RPL::errNoSuchChannel(msg.params.value[2]), server);
	}
	
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
