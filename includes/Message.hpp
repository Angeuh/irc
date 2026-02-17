#ifndef MESSAGE_HPP
# define MESSAGE_HPP
# include "header.hpp"
# include "ClientConnection.hpp"
# include "Client.hpp"
# include "Channel.hpp"
# include "Server.hpp"
# include "RPL.hpp"

enum states
{
	START,
	PREFIX,
	COMMAND,
	PARAMS,
	TRAILING,
	END
};

typedef struct s_token
{
	int			start;
	int			end;
	std::string	value;
}	t_token;

enum actions
{
	CAP,
	NICK,
	USER,
	PASS,
	JOIN,
	TOPIC,
	MODE,
	INVITE,
	KICK,
	PRIVMSG,
	DEFAULT
};

// irc message : [':'prefix] <command> [parameters (max 15)] <CR-LF>
// not more than 512 char including CR-LF
class Message
{
	private:
		void	parseMessage( void );
		void	chooseCommand( void );
	
	public:
		Message( void );
		Message( const std::string& );
		Message( const char *, int );
		~Message( void );

		std::string				rawMessage;
		t_token					prefix;
		t_token					commandToken;
		int						command;
		std::vector<t_token>	params;
		bool					hasTrailing;

		class ParsingError : public std::exception {
    		public:
    	    	const char* what() const throw();
    	};
};

std::ostream	&operator<<(std::ostream &os, Message &msg);

#endif
