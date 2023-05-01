#include "IRCServer.hpp"

IRCServer::IRCServer( int port, string pswd ) : port(port), pswd(pswd), has_started(false)
{
	if (!port)
	{
		cerr << "Invalid port " << port << endl;
		return;
	}
	if (pswd.length() < 4)
	{
		cerr << "Invalid password " << pswd << ", it must have at least 4 characters\n";
		return;
	}
	if (create_socket())
		return;
	cout << "Starting IRC server at port " << port << " with password: " << pswd << endl;
	this->has_started = true;
}

IRCServer::~IRCServer( void )
{
	shutdown( sockfd, SHUT_RDWR );
}

bool IRCServer::get_has_started( void ) const
{
	return this->has_started;
}

bool IRCServer::create_socket( void )
{
	sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	if (sockfd == -1)
	{
		cerr << "IRCServer::create_socket: socket() error\n";
		return true;
	}

	//if needed setsockopt here

	{
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_port = htons(this->port);

		if (bind( sockfd, (struct sockaddr*) &addr, sizeof(addr) ) == -1)
		{
			cerr << "IRCServer::create_socket: bind() error: " << strerror(errno) << endl;
			return true;
		}
	}

	if (listen( sockfd, 128 ) == -1)
	{
		cerr << "IRCServer::create_socket: listen() error: " << strerror(errno) << endl;
		return true;
	}

	return false;
}

