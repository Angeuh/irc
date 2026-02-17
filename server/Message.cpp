#include "../includes/Message.hpp"

Message::Message() {}

Message::Message( const std::string& msg ) :
	rawMessage(msg),
	howManyParam(0),
	hasTrailing(false)
{
	this->prefix.value = "";
	this->commandToken.value = "";
	this->parseMessage();
	this->chooseCommand();
}

Message::Message(const char *buf, int bytesReceived) : 
	rawMessage(std::string(buf, bytesReceived)),
	howManyParam(0),
	hasTrailing(false)
{
	std::cout << rawMessage << std::endl;
	this->prefix.value = "";
	this->commandToken.value = "";
	this->parseMessage();
	this->chooseCommand();
}

Message::~Message() {}

void	Message::chooseCommand( void )
{
	std::map<std::string, int>	commands;
	
	commands["CAP"] = CAP;
	commands["NICK"] = NICK;
	commands["USER"] = USER;
	commands["PASS"] = PASS;
	commands["JOIN"] = JOIN;
	commands["TOPIC"] = TOPIC;
	commands["MODE"] = MODE;
	commands["INVITE"] = INVITE;
	commands["KICK"] = KICK;
	commands["PRIVMSG"] = PRIVMSG;
	commands["DEFAULT"] = DEFAULT;

	switch (commands[this->commandToken.value]) {
	case CAP:
		this->command = CAP;
		break;
	case NICK:
		this->command = NICK;	
		break;
	case USER:
		this->command = USER;
		break;
	case PASS:
		this->command = PASS;
		break;
	case JOIN:
		this->command = JOIN;
		break;
	case TOPIC:
		this->command = TOPIC;
		break;
	case MODE:
		this->command = MODE;
		break;
	case INVITE:
		this->command = INVITE;
		break;
	case KICK:
		this->command = KICK;
		break;
	case PRIVMSG:
		this->command = PRIVMSG;
		break;
	default:
		this->command = DEFAULT;
	}
}

static void	skipSpaces(std::string &msg, unsigned long &i)
{
	while (i < msg.length() && msg[i] == ' ')
		i++;
}

static void	readUntilDelimiter(std::string &msg, unsigned long &i, std::string delim)
{
	while (i < msg.length() && delim.find(msg[i]) == std::string::npos)
		i++;
}

static void	makeToken(unsigned long start, unsigned long end, t_token &token, std::string raw)
{
	token.start = start;
	token.end = end;
	token.value = raw.substr(start, end - start);
	// std::cout << "[TOKEN] " << token.value << "[end]" << std::endl;
}

void	Message::parseMessage()
{
	if (this->rawMessage.empty())
		throw ParsingError();
	states			state = START;
	t_token			token;
	unsigned long	i = 0;
	unsigned long	tokenStart = 0;

	while (i < this->rawMessage.length())
	{
		// std::cout << "[STATE=" << state << "] i=" << i;
		// std::cout << " char='" << this->rawMessage[i] << "' tokenStart='" << tokenStart << "'" << std::endl;
		switch (state) {
		case START:
			if (this->rawMessage[i] == ':') {
				state = PREFIX;
				i++;
			}
			else
				state = COMMAND;
			tokenStart = i;
			break;
		case PREFIX:
			readUntilDelimiter(this->rawMessage, i, " ");
			makeToken(tokenStart, i, this->prefix, this->rawMessage);
			state = COMMAND;
			skipSpaces(this->rawMessage, i);
			tokenStart = i;
			break;
		case COMMAND:
			readUntilDelimiter(this->rawMessage, i, "\r ");
			makeToken(tokenStart, i, this->commandToken, this->rawMessage);
			if (this->rawMessage[i] == '\r') {
				state = END;
			} else if (this->rawMessage[i] == ' ') {
				state = PARAMS;
				skipSpaces(this->rawMessage, i);
				if (this->rawMessage[i] == ':') {
					state = TRAILING;
					i++;
				}
			}
			tokenStart = i;
			break;
		case PARAMS:
			readUntilDelimiter(this->rawMessage, i, " :\r");
			makeToken(tokenStart, i, token, this->rawMessage);
			this->params.push_back(token);
			if (this->rawMessage[i] == '\r') {
				state = END;
			} else if (this->rawMessage[i] == ' ') {
				skipSpaces(this->rawMessage, i);
				if (this->rawMessage[i] == ':') {
					state = TRAILING;
					i++;
				}
			}
			tokenStart = i;
			break;
		case TRAILING:
			readUntilDelimiter(this->rawMessage, i, "\r");
			makeToken(tokenStart, i, token, this->rawMessage);
			this->params.push_back(token);
			this->hasTrailing = true;
			if (token.value.empty()) {
				this->params.pop_back();
				this->hasTrailing = false;
			}
			state = END;
			tokenStart = i;
			break;
		case END:
			if (this->rawMessage[i] != '\r' || i + 1 == this->rawMessage.length())
				throw ParsingError();
			if (this->rawMessage[i + 1] != '\n')
				throw ParsingError();
			while (i < this->rawMessage.length())
				i++;
			break;
		}
	}
}

char const *Message::ParsingError::what() const throw()
{
    return "Parsing failed";
}

std::ostream	&operator<<(std::ostream &os, Message &msg)
{
	os << std::endl << "-- MESSAGE --" << std::endl;
	os << "RAW : " << msg.rawMessage;
	os << "PREFIX : " << msg.prefix.value << std::endl;
	os << "COMMAND : " << msg.commandToken.value << std::endl;
	os << "PARAMS : " << std::endl;
	int	i = 0;
	for (std::vector<t_token>::iterator it = msg.params.begin(); it != msg.params.end(); ++it)
		os << "\t[" << i++ << "] " << (*it).value << std::endl;
	return (os);
}
