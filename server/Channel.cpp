#include "../includes/Channel.hpp"

Channel::Channel() {}

// create channel with user as operator
Channel::Channel( const std::string & n, int user ) :
	name(n),
	inviteOnly(false),
	hasTopic(false),
	hasKey(false),
	hasLimit(false)
{
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

bool	Channel::isInviteOnly()
{
	return (this->inviteOnly);
}

bool	Channel::isFull()
{
	return (hasLimit && this->users.size() == limit);
}

std::string	Channel::getName()
{
	return (this->name);
}

std::string	Channel::getTopic()
{
	return (this->topic);
}

std::string	Channel::getKey()
{
	return (this->key);
}
