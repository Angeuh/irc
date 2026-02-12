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

class Server
{
    private:
        int serverSocket;
        int epfd; // epoll fd

        std::map<int, ClientConnection> clients;
        std::map<std::string, Channel> channels;

        int acceptNewClient();
        int handleClientMessage(int fd);
        void broadcastingMessage(std::map<int, ClientConnection> &clients,
                                const std::string &content,
								const std::string &command,
                                int fd);
        int connectionIrssi(std::map<int, ClientConnection> &clients,
                            std::string &msg, int fd);
        int joinChannel(std::map<int, ClientConnection> &clients,
                            std::string &msg, int fd,
                            std::map<std::string, Channel> &channels);

        void addToEpoll(int fd, uint32_t events);
        void modifyEpoll(int fd, uint32_t events);
        void removeFromEpoll(int fd);

    public:
        Server();
        ~Server();
        Server(int port);

        void run();

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
