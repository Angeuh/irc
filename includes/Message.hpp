#ifndef MESSAGE_HPP
# define MESSAGE_HPP
# include "header.hpp"

// irc message : [':'prefix] <command> [parameters (max 15)] <CR-LF>
// not more than 512 char including CR-LF
class Message
{
	private:
		std::string					rawMessage;
		std::string					prefix;
		std::string					command;
		std::vector<std::string>	params;

		void		parseMessage( void );

	public:
		Message( void );
		Message( const char *, int );
		~Message( void );

		void		printParsedMessage( void ) const;
	class ParsingError : public std::exception {
    	public:
        	const char* what() const throw();
    };
};

#endif
