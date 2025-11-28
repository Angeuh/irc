#ifndef CLIENT_HPP
# define CLIENT_HPP
# include <cstring>
# include <iostream>
# include <netinet/in.h>
# include <sys/socket.h>
# include <unistd.h>
# include <vector>
# include <poll.h>


class Client
{

    private:
        int sock;


    public:
        Client();
        ~Client();
        Client(const std::string& host, int port);
        int sendMessage(const std::string &msg);
        void run(); // optional: read from server
};
#endif

