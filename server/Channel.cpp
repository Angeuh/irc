#include "../includes/Channel.hpp"

Channel::Channel() {}

Channel::Channel(const std::string & n, int user) : name(n), topic("") {
	this->users.insert(user);
	this->operators.insert(user);
}

Channel::~Channel() {
	this->users.clear();
	this->operators.clear();
}

void	Channel::insertUser(int user)
{
	this->users.insert(user);
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

// format : TOPIC []
// no param : show topic
// empty param : remove topic
// else replace/set topic + tell all users
int		Channel::topicCmd( std::string &param, Channel &channel,
	std::map<int, ClientConnection> &clients, int fd, std::vector<pollfd> &fds )
{
	std::cout << "TOPIC COMMAND (param : " << param << ") in channel " << channel.name << std::endl;
	if (param.empty())
		channel.topic = "";
	else {
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
