#include "server.hpp"
#include <iostream>
#include <string>

int	main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cout << "argument non valable/Usage: ./ircserv <port> <password>\n";
		return (1);
	}
	
	Server server(av[1], av[2]);

	server.start();
	return(0);
}
