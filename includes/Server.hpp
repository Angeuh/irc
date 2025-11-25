#ifndef SERVER_HPP
# define SERVER_HPP
# include <cstring>
# include <iostream>
# include <netinet/in.h>
# include <sys/socket.h>
# include <unistd.h>
# include <vector>
# include <poll.h>

class Server
{

    private:
        int serverSocket;
        std::vector<pollfd> fds;
        void acceptNewClient();
        void handleClientMessage(size_t index);

    public:
    Server();
        ~Server();
        Server(int port);
        void run(); // to start the server, somehow ?
};

#endif