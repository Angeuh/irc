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
        int epfd; // epoll fd
        std::map<int, ClientConnection>	clients;
		std::map<std::string, Channel>	channels;
        std::string						password;

	
        int     acceptNewClient();
        void	callRecv( int, int );
		void	handleClientMessage( Message &, int );
        //int handleClientMessage(int fd);
		void	handleRegistration( Message &, int );

        int connectionIrssi(std::map<int, ClientConnection> &clients,
                            std::string &msg, int fd);
        int joinChannel(std::map<int, ClientConnection> &clients,
                            std::string &msg, int fd,
                            std::map<std::string, Channel> &channels);

    public:
        static bool             Signal;
        Server();
        ~Server();
        Server( int, std::string );
        void run(); // to start the server, somehow ?
        void addToEpoll(int fd, uint32_t events);
        void modifyEpoll(int fd, uint32_t events);
        void removeFromEpoll(int fd);
        void broadcastingMessage(std::map<int, ClientConnection> &clients,
                                const std::string &content,
								const std::string &command,
                                int fd);

        static void SignalHandler(int signum);
        class PollError : public std::exception {
        public:
            const char* what() const throw();
        };
};

void broadcastingMessage(std::map<int, ClientConnection> &clients,
                                const std::string &content,
								const std::string &command,
                                int fd);

#endif
