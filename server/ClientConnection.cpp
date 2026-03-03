#include "../includes/ClientConnection.hpp"

ClientConnection::ClientConnection() : 
	    fd(0),
		lastActivity(0),
		hasNick(false),
		hasUser(false),
		hasPass(false),
		isRegistered(false),
		waitingForPong(false)
{}

ClientConnection::~ClientConnection() {}

bool	ClientConnection::operator==( const ClientConnection& other ) const
{
	return (this->fd == other.fd);
}

void	ClientConnection::inviteUser(Channel &channel){
	invitedChannels[channel.getName()] = &channel;
}
