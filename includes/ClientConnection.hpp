#ifndef CLIENTCONNECTION_HPP
# define CLIENTCONNECTION_HPP
# include "header.hpp"
# include "Client.hpp"
# include "Channel.hpp"
# include "Server.hpp"
# include "RPL.hpp"
# include "Message.hpp"

class ClientConnection
{

    private:

    public:

		ClientConnection();
        ~ClientConnection();	
        int fd;                   // socket
        std::string username;     // who they are
        std::string readBuffer;   // partial incoming data
        std::string writeBuffer;  // queued outgoing data
        std::string name;         // real name used in irssi
        std::string host;         // hostname or IP address
        std::string currentChannel; // /join x (channel name)
        bool loggedIn;  
};

#endif 
