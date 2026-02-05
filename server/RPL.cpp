#include "../includes/header.hpp"

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
