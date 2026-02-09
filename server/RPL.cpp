#include "../includes/RPL.hpp"

RPL::RPL() {}

RPL::~RPL() {}

std::string	RPL::rplNoTopic(const std::string &client, const std::string &channel)
{
    return ":" + std::string(SERVERNAME) + " 331 " + client + " " + channel + " :\r\n";
}

std::string	RPL::rplTopic(const std::string &client, const std::string &channel, const std::string &topic)
{
    return ":" + std::string(SERVERNAME) + " 332 " + client + " " + channel + " :" + topic + "\r\n";
}

std::string	RPL::rplInviting(const std::string &client, const std::string &channel, const std::string &topic)
{
    return ":" + std::string(SERVERNAME) + " 341 " + client + " " + channel + " :" + topic + "\r\n";
}

std::string	RPL::errNeedMoreParams(const std::string &command)
{
    return ":" + std::string(SERVERNAME) + " 461 " + command + " :Not enough parameters\r\n";
}

std::string	RPL::errNotOnChannel(const std::string &command)
{
    return ":" + std::string(SERVERNAME) + " 442 " + command + " :You're not on that channel\r\n";
}

std::string	RPL::errChanOpPrivsNeeded(const std::string &client, const std::string &channel)
{
    return ":" + std::string(SERVERNAME) + " 482 " + client + " " + channel + " :You're not channel operator\r\n";
}

std::string	RPL::errNoChanModes(const std::string &command)
{
    return ":" + std::string(SERVERNAME) + " 477 " + command + " :Channel doesn't support modes\r\n";
}

// - Pour les RPL: ":"serverName + " " + RPLnum + " " + RPLmsg + "\r\n";
// - Pour les ERR: ":"serverName + " " + ERRnum + " " + command + " " + ERRmsg + "\r\n";
// - Pour les reponses informatives: ":"nickname"!~"+username"@"serverName + " " + command + " :" + variable + "\r\n";

void RPL::sendRPL(ClientConnection &client, const std::string &content, std::vector<pollfd> &fds)
{
    client.writeBuffer += content;
    std::vector<pollfd>::iterator pit = fds.begin();
    for (; pit != fds.end(); ++pit)
    {
    	if (pit->fd == client.fd)
			break;
	}
	pit->events |= POLLOUT;
    std::cout << "[RPL/ERR] client=" << client.username
        << " channel='" << client.currentChannel
        << "' msg='" << content << "'\n";
    std::cout << "RPL/ERR OK: " << content;
}
