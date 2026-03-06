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
# include "utils.hpp"
# define SERVERNAME "localhost"

class Channel;
class ClientConnection;
class Message;

// irssi -n nickname -c localhost -p 8080 -w 123
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
        std::string generateFreeNick(const std::string &base);

        void	joinCmd( Message&, ClientConnection& );
		void	joinOneChannel( ClientConnection&, std::string&, std::string&, bool );
		void	welcomeToChannel( Channel&, ClientConnection& );
		void	kickCmd( Message&, ClientConnection& );
		void	inviteCmd( Message&, ClientConnection& );
		void	topicCmd( Message&, ClientConnection& );
		void	modeCmd( Message& , ClientConnection& );
		void	applyMode( char, char, std::string, ClientConnection&, Channel&, std::string &, std::string &, bool );
		void	quitAllChannels( ClientConnection& );
        void    partCmd(Message &, ClientConnection &);
        void    quitChannel(ClientConnection &, const std::string &, const std::string &, const std::string &);
		void	whoCmd( Message &, ClientConnection & );
        void    privmsgCmd(Message &, ClientConnection &);
        void    pingClients();
		void	handleRegistration( Message &, ClientConnection & );
		void	handleClientMessage( Message &, ClientConnection & );
		void 	broadcastingMessage( ClientConnection&, const std::string&, const std::string&, Channel &);
		void	sendMessage( ClientConnection&, const std::string& );


    public:
		Server();
		~Server();
		Server( int, std::string );
		
		static bool             Signal;
        time_t lastPingTime;
        static const int PING_INTERVAL = 300; //seconds
        static const int PONG_TIMEOUT = 60;
		
		void	run(); // to start the server, somehow ?
        void	addToEpoll(int fd, uint32_t events);	// pass to private ?
        void	modifyEpoll(int fd, uint32_t events);	// pass to private ?
        void	removeFromEpoll(int fd);				// pass to private ?

        static void SignalHandler(int signum);
        class PollError : public std::exception {
        public:
            const char* what() const throw();
        };
};

#endif
