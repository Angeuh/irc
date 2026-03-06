#ifndef CLIENTCONNECTION_HPP
# define CLIENTCONNECTION_HPP
# include "header.hpp"
# include "Client.hpp"
# include "Channel.hpp"
# include "Server.hpp"
# include "RPL.hpp"
# include "Message.hpp"

class ClientConnection
{
    public:

		ClientConnection();
        ~ClientConnection();

		void	inviteUser(Channel &channel);
		bool	operator==( const ClientConnection& ) const;

        int 							fd;					// socket
        std::string 					username;			// who they are
		std::string 					realname;			// who they are
        std::string 					readBuffer;			// partial incoming data
        std::string						writeBuffer;		// queued outgoing data
        std::string 					name;				// real name used in irssi
        std::string 					host;				// hostname or IP address
        time_t							lastActivity;		// for timeout handling
        time_t						    lastPingSent;		    // time of last PING sent, for timeout handling
		bool							hasNick;
		bool							hasUser;
		bool							hasPass;
		bool							isRegistered;
        bool							waitingForPong;  
		std::map<std::string, Channel *>	activeChannels;
		std::map<std::string, Channel *>	invitedChannels;
		
};

#endif 
