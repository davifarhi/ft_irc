#ifndef IRCSERVER_H

#define IRCSERVER_H

#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <netdb.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <vector>
#include <set>

#include "Client.hpp"
#include "debug.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::set;

class IRCServer
{
	private:
		const int port;
		const string pswd;
		bool has_started;
		int sockfd;
		vector<pollfd> pfds;
		set<Client> clients;

	public:
		IRCServer( int port, string pswd );
		~IRCServer( void );

		void run( void );

		bool get_has_started( void ) const;

	private:
		bool create_socket( void );
		pollfd pfd_construct( int, short, short ) const;
		void client_connect( void );
		void client_disconnect( int fd );
};

#endif /* end of include guard: IRCSERVER_H */
