#include "../includes/header.hpp"
# include "ClientConnection.hpp"
# include "Client.hpp"
# include "Channel.hpp"
# include "Server.hpp"
# include "RPL.hpp"
# include "Message.hpp"

bool	isSpace(char *str)
{
	int	i = 0;

	while (str[i]) {
		if (str[i] == ' ')
			return (true);
		i++;
	}
	return (false);
}

int checkArguments(int ac, char *av[], int &socket)
{
	if (ac != 3) {
		std::cerr << "Server connection requires a port number and password, only those two" << std::endl;
		return (FAILURE);
	}

	socket = std::atoi(av[1]);
	if (socket <= 0 || socket >= 65536) {
		std::cerr << "Port doesn't exist" << std::endl;
		return (FAILURE);
	}
	if (!av[2] || av[2][0] == '\0') {
		std::cerr << "Password empty" << std::endl;
		return (FAILURE);
	}
	if (isSpace(av[2])) {
		std::cerr << "Password can't contain spaces" << std::endl;
		return (FAILURE);
	}
	return (SUCCESS);
}

int main(int ac, char *av[])
{
	int socket;

	if (checkArguments(ac, av, socket) != 0)
		return (FAILURE);
	try {
		Server server(socket, av[2]);
		signal(SIGINT, Server::SignalHandler); 
		signal(SIGQUIT, Server::SignalHandler);
		server.run();
	}
	catch (const std::exception &e) {
		std::cerr << "Server failed to start: " << e.what() << std::endl;
		return (FAILURE);
	}
	return (SUCCESS);
}
