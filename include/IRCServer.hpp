#ifndef IRCSERVER_H

#define IRCSERVER_H

#include <string>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
using std::cout;
using std::cerr;
using std::endl;
using std::string;

class IRCServer
{
	private:
		const int port;
		const string pswd;
		bool has_started;
		int sockfd;
	public:
		IRCServer( int port, string pswd );
		~IRCServer( void );

		bool get_has_started( void ) const;

	private:
		bool create_socket( void );
};

#endif /* end of include guard: IRCSERVER_H */
