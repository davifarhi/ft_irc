#include <iostream>
#include <string>
#include <cstdlib>

#include "IRCServer.hpp"

using std::string;
using std::cerr;
using std::cout;
using std::endl;

int main( int ac, char **av )
{
	if (ac != 3)
	{
		cerr << "Wrong arguments\n";
		return 1;
	}
	IRCServer serv(atoi(av[1]), av[2]);
	if (!serv.get_has_started())
	{
		cerr << "The server has failed to start\n";
		return 1;
	}
}
