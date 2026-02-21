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

bool	Channel::operator==( const Channel other ) const
{
	return (this->getName() == other.getName());
}

void	Channel::insertUser( const ClientConnection &user )
{
	this->users.push_back(user);
}

ClientConnection* Channel::getUserByNick(const std::string &nick)
{
	for (std::vector<ClientConnection>::iterator it = users.begin();
		 it != users.end();
		 ++it)
	{
		if (it->username == nick)
			return &(*it);
	}
	return NULL;
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

std::string	Channel::getName() const
{
	return (this->name);
}

std::string	Channel::getTopic() const
{
	return (this->topic);
}

void	Channel::setTopic( const std::string &newTopic )
{
	this->topic = newTopic;
}

std::string	Channel::getKey() const
{
	return (this->key);
}

void	Channel::setKey( const std::string &newKey )
{
	this->key = newKey;
}
