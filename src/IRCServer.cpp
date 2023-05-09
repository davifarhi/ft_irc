#include "IRCServer.hpp"

IRCServer::IRCServer( int port, string pswd ) : msg_parser(*this), port(port), pswd(pswd), has_started(false)
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
	cout << "Ready to start IRC server at port " << port << " with password: " << pswd << endl;
	this->has_started = true;
	this->running = true;
}

IRCServer::~IRCServer( void )
{
	while (pfds.size() > 1)
	{
		send_message_to_client( *clients.find(pfds.back().fd), "ft_irc QUIT :Server stopped" );
		client_disconnect(pfds.back().fd);
	}
	shutdown( sockfd, SHUT_RDWR );
	close(sockfd);
}

void IRCServer::run( void )
{
	pfds.push_back(pfd_construct(sockfd, POLLIN | POLLPRI, 0));
	while (running)
	{
		while (!fds_to_disconnect.empty())
		{
			client_disconnect(fds_to_disconnect.top());
			fds_to_disconnect.pop();
		}

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
			if (it->revents &POLLHUP)
			{
				it = --client_disconnect(it->fd);
				continue;
			}
			if (it->revents &POLLIN)
			{
				receive_message(const_cast<Client&>(*clients.find(it->fd)));
			}
		}
	}
}

const string& IRCServer::get_pswd( void ) const
{ return pswd; }

bool IRCServer::get_has_started( void ) const
{
	return this->has_started;
}

void IRCServer::stop( void )
{
	this->running = false;
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
	set<Client>::iterator client_it = clients.find(fd);
	if (client_it != clients.end())
	{
		//bad for performance, iterating throught set
		for (set<Channel*>::iterator it = client_it->channels.begin(); it != client_it->channels.end(); it++)
		{
			(*it)->part_client(const_cast<Client&>(*client_it));
		}
		if (DEBUG_CLIENT_CONNECTION)
			cout << "client disconnected: " << *client_it << endl;
		clients.erase(client_it);
	}
	vector<pollfd>::iterator client = std::find( pfds.begin(), pfds.end(), Client(fd) );
	if (client == pfds.end())
	{
		cerr << "IRCServer::client_disconnect: error client missing in pollfd vector\n";
	}
	if (close(fd) == -1)
	{
		cerr << "IRCServer::client_disconnect: close() error " << strerror(errno) << endl;
	}
	receive_buf.erase(fd);
	return pfds.erase(client);
}

void IRCServer::receive_message( Client& client )
{
	char buf[BUFFER_SIZE];
	string& line = receive_buf[client.fd];
	while (true)
	{
#ifdef LINUX_OS
		ssize_t n = recv( client.fd, buf, BUFFER_SIZE, MSG_DONTWAIT );
#else
		ssize_t n = recv( client.fd, buf, BUFFER_SIZE, 0 );
#endif
		if (n <= 0)
			break;
		for (int i = 0; i < n; i++)
		{
			if (buf [i] == '\n')
			{
				// cleaning carriage returns in received text
				if (line.size() > 1)
				{
					if (line[line.size() - 1] == '\r')
						line.erase(line.size() - 1);
					if (DEBUG_PRINT_RECEIVED_MESSAGE)
						cout << "RX " << client << ": " << line << endl;
					msg_parser.parse( client, line );
				}
				line.clear();
			}
			else
			{
				line.push_back(buf[i]);
			}
		}
	}
}

bool IRCServer::get_user( const string& name, Client** res )
{
	// bad for performance, iterating through set
	for (set<Client>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		if (it->nickname == name)
		{
			if (res)
				*res = const_cast<Client*>(&(*it));
			return true;
		}
	}
	return false;
}

void IRCServer::send_message_to_client( const Client& client, string msg )
{
	if (msg[msg.size() - 1] == '\n')
		msg.erase(msg.size() - 1);
	msg += "\r\n";
	if (send( client.fd, msg.c_str(), msg.size(), 0) == -1)
	{
		cerr << "IRCServer::send_message_to_client: send() error: " << strerror(errno) << endl;
		return;
	}
	if (DEBUG_PRINT_SENT_MESSAGE)
	{
		cout << "TX " << client << ": " << msg;
	}
}

Channel& IRCServer::get_channel( const string& name )
{
	return const_cast<Channel&>(*channels.insert(name).first);
}

bool IRCServer::get_channel( const string& name, Channel** res )
{
	set<Channel>::iterator it = channels.find(name);
	if (it == channels.end())
		return false;
	*res = const_cast<Channel*>(&(*it));
	return true;
}

void IRCServer::print_channels( void )
{
	int i = 0;
	for (set<Channel>::iterator it = channels.begin(); it != channels.end(); it++)
	{
		cout << i++ << " " << it->name << " userN " << it->clients.size() << endl;
	}
}

void IRCServer::delete_channel_if_empty( Channel* channel )
{
	if (channel->clients.size()) return;
	channels.erase(*channel);
}

void IRCServer::send_msg_to_all( string msg, Client* exception )
{
	//performance problem, iterating through set
	for (set<Client>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		if (&(*it) != exception)
			send_message_to_client( *it, msg );
	}
}
