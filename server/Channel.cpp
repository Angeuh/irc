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

std::string	Channel::getName()
{
	return (this->name);
}

std::string	Channel::getTopic()
{
	return (this->topic);
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
