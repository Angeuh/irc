#include "../includes/ClientConnection.hpp"

ClientConnection::ClientConnection() : 
	    fd(0),
		hasNick(false),
		hasUser(false),
		hasPass(false),
		isRegistered(false)
{}

ClientConnection::~ClientConnection() {}
