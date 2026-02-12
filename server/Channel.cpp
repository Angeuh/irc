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
	int fd, std::vector<pollfd> &fds )
{
	(void) msg;
	(void) clients;
	(void) fd;
	(void) fds;
	// if (this->isOperator(fd) == false && this->isInviteOnly == true) 
		// RPL::sendRPL(clients[fd], RPL::errChanOpPrivsNeeded(clients[fd].username, this->name), fds);
	// if (this->isInviteOnly)
	// 	std::cout << "ok\n";
	return (1);
}

// format : TOPIC [param]
int		Channel::topicCmd( Message &msg, std::map<int, ClientConnection> &clients,
	int fd, std::vector<pollfd> &fds )
{
	if (msg.howManyParam == 0) {
		RPL::sendRPL(clients[fd], RPL::rplTopic(clients[fd].username, this->name, msg.params[1].value), fds);
	} else if (this->isOperator(fd) == false) {
		RPL::sendRPL(clients[fd], RPL::errChanOpPrivsNeeded(clients[fd].username, this->name), fds);
	} else if (msg.params[1].value.empty()) {
		this->topic = "";
		RPL::sendRPL(clients[fd], RPL::rplTopic(clients[fd].username, this->name, msg.params[1].value), fds);
	} else {
		this->topic = msg.params[1].value;
		broadcastingMessage(clients, msg.params[0].value, "TOPIC", fd, fds);
	}
	return (1);
}

int		Channel::modeCmd( Message &msg )
{
	(void) msg;
	return (1);
}
