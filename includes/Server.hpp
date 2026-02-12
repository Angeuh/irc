#ifndef SERVER_HPP
# define SERVER_HPP
# include "header.hpp"
# include "ClientConnection.hpp"
# include "Client.hpp"
# include "Channel.hpp"
# include "RPL.hpp"
# include "Message.hpp"
# define SERVERNAME "localhost"

class Channel;
class ClientConnection;
class Message;

class Server
{
    private:
        int								serverSocket;
        std::vector<pollfd>				fds;
        std::map<int, ClientConnection>	clients;
		std::map<std::string, Channel>	channels;
		std::string						password;
	
        int     acceptNewClient();
		void	callRecv( int, int );
		void	handleClientMessage( Message &, int );
		void	handleRegistration( Message &, int );

    public:
        Server();
        ~Server();
        Server( int, std::string );
        void run(); // to start the server, somehow ?

    class PollError : public std::exception {
            public:
                const char* what() const throw();
            };
};

void broadcastingMessage(std::map<int, ClientConnection> &clients,
                                const std::string &content,
								const std::string &command,
                                int fd,
                                std::vector<pollfd> &fds);

#endif
