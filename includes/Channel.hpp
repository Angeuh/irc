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
		std::string						name; // 50 char max, case insensitive ! no ',' or ' '
		std::string						topic;
		std::string						key;
		bool							hasKey;
		int								limit;
		bool							hasLimit;
        std::set<ClientConnection &>	users;
        std::set<ClientConnection &>	operators;
		bool							inviteOnly;

    public:
        Channel( void );
        ~Channel( void );
        Channel( const std::string &, const ClientConnection & );

		void		insertUser(int);
		bool		isOperator( int ); //switch to client connection
		bool		isOnChannel( int );
		bool		isInviteOnly();
		bool		isFull();
		std::string	getName();
		std::string	getTopic();
};

#endif
