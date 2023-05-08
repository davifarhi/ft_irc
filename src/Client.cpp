#include "Client.hpp"

unsigned int Client::guest_n = 0;

Client::Client( int fd ) : fd(fd), port(int()), hostname(string()) {}

Client::Client( int fd, int port, const string &hostname ) : fd(fd), port(port), hostname(hostname)
{
	on_cap_negotiation = false;
	nickname = SSTR( "Guest" << ++guest_n);
	authenticated = false;
	registered = false;
}

bool Client::is_in_channel( Channel& channel ) const
{
	return (channels.find(&channel) != channels.end());
}

void Client::join_channel( Channel& channel )
{
	channels.insert(&channel);
}

void Client::part_channel( Channel& channel )
{
	channels.erase(&channel);
}

void Client::leave_all_channels( void )
{
	for (set<Channel*>::iterator it = channels.begin(); it != channels.end(); it++)
		(*it)->part_client(const_cast<Client&>(*this));
	channels.clear();
}

bool Client::is_registration_done( void )
{
	return authenticated && registered;
}

bool operator<( const Client& lhs, const Client& rhs ) { return (lhs.fd < rhs.fd); }

std::ostream& operator<<( std::ostream& os, const Client& c )
{
	os << "(" << c.fd << ", " << c.port << ", " << c.hostname << ", " << c.nickname << ")";
	return os;
}

bool operator==( const pollfd& lhs, const Client& rhs ) { return lhs.fd == rhs.fd; }
