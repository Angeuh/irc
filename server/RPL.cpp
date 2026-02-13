#include "../includes/RPL.hpp"

RPL::RPL() {}

RPL::~RPL() {}

//username
std::string	RPL::rplWelcome(const std::string &username)
{
    return ":" + std::string(SERVERNAME) + " 001 " + username + " :Welcome to our ft_irc\r\n";
}

//username, channel
std::string	RPL::rplNoTopic(const std::string &username, const std::string &channel)
{
    return ":" + std::string(SERVERNAME) + " 331 " + username + " " + channel + " :\r\n";
}

//username, channel, topic
std::string	RPL::rplTopic(const std::string &username, const std::string &channel, const std::string &topic)
{
    return ":" + std::string(SERVERNAME) + " 332 " + username + " " + channel + " :" + topic + "\r\n";
}

//username, channel
std::string	RPL::rplInviting(const std::string &username, const std::string &channel, const std::string &topic)
{
    return ":" + std::string(SERVERNAME) + " 341 " + username + " " + channel + " :" + topic + "\r\n";
}

//command
std::string	RPL::errNeedMoreParams(const std::string &command)
{
    return ":" + std::string(SERVERNAME) + " 461 " + command + " :Not enough parameters\r\n";
}

std::string	RPL::errNotOnChannel(const std::string &command)
{
    return ":" + std::string(SERVERNAME) + " 442 " + command + " :You're not on that channel\r\n";
}

//username, channel
std::string	RPL::errChanOpPrivsNeeded(const std::string &username, const std::string &channel)
{
    return ":" + std::string(SERVERNAME) + " 482 " + username + " " + channel + " :You're not channel operator\r\n";
}

std::string	RPL::errInviteOnlyChan(const std::string &client, const std::string &channel)
{
    return ":" + std::string(SERVERNAME) + " 473 " + client + " " + channel + " :Cannot join channel\r\n";
}

std::string	RPL::errNoChanModes(const std::string &command)
{
    return ":" + std::string(SERVERNAME) + " 477 " + command + " :Channel doesn't support modes\r\n";
}

std::string	RPL::errNoNickNameGiven( void )
{
    return ":" + std::string(SERVERNAME) + " 431 :No nickname given\r\n";
}

std::string	RPL::errErroneusNickname( void )
{
	return ":" + std::string(SERVERNAME) + " 432 :Erroneous nickname\r\n";
}

std::string	RPL::errNickNameInUse( void )
{
	return ":" + std::string(SERVERNAME) + " 433 :Nickname is already in use\r\n";
}

std::string	RPL::errAlreadyRegistred( void )
{
	return ":" + std::string(SERVERNAME) + " 462 :Unauthorized command (already registered)\r\n";
}
// - Pour les RPL: ":"serverName + " " + RPLnum + " " + RPLmsg + "\r\n";
// - Pour les ERR: ":"serverName + " " + ERRnum + " " + command + " " + ERRmsg + "\r\n";
// - Pour les reponses informatives: ":"nickname"!~"+username"@"serverName + " " + command + " :" + variable + "\r\n";

void RPL::sendRPL(ClientConnection &client, const std::string &content, Server &server)
{
    client.writeBuffer += content;
    server.modifyEpoll(client.fd, EPOLLIN | EPOLLOUT);
    std::cout << "[RPL/ERR] sender=" << client.username
        << " channel='" << client.currentChannel
        << "' msg='" << content << "'\n";
    std::cout << "RPL/ERR OK: " << content << std::endl;
}
