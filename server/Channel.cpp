#include "../includes/Channel.hpp"

Channel::Channel() {}

Channel::Channel(const std::string & n, int user) : name(n), topic(""), isInviteOnly(false) {
	this->users.insert(user);
	this->operators.insert(user);
}

Channel::~Channel() {
	this->users.clear();
	this->operators.clear();
}

bool	Channel::get_isInviteOnly()
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

bool	Channel::isOnChannel( int user )
{
	return (this->users.find(user) != this->users.end());
}

// format : KICK <channel, ...> <nick, ...> [<reason>]
// either multiple channels or multiple users
// reason broadcasted to all users
int		Channel::kickCmd( Message &msg )
{
	(void) msg;
	return (1);
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
	if (this->isInviteOnly)
		std::cout << "ok\n";
	return (1);
}

// format : TOPIC [param]
int		Channel::topicCmd( Message &msg, std::map<int, ClientConnection> &clients,
	int fd)
{
	(void) msg;
	(void) clients;
	(void) fd;
	// if (msg.hasParam() == false) {
	// 	RPL::sendRPL(clients[fd], RPL::rplTopic(clients[fd].username, this->name, param), fds);
	// } else if (this->isOperator(fd) == false) {
	// 	RPL::sendRPL(clients[fd], RPL::errChanOpPrivsNeeded(clients[fd].username, this->name), fds);
	// } else if (param.empty()) {
	// 	this->topic = "";
	// 	RPL::sendRPL(clients[fd], RPL::rplTopic(clients[fd].username, this->name, param), fds);
	// } else {
	// 	this->topic = param;
	// 	broadcastingMessage(clients, param, "TOPIC", fd, fds);
	// }
	return (1);
}

int		Channel::modeCmd( Message &msg )
{
	(void) msg;
	return (1);
}
