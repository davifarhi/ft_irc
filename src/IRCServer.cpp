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

void IRCServer::run( void )
{
	bool running = true;
	pfds.push_back(pfd_construct(sockfd, POLLIN | POLLPRI, 0));
	while (running)
	{
		int p;
		if ((p = poll( &(*pfds.begin()), pfds.size(), 0 )) == -1)
			cerr << "IRCServer::run: poll() error: " << strerror(errno) << endl;
		if (!p)
			continue;

		if (pfds.begin()->revents &POLLIN)
		{
			client_connect();
		}
		for (vector<pollfd>::iterator it = ++pfds.begin(); it != pfds.end(); it++)
		{
			if (it->revents &POLLIN)
			{
				receive_message(it->fd);
			}
			if (it->revents &POLLHUP)
			{
				it = --client_disconnect(it->fd);
			}
		}
	}
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
	{
		int opt = 1;
		if (setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt) )
				&& setsockopt( sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt) ))
		{
			cerr << "IRCServer::create_socket: setsockopt() error: " << strerror(errno) << endl;
			return true;
		}
	}
	{
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_port = htons(this->port);

		if (bind( sockfd, (sockaddr*) &addr, sizeof(addr) ) == -1)
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

pollfd IRCServer::pfd_construct( int fd, short events, short revents ) const
{
	pollfd pfd = {fd, events, revents};
	return pfd;
}

bool operator==( const pollfd& lhs, const Client& rhs ) { return lhs.fd == rhs.fd; }

void IRCServer::client_connect( void )
{
	sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	pfds.push_back(pfd_construct( accept( sockfd, (sockaddr *) &addr, &addrlen ), POLLIN | POLLPRI, 0 ));
	if (pfds.back().fd == -1)
	{
		pfds.pop_back();
		cerr << "IRCServer::client_connect: accept() error: " << strerror(errno) << endl;
		return;
	}

	{
		char hostname[NI_MAXHOST];
		int gai_error;
		if ((gai_error = getnameinfo( (sockaddr *) &addr, sizeof(addr), hostname, NI_MAXHOST, NULL, 0, NI_NUMERICSERV )))
		{
			pfds.pop_back();
			cerr << "IRCServer::client_connect: getnameinfo() error: " << gai_strerror(gai_error) << endl;
			return;
		}
#ifndef LINUX_OS
		if (fcntl( pfds.back().fd, F_SETFL, O_NONBLOCK ) == -1)
			cerr << "IRCServer::client_connect: fcntl() error: " << strerror(errno) << endl;
#endif
		set<Client>::iterator inserted = clients.insert(Client( pfds.back().fd, ntohs(addr.sin_port), hostname )).first;
		if (DEBUG_CLIENT_CONNECTION)
			cout << "client connected: " << *inserted << endl;
	}
}

vector<pollfd>::iterator IRCServer::client_disconnect( int fd )
{
	{
		set<Client>::iterator client = clients.find(fd);
		if (DEBUG_CLIENT_CONNECTION)
			cout << "client disconnected: " << *client << endl;
		clients.erase(client);
	}
	vector<pollfd>::iterator client = std::find( pfds.begin(), pfds.end(), Client(fd) );
	if (client == pfds.end())
	{
		cerr << "IRCServer::client_disconnect: error client missing in pollfd vector\n";
	}
	return pfds.erase(client);
}

void IRCServer::receive_message( int fd )
{
	char buf[BUFFER_SIZE];
	string line;
	while (true)
	{
#ifdef LINUX_OS
		ssize_t n = recv( fd, buf, BUFFER_SIZE, MSG_DONTWAIT );
#else
		ssize_t n = recv( fd, buf, BUFFER_SIZE, 0 );
#endif
		if (n <= 0)
			break;
		for (int i = 0; i < n; i++)
		{
			if (buf[i] == '\n')
			{
				cout << "message received from " << *clients.find(fd) << ": " << line << endl;
				line.clear();
			}
			else
			{
				line.push_back(buf[i]);
			}
		}
	}
}
