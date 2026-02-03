#ifndef SERVER_HPP
# define SERVER_HPP
# include <cstring>
# include <iostream>
# include <cstdlib>
# include <netinet/in.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <vector>
# include <poll.h>
# include <map>
# include <sstream>
# include "ClientConnection.hpp"
# include "Client.hpp"
# include "Channel.hpp"

class Channel;

class Server
{

    private:
        int serverSocket;
        std::vector<pollfd> fds;
        std::map<int, ClientConnection> clients;
		std::map<std::string, Channel> channels;
        int     acceptNewClient();
        int     handleClientMessage(size_t index);

    public:
        Server();
        ~Server();
        Server(int port);
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
