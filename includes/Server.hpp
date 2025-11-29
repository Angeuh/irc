#ifndef SERVER_HPP
# define SERVER_HPP
# include <cstring>
# include <iostream>
# include <netinet/in.h>
# include <sys/socket.h>
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
};

#endif
