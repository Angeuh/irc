#include "../includes/Channel.hpp"

Channel::Channel() {}

// create channel with user as operator
Channel::Channel( const std::string & n, ClientConnection &user ) :
	name(n),
	inviteOnly(false),
	hasTopic(false),
	hasKey(false),
	hasLimit(false)
{
	this->users.push_back(user);
	this->operators.push_back(user);
}

Channel::~Channel() {
	this->users.clear();
	this->operators.clear();
}

void	Channel::insertUser( const ClientConnection &user )
{
	this->users.push_back(user);
}

void	Channel::removeUser( const ClientConnection &user )
{
	std::vector<ClientConnection>::iterator it;
	
	it = std::find(this->users.begin(), this->users.end(), user);
	if (it != this->users.end())
		this->users.erase(it);
}

bool	Channel::isOperator( const ClientConnection &user )
{
	return (std::find(this->operators.begin(), this->operators.end(), user) != this->operators.end());
}

bool	Channel::isOnChannel( const ClientConnection &user )
{
	return (std::find(this->users.begin(), this->users.end(), user) != this->users.end());
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
