#ifndef IRCSERVER_H

#define IRCSERVER_H

#include "debug.hpp"
#include "answer.hpp"

#include <string>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <netdb.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <vector>
#include <stack>
#include <set>
#include <map>
#include <list>
#include <algorithm>

#ifndef LINUX_OS
# include <fcntl.h>
#endif


#define SSTR( x ) static_cast< std::ostringstream & >( ( std::ostringstream() << std::dec << x ) ).str()

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::set;
using std::map;
using std::stack;
using std::list;

class Client;
class MessageParser;
class Channel;
class IRCServer;

#include "Channel.hpp"
#include "MessageParser.hpp"
#include "Client.hpp"

#define BUFFER_SIZE 1024

class IRCServer
{
	private:
		MessageParser msg_parser;
		const int port;
		const string pswd;
		bool has_started;
		bool running;
		int sockfd;
		vector<pollfd> pfds;
		set<Client> clients;
		set<Channel> channels;
		stack<int> fds_to_disconnect;
		map<int,string> receive_buf;

		friend class MessageParser;

	public:
		IRCServer( int port, string pswd );
		~IRCServer( void );

		void run( void );

		void send_message_to_client( const Client& client, string msg );

		const string& get_pswd( void ) const;
		bool get_has_started( void ) const;
		void stop( void );

	private:
		bool create_socket( void );
		pollfd pfd_construct( int fd, short events, short revents ) const;
		void client_connect( void );
		vector<pollfd>::iterator client_disconnect( int fd );
		void receive_message( Client& client );

		bool get_user( const string& name, Client** res );

		Channel& get_channel( const string& name );
		bool get_channel( const string& name, Channel** res );
		void channel_remove_user( Client& client );
		void channel_add_user( Client& client, Channel& channel );
		void print_channels( void );
		void delete_channel_if_empty( Channel* channel );

		void send_msg_to_all( string msg, Client* exception = 0 );
};

#endif /* end of include guard: IRCSERVER_H */
