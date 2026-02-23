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
			
	public:
		Channel( void );
		~Channel( void );
		Channel( const std::string &, ClientConnection & );
		
		bool	operator==( const Channel ) const;

		std::vector<ClientConnection>	operators;
		std::vector<ClientConnection>	users;
		bool							inviteOnly;
		bool							hasTopic;
		bool							hasKey;
		bool							hasLimit;
		bool							hasTopicRestriction;

		void				insertUser( const ClientConnection & );
		void				removeUser( const ClientConnection & );
		void				insertOperator( const ClientConnection & );
		void				removeOperator( const ClientConnection & );
		bool				isOperator( const ClientConnection & );
		bool				isOnChannel( const ClientConnection & );
		bool				isFull();
		std::string			getName() const;
		std::string			getTopic() const;
		void				setTopic( const std::string & );
		std::string			getKey() const;
		void				setKey( const std::string & );
		unsigned long		getLimit() const;
		void				setLimit( const unsigned long );
};

#endif
