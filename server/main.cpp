// C++ program to show the example of server application in
// socket programming

#include "../includes/Server.hpp"


int main() {
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
}