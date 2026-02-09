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

// format : INVITE <nickname> <channel>
int		Channel::inviteCmd( std::string &param, bool hasParam,
	std::map<int, ClientConnection> &clients, int fd, std::vector<pollfd> &fds )
{
	(void) param;
	(void) hasParam;
	(void) clients;
	(void) fd;
	(void) fds;
	// if (this->isOperator(fd) == false && this->isInviteOnly == true) 
		// RPL::sendRPL(clients[fd], RPL::errChanOpPrivsNeeded(clients[fd].username, this->name), fds);
	if (this->isInviteOnly)
		std::cout << "ok\n";
	return (1);
}

// format : TOPIC [param]
int		Channel::topicCmd( std::string &param, bool hasParam,
	std::map<int, ClientConnection> &clients, int fd, std::vector<pollfd> &fds )
{
	if (hasParam == false) {
		RPL::sendRPL(clients[fd], RPL::rplTopic(clients[fd].username, this->name, param), fds);
	} else if (this->isOperator(fd) == false) {
		RPL::sendRPL(clients[fd], RPL::errChanOpPrivsNeeded(clients[fd].username, this->name), fds);
	} else if (param.empty()) {
		this->topic = "";
		RPL::sendRPL(clients[fd], RPL::rplTopic(clients[fd].username, this->name, param), fds);
	} else {
		this->topic = param;
		broadcastingMessage(clients, param, "TOPIC", fd, fds);
	}
	return (1);
}

int		Channel::modeCmd( std::string &param )
{
	(void) param;
	return (1);
}
