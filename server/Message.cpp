#include "../includes/header.hpp"

Message::Message() {}

Message::Message(const char * buf, int bytesReceived) : 
	rawMessage(std::string(buf, bytesReceived))
{
	std::cout << rawMessage << std::endl;

	this->parseMessage();
}

Message::~Message() {}

void	Message::parseMessage()
{
	if (this->rawMessage.empty())
		throw ParsingError();
	std::string				msg(this->rawMessage);
	std::string::iterator	pos;
	if (msg[0] == ':')
	{
		pos = msg.find(" ");
		if (pos == std::string::npos)
			throw ParsingError();
		this->prefix = msg.substr(0, pos);
		msg.erase(0, pos + 1);
	}
	pos = msg.find(" ");

}

char const *Message::ParsingError::what() const throw()
{
    return "Parsing failed";
}

void	Message::printParsedMessage( void ) const
{
	std::cout << "RAW : [" << this->rawMessage << "]" << std::endl;
	std::cout << "PREFIX : [" << this->prefix << "]" << std::endl;
	std::cout << "COMMAND : [" << this->command << "]" << std::endl;
	std::cout << "PARAMS : " << std::endl;
	for (std::vector::iterator it = this->params.begin(); it < this->params.end(); it++)
		std::cout << "[" << it << "]" << std::endl;
}
