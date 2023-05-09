#include <iostream>
#include <string>
#include <cstdlib>
#include <csignal>

#include "IRCServer.hpp"

using std::string;
using std::cerr;
using std::cout;
using std::endl;

IRCServer* g_server = 0;

void irc_sigint_handler( int i )
{
	(void)i;
	cout << "SIGINT received, stopping irc server\n";
	g_server->stop();
}

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
	g_server = &serv;
	signal( SIGINT, &irc_sigint_handler );
	serv.run();
	//system("leaks ircserv");
	return 0;
}
