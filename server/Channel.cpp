#include "../includes/Channel.hpp"

Channel::Channel() {}

// create channel with user as operator
Channel::Channel( const std::string & n, ClientConnection &user ) :
	name(n),
	inviteOnly(false),
	hasTopic(false),
	hasKey(false),
	hasLimit(false),
	hasTopicRestriction(true)
{
	this->users.push_back(&user);
	this->operators.push_back(&user);
}

Channel::~Channel() {
	this->users.clear();
	this->operators.clear();
	for (std::vector<ClientConnection *>::iterator it = invitedUsers.begin(); it != invitedUsers.end(); ++it) {
		(*it)->invitedChannels.erase(this->name);
	}
}

bool	Channel::operator==(  Channel other ) const
{
	return (this->getName() == other.getName());
}

void	Channel::insertUser(  ClientConnection &user )
{
	this->users.push_back(&user);
}

void	Channel::removeUser( const ClientConnection &user )
{
	std::vector<ClientConnection*>::iterator it;
	
	it = std::find(this->users.begin(), this->users.end(), &user);
	if (it != this->users.end())
		this->users.erase(it);
}

void	Channel::insertOperator(  ClientConnection &op )
{
	this->operators.push_back(&op);
}

void	Channel::removeOperator(  ClientConnection &op )
{
	std::vector<ClientConnection*>::iterator it;
	
	it = std::find(this->operators.begin(), this->operators.end(), &op);
	if (it != this->operators.end())
		this->operators.erase(it);
}

bool	Channel::isOperator(  ClientConnection &user )
{
	return (std::find(this->operators.begin(), this->operators.end(), &user) != this->operators.end());
}

bool	Channel::isOnChannel(  ClientConnection &user )
{
	return (std::find(this->users.begin(), this->users.end(), &user) != this->users.end());
}

bool	Channel::isFull()
{
	return (hasLimit && this->users.size() >= limit);
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

unsigned long	Channel::getLimit() const
{
	return (this->limit);
}

void	Channel::setLimit( const unsigned long newLimit )
{
	this->limit = newLimit;
}

ClientConnection* Channel::getUserByNick(const std::string &nick)
{
	for (std::vector<ClientConnection *>::iterator it = users.begin();
		 it != users.end();
		 ++it)
	{
		if ((*it)->username == nick)
			return *it;
	}
	return NULL;
}

void Channel::inviteUser(ClientConnection &user) {
    invitedUsers.push_back(&user);
}

bool Channel::isInvited(ClientConnection &user) {
    for (size_t i = 0; i < invitedUsers.size(); ++i) {
        if (invitedUsers[i]->username == user.username)
            return true;
    }
    return false;
}

void Channel::removeInvitation(ClientConnection &user) {
    for (std::vector<ClientConnection*>::iterator it = invitedUsers.begin(); it != invitedUsers.end(); ++it) {
        if ((*it)->username == user.username) {
            invitedUsers.erase(it);
            break;
        }
    }
}
