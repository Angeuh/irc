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
        std::set<int>	users;
        std::set<int>	operators;
		bool			isInviteOnly;

    public:
        Channel( void );
        ~Channel( void );
        Channel( const std::string &, int );

		void	insertUser(int);
		bool	isOperator( int );
		bool	isOnChannel( int );
		bool	getIsInviteOnly();
		int	isOnChannelNick( std::string nickname, std::map<int, ClientConnection> clients);

		int		kickCmd( Message & , std::map<int, ClientConnection> &, int, Server &server);
		int		inviteCmd( Message &, std::map<int, ClientConnection> &, int);
		int		topicCmd( Message &, std::map<int, ClientConnection> &, int, Server &Server);
		int		modeCmd( Message & );
};

#endif
