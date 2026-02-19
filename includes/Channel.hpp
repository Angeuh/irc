#ifndef CHANNEL_HPP
# define CHANNEL_HPP
# include "header.hpp"
# include "ClientConnection.hpp"
# include "Client.hpp"
# include "Server.hpp"
# include "RPL.hpp"
# include "Message.hpp"

class ClientConnection;
class Message;
class Server;

class Channel
{
    private:
		std::string		name; // 50 char max, case insensitive ! no ',' or ' '
		std::string		topic;
		std::string		key;
		unsigned long	limit;
		bool			inviteOnly;
			
	public:
		Channel( void );
		~Channel( void );
		Channel( const std::string &, ClientConnection & );
	
		std::vector<ClientConnection>	operators;
		std::vector<ClientConnection>	users;
		bool							hasTopic;
		bool							hasKey;
		bool							hasLimit;

		void		insertUser( ClientConnection & );
		void		removeUser( ClientConnection & );
		bool		isOperator( ClientConnection & );
		bool		isOnChannel( ClientConnection & );
		bool		isInviteOnly();
		bool		isFull();
		std::string	getName();
		std::string	getTopic();
		std::string	getKey();
};

#endif
