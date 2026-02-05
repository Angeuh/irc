#include "../includes/header.hpp"

Channel::Channel() {}

Channel::Channel(const std::string & n, int user) : name(n), topic("") {
	this->users.insert(user);
	this->operators.insert(user);
}

Channel::~Channel() {
	this->users.clear();
	this->operators.clear();
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
int		Channel::kickCmd( std::string &param )
{
	(void) param;
	return (1);
}

int		Channel::inviteCmd( std::string &param )
{
	(void) param;
	return (1);
}

// format : TOPIC [param]
int		Channel::topicCmd( std::string &param, bool hasParam, Channel &channel,
	std::map<int, ClientConnection> &clients, int fd, std::vector<pollfd> &fds )
{
	if (hasParam == false) { // no param : show topic
		sendingMessage(clients[fd], RPL::rplTopic(clients[fd].username, channel.name, param), fds);
	} else if (this->isOperator(fd) == false) { //not operator
		sendingMessage(clients[fd], RPL::errChanOpPrivsNeeded(clients[fd].username, channel.name), fds);
	} else if (param.empty()) { // empty param : remove topic
		channel.topic = "";
		sendingMessage(clients[fd], RPL::rplTopic(clients[fd].username, channel.name, param), fds);
	} else { // replace/set topic + tell all users
		channel.topic = param;
		broadcastingMessage(clients, param, "TOPIC", fd, fds);
	}
	return (1);
}

int		Channel::modeCmd( std::string &param )
{
	(void) param;
	return (1);
}
