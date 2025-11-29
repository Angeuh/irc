#ifndef CLIENTCONNECTION_HPP
# define CLIENTCONNECTION_HPP
# include <iostream>
# include <string>

class ClientConnection
{

    private:

    public:
        int fd;                   // socket
        std::string username;     // who they are
        std::string readBuffer;   // partial incoming data
        std::string writeBuffer;  // queued outgoing data
        std::string name;         // real name used in irssi
        bool loggedIn = false;  
};

#endif 