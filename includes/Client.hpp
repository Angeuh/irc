#ifndef CLIENT_HPP
# define CLIENT_HPP
# include "header.hpp"

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

