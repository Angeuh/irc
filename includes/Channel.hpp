#ifndef CHANNEL_HPP
# define CHANNEL_HPP
# include "header.hpp"
# include "ClientConnection.hpp"
# include "Client.hpp"
# include "Server.hpp"
# include "RPL.hpp"
# include "Message.hpp"

class ClientConnection;

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

		int		kickCmd( std::string & );
		int		inviteCmd( std::string &, bool, std::map<int, ClientConnection> &, int, std::vector<pollfd> & );
		int		topicCmd( std::string &, bool, std::map<int, ClientConnection> &, int, std::vector<pollfd> & );
		int		modeCmd( std::string & );
};

#endif
