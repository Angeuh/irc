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

class Server
{

    private:
        int serverSocket;
        std::vector<pollfd> fds;
        std::map<int, ClientConnection> clients;
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

#endif
