#include "../includes/Server.hpp"

bool Server::Signal = false; 

Server::Server() {}

Server::~Server()
{
	for (std::map<int, ClientConnection>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        close(it->second.fd);
        removeFromEpoll(it->second.fd);
    }
    clients.clear();
    close(serverSocket);
    close(epfd);
    
    std::cout << "Server Closed" << std::endl;
}

Server::Server(int port, std::string pass) :
	password(pass)
{
	Signal = false;
	lastPingTime = time(NULL);
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        throw std::runtime_error("Socket creation failed");
    }
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(sockaddr_in));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    epfd = epoll_create(1024);
    if (epfd < 0)
	{
		close(serverSocket); 
        throw std::runtime_error("epoll_create failed");
	}
    if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
		close(epfd);
        close(serverSocket);
        throw std::runtime_error("Bind failed");
    }
    if (listen(serverSocket, 128) < 0)
    {
		close(epfd);
        throw std::runtime_error("Listen failed");
    }
    addToEpoll(serverSocket, EPOLLIN);
    std::cout << "Server listening on port " << port << std::endl;
}

void Server::SignalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "Signal Received!" << std::endl;
	Signal = true;
}

char const *Server::PollError::what() const throw()
{
    return "Poll failed";
}

//send content to all users in channel (except for user if PRIVMSG)
void Server::broadcastingMessage( ClientConnection &user, const std::string &command, const std::string &content, Channel &channel)
{
	bool	skipUser = (command == "PRIVMSG");
	for (std::vector<ClientConnection *>::iterator it = channel.users.begin(); it != channel.users.end(); ++it)
    {
        ClientConnection *client = *it;
        if (!skipUser || client->username != user.username) {
            std::cout << "[BROADCAST] send to " << client->username << std::endl;
            sendMessage(*client, content);
        }
    }
}

//send content to client
void Server::sendMessage( ClientConnection &client, const std::string &content )
{
    client.writeBuffer += content;
    this->modifyEpoll(client.fd, EPOLLIN | EPOLLOUT);
    std::cout << "[SENDMESSAGE] " << content << std::endl;
}

int Server::acceptNewClient()
{
    int clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket < 0)
    {
        std::cerr << "Accept failed" << std::endl;
        return -1;
    }
    addToEpoll(clientSocket, EPOLLIN);
    clients.insert(std::make_pair(clientSocket, ClientConnection()));
    clients[clientSocket].fd = clientSocket;
	clients[clientSocket].lastActivity = time(NULL);
    std::cout << "New client connected: fd " << clientSocket << std::endl;
    return clientSocket;
}

void Server::callRecv(int fd)
{
	char	buffer[4096];
    int 	bytesReceived = recv(fd, buffer, sizeof(buffer), 0);
	size_t	pos;

	if (bytesReceived <= 0)
	{
		removeFromEpoll(fd);
		close(fd);
		clients.erase(fd);
		std::cout << "Client " << fd << " disconnected" << std::endl;
		return ;
	}
	this->clients[fd].readBuffer += std::string(buffer, bytesReceived);
	pos = this->clients[fd].readBuffer.find("\r\n");
	while (pos != std::string::npos)
	{
		std::string	line = this->clients[fd].readBuffer.substr(0, pos + 2);
		if (line.length() > 512) {
			std::cout << "Message too long" << std::endl;
			return ;
		}
		this->clients[fd].readBuffer.erase(0, pos + 2);
		try {
			Message	msg(line);
			std::cout << msg << std::endl;
			if (this->clients[fd].isRegistered == true)
				handleClientMessage(msg, this->clients[fd]);
			else 
				handleRegistration(msg, this->clients[fd]);	
		} catch(const Message::ParsingError& e) {
			std::cerr << e.what() << std::endl;
			return ;
		}
		pos = this->clients[fd].readBuffer.find("\r\n");
	}
}

void Server::pingClients()
{
    time_t now = time(NULL);
    if (now - lastPingTime < PING_INTERVAL)
        return;

    std::map<int, ClientConnection>::iterator it = clients.begin();
	while (it != clients.end())
	{
		ClientConnection &client = it->second;
		if (client.isRegistered)
		{
			if (now - client.lastActivity >= PING_INTERVAL)
			{
				if (client.waitingForPong && now - client.lastPingSent >= PONG_TIMEOUT)
				{
					std::cout << "[PING TIMEOUT] Disconnecting " << client.username << std::endl;
					removeFromEpoll(client.fd);
					close(client.fd);
					std::map<int, ClientConnection>::iterator toErase = it;
					++it;
					clients.erase(toErase);
					continue;
				}
				std::string servername = SERVERNAME;
				std::string pingMsg = ":" + servername + " PING :" + servername + "\r\n";
				sendMessage(client, pingMsg);
				client.lastPingSent = now;
				client.waitingForPong = true;
				std::cout << "[PING] Sent to " << client.username << std::endl;
			}
		}
		++it;
	}
	lastPingTime = now;
}

void Server::run()
{
    const int MAX_EVENTS = 65335;
	std::vector<struct epoll_event> events(MAX_EVENTS);

    while (!Signal) {
        int n = epoll_wait(epfd, events.data(), events.size(), 1000);

        if (n < 0) {
            if (errno == EINTR)
                return;
            throw PollError();
        }
        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;
            if (fd == serverSocket) {
                acceptNewClient();
                continue;
            }
            if (events[i].events & EPOLLIN) {
                int fd_epoll = events[i].data.fd;
                callRecv(fd_epoll);
				std::map<int, ClientConnection>::iterator it = clients.find(fd_epoll);
				if (it != clients.end())
					it->second.lastActivity = time(NULL); 
            }
            if (events[i].events & EPOLLOUT) {
                ClientConnection &client = clients[fd];

                if (!client.writeBuffer.empty()) {
                    int sent = send(fd, client.writeBuffer.c_str(), client.writeBuffer.size(), 0);
                    if (sent > 0)
                        client.writeBuffer.erase(0, sent);
					else if (sent == -1) {
						std::cerr << "Send failed on fd " << fd << " errno: " << errno << std::endl;
						removeFromEpoll(fd);
						close(fd);
						clients.erase(fd);
						return;
					}
                    if (client.writeBuffer.empty())
                        this->modifyEpoll(fd, EPOLLIN);
                }
            }
        }
		pingClients();
    }
}

