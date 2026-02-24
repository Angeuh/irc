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
		
		bool	operator==( const Channel ) const;

		std::vector<ClientConnection>	operators;
		std::vector<ClientConnection>	users;
		bool							hasTopic;
		bool							hasKey;
		bool							hasLimit;

		void		insertUser( const ClientConnection & );
		void		removeUser( const ClientConnection & );
		bool		isOperator( const ClientConnection & );
		bool		isOnChannel( const ClientConnection & );
		bool		isInviteOnly();
		bool		isFull();
		std::string	getName() const;
		std::string	getTopic() const;
		void		setTopic( const std::string & );
		std::string	getKey() const;
		void		setKey( const std::string & );
		ClientConnection* getUserByNick(const std::string &nick);


};

#endif
