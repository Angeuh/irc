#include "../includes/RPL.hpp"

RPL::RPL() {}

RPL::~RPL() {}

//modify for user
//username, command, channel, content
std::string	RPL::ircMessageContent(const std::string &username, const std::string &command, const std::string &channel, const std::string &content)
{
	return (":" + username + "!user@localhost " + command + " " + channel + " :" + content + "\r\n");
}

//username, command, channel
std::string	RPL::ircMessageNoContent(const std::string &username, const std::string &command, const std::string &channel)
{
	return (":" + username + "!user@localhost " + command + " " + channel + "\r\n");
}

//username
std::string	RPL::rplWelcome(const std::string &username)
{
    return (":" + std::string(SERVERNAME) + " 001 " + username + " :Welcome to our ft_irc\r\n");
}

//username, channel
std::string	RPL::rplNoTopic(const std::string &username, const std::string &channel)
{
    return (":" + std::string(SERVERNAME) + " 331 " + username + " " + channel + " :\r\n");
}

//username, channel, topic
std::string	RPL::rplTopic(const std::string &username, const std::string &channel, const std::string &topic)
{
    return (":" + std::string(SERVERNAME) + " 332 " + username + " " + channel + " :" + topic + "\r\n");
}

//username, channel
std::string	RPL::rplInviting(const std::string &username, const std::string &channel, const std::string &topic)
{
    return (":" + std::string(SERVERNAME) + " 341 " + username + " " + channel + " :" + topic + "\r\n");
}

//username, channel object
std::string	RPL::rplNameReply( const std::string &username, Channel &channel )
{
	std::string	res = ":" + std::string(SERVERNAME) + " 353 " + username + " = " + channel.getName() + " :";
	
	for (std::vector<ClientConnection>::iterator it = channel.users.begin(); it != channel.users.end(); it++) {
		if (channel.isOperator(*it))
			res += "@";
		res += it->name;
		res += " ";
	}
	res += "\r\n";
	return (res);
}

//username, channel
std::string	RPL::rplEndOfNames( const std::string &username, const std::string &channel )
{
	return (":" + std::string(SERVERNAME) + " 366 " + username + " " + channel + " :End of /NAMES list\r\n");
}

//command
std::string	RPL::errNeedMoreParams(const std::string &command)
{
    return (":" + std::string(SERVERNAME) + " 461 " + command + " :Not enough parameters\r\n");
}

std::string	RPL::errNotOnChannel(const std::string &command)
{
    return (":" + std::string(SERVERNAME) + " 442 " + command + " :You're not on that channel\r\n");
}

//username, channel
std::string	RPL::errChanOpPrivsNeeded(const std::string &username, const std::string &channel)
{
    return (":" + std::string(SERVERNAME) + " 482 " + username + " " + channel + " :You're not channel operator\r\n");
}

//username, channel
std::string	RPL::errChannelIsFull(const std::string &client, const std::string &channel)
{
    return (":" + std::string(SERVERNAME) + " 471 " + client + " " + channel + " :Cannot join channel (+l)\r\n");
}

//username, channel
std::string	RPL::errInviteOnlyChan(const std::string &client, const std::string &channel)
{
    return (":" + std::string(SERVERNAME) + " 473 " + client + " " + channel + " :Cannot join channel (+i)\r\n");
}

//username, channel
std::string	RPL::errBadChannelKey(const std::string &client, const std::string &channel)
{
    return (":" + std::string(SERVERNAME) + " 475 " + client + " " + channel + " :Cannot join channel (+k)\r\n");
}

std::string	RPL::errNoChanModes(const std::string &command)
{
    return (":" + std::string(SERVERNAME) + " 477 " + command + " :Channel doesn't support modes\r\n");
}

std::string	RPL::errNoNickNameGiven( void )
{
    return (":" + std::string(SERVERNAME) + " 431 :No nickname given\r\n");
}

std::string	RPL::errErroneusNickname( void )
{
	return (":" + std::string(SERVERNAME) + " 432 :Erroneous nickname\r\n");
}

//username
std::string	RPL::errNickNameInUse( const std::string &username )
{
	return (":" + std::string(SERVERNAME) + " 433 * " + username + " :Nickname is already in use\r\n");
}

std::string	RPL::errAlreadyRegistred( void )
{
	return (":" + std::string(SERVERNAME) + " 462 :Unauthorized command (already registered)\r\n");
}

