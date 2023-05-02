#include "Client.hpp"

Client::Client( int fd ) : fd(fd), port(int()), hostname(string()) {}

Client::Client( int fd, int port, const string &hostname ) : fd(fd), port(port), hostname(hostname)
{
	on_cap_negotiation = false;
	authenticated = false;
}

bool operator<( const Client& lhs, const Client& rhs ) { return (lhs.fd < rhs.fd); }

std::ostream& operator<<( std::ostream& os, const Client& c )
{
	os << "(" << c.fd << ", " << c.port << ", " << c.hostname << ")";
	return os;
}

bool operator==( const pollfd& lhs, const Client& rhs ) { return lhs.fd == rhs.fd; }
