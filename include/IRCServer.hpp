#ifndef IRCSERVER_H

#define IRCSERVER_H

#include "debug.hpp"

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
#include <algorithm>

#ifndef LINUX_OS
# include <fcntl.h>
#endif

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::set;

class Client;
class MessageParser;
class IRCServer;

#include "MessageParser.hpp"
#include "Client.hpp"

#define BUFFER_SIZE 1024

//TODO password
//TODO channels
//TODO receive and send messages

class IRCServer
{
	private:
		MessageParser msg_parser;
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
		pollfd pfd_construct( int fd, short events, short revents ) const;
		void client_connect( void );
		vector<pollfd>::iterator client_disconnect( int fd );
		void receive_message( Client& client );

		friend class MessageParser;
};

#endif /* end of include guard: IRCSERVER_H */
