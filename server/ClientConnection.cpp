#include "../includes/ClientConnection.hpp"

ClientConnection::ClientConnection() : 
	    fd(0),
		hasNick(false),
		hasUser(false),
		isRegistered(false)
{}

ClientConnection::~ClientConnection() {}
