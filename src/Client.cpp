#include "Client.hpp"

Client::Client( int fd ) : fd(fd), port(int()), hostname(string()) {}

Client::Client( int fd, int port, const string &hostname ) : fd(fd), port(port), hostname(hostname) {}

bool operator<( const Client& lhs, const Client& rhs ) { return (lhs.fd < rhs.fd); }

std::ostream& operator<<( std::ostream& os, const Client& c )
{
	//os << "fd: " << c.fd << ", port: " << c.port << ", hostname: " << c.hostname;
	os << "(" << c.fd << ", " << c.port << ", " << c.hostname << ")";
	return os;
}
