// C++ program to show the example of server application in
// socket programming

#include "../includes/header.hpp"

/* int main() {
    // Create listening socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 5);

    std::cout << "Server listening on port 8080\n";

    // Array of pollfds ( vector for dynamic size, we dont know how many clients will connect )
    std::vector<pollfd> fds;

    // Add listening socket to poll list
    pollfd listenPoll{};
    listenPoll.fd = serverSocket;
    listenPoll.events = POLLIN;
    fds.push_back(listenPoll);

    char buffer[1024];

    while (true) 
    {
        // events on socket listening
        int ret = poll(fds.data(), fds.size(), -1);

        for (size_t i = 0; i < fds.size(); i++) 
        {

            // new client is connecting
            if (fds[i].fd == serverSocket && (fds[i].revents & POLLIN))
            {
                int client = accept(serverSocket, nullptr, nullptr);
                std::cout << "New client: " << client << "\n";

                pollfd c{};
                c.fd = client;
                c.events = POLLIN;
                fds.push_back(c);
            }

            // A client sent data ( means event on socket )
            else if (fds[i].revents & POLLIN) 
            {
                memset(buffer, 0, sizeof(buffer));
                int bytes = recv(fds[i].fd, buffer, sizeof(buffer), 0);

                if (bytes <= 0) {
                    // Client disconnected ( delete cleanly from poll list )
                    std::cout << "Client " << fds[i].fd << " disconnected.\n";
                    close(fds[i].fd);
                    fds.erase(fds.begin() + i);
                    i--;
                    continue;
                }

                std::cout << "Client " << fds[i].fd << ": " << buffer << std::endl;
            }
        }
    }

    close(serverSocket);
    return 0;
} */

int checkArguments(int ac, char *av[], int socket)
{
    if (ac != 3)
    {
        std::cerr << "Server connection requires a port number and password, only those two" << std::endl;
        return 1;
    }
    if (socket <= 0 || socket >= 65536)
    {
        std::cerr << "Port doesn't exist" << std::endl;
        return 1;
    }
    if (!av[2] || strcmp(av[2], "123") != 0)
    {
        std::cerr << "Wrong password" << std::endl;
        return 1;
    }
    return 0;
}

int main(int ac, char *av[])
{
    int socket = std::atoi(av[1]);
    if (checkArguments(ac, av, socket) != 0)
        return 1;
    try
    {
        Server server(socket);
        server.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Server failed to start: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
