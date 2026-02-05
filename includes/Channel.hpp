#ifndef CHANNEL_HPP
# define CHANNEL_HPP
# include "header.hpp"

class ClientConnection;

class Channel
{
    private:
		std::string		name;
		std::string		topic;
        std::set<int>	users;
        std::set<int>	operators;

    public:
        Channel( void );
        ~Channel( void );
        Channel( const std::string &, int );

		void	insertUser(int);
		bool	isOperator( int );
		bool	isOnChannel( int );

		int		kickCmd( std::string & );
		int		inviteCmd( std::string & );
		int		topicCmd( std::string &, bool, Channel &, std::map<int, ClientConnection> &, int, std::vector<pollfd> & );
		int		modeCmd( std::string & );
};

#endif
