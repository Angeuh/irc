#ifndef SERVER_HPP
# define SERVER_HPP
# include "header.hpp"
# include "ClientConnection.hpp"
# include "Client.hpp"
# include "Channel.hpp"
# include "RPL.hpp"
# include "Message.hpp"
# include "errno.h"
# include "signal.h"
# define SERVERNAME "localhost"

class Channel;
class ClientConnection;
class Message;

class Server
{
    private:
        int								serverSocket;
        int 							epfd; // epoll fd
        std::map<int, ClientConnection>	clients;
		std::map<std::string, Channel>	channels;
        std::string						password;

	
        int     acceptNewClient();
        void	callRecv( int );
		void	handleClientMessage( Message&, ClientConnection& );
		void	handleRegistration( Message&, ClientConnection& );

        void	joinCmd( Message&, ClientConnection& );
		void	joinOneChannel( ClientConnection &, std::string &, std::string &, bool );
		void	welcomeToChannel( Channel &, ClientConnection & );
		void	kickCmd( Message&, ClientConnection& );
		void	inviteCmd( Message&, ClientConnection& );
		void	topicCmd( Message&, ClientConnection& );
		void	modeCmd( Message& , ClientConnection& );

    public:
		Server();
		~Server();
		Server( int, std::string );
		
		static bool             Signal;
		
		void	run(); // to start the server, somehow ?
        void	addToEpoll(int fd, uint32_t events);	// pass to private ?
        void	modifyEpoll(int fd, uint32_t events);	// pass to private ?
        void	removeFromEpoll(int fd);				// pass to private ?
		void 	broadcastingMessage( ClientConnection &, const std::string &, const std::string & );
		void	sendMessage( ClientConnection &, const std::string & );

        static void SignalHandler(int signum);
        class PollError : public std::exception {
        public:
            const char* what() const throw();
        };
};

#endif
