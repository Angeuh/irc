#ifndef CHANNEL_HPP
# define CHANNEL_HPP
# include <cstring>
# include <iostream>
# include <netinet/in.h>
# include <sys/socket.h>
# include <unistd.h>
# include <vector>
# include <poll.h>
# include <set>
# include <sstream>
# include "ClientConnection.hpp"
# include "Client.hpp"
# include "Server.hpp"

class Channel
{
    private:
		std::string		name;
		std::string		topic;
        std::set<int>	users;
        std::set<int>	operators;

		std::string	rplNoTopic(const std::string &command);
		std::string	rplTopic(const std::string &command);

		std::string	errNeedMoreParams(const std::string &command);
		std::string	errNotOnChannel(const std::string &command);
		std::string	errChanOpPrivsNeeded(const std::string &command);
		std::string	errNoChanModes(const std::string &command);

    public:
        Channel();
        ~Channel();
        Channel( const std::string &, int );

		void	insertUser(int);
		bool	isOperator( int );
		bool	isOnChannel( int );

		int		kickCmd( std::string & );
		int		inviteCmd( std::string & );
		int		topicCmd( std::string &, Channel &, std::map<int, ClientConnection> &, int, std::vector<pollfd> & );
		int		modeCmd( std::string & );
};

#endif
