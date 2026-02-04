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

std::string	Channel::rplNoTopic(const std::string &command)
{
    return ":" + std::string(SERVERNAME) + " 331 " + command + " :No topic is set\r\n";
}

std::string	Channel::rplTopic(const std::string &command)
{
    return ":" + std::string(SERVERNAME) + " 332 " + command + " :\r\n";
}

std::string	Channel::errNeedMoreParams(const std::string &command)
{
    return ":" + std::string(SERVERNAME) + " 461 " + command + " :Not enough parameters\r\n";
}

std::string	Channel::errNotOnChannel(const std::string &command)
{
    return ":" + std::string(SERVERNAME) + " 442 " + command + " :You're not on that channel\r\n";
}

std::string	Channel::errChanOpPrivsNeeded(const std::string &command)
{
    return ":" + std::string(SERVERNAME) + " 482 " + command + " :You're not channel operator\r\n";
}

std::string	Channel::errNoChanModes(const std::string &command)
{
    return ":" + std::string(SERVERNAME) + " 477 " + command + " :Channel doesn't support modes\r\n";
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
	// std::cout << "TOPIC COMMAND (param : " << param << ") in channel " << channel.name << std::endl;

	if (this->isOperator(fd) == false) {
		sendingMessage(clients[fd], errChanOpPrivsNeeded("TOPIC"), fds);
	}


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
