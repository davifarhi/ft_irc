#include "IRCServer.hpp"

#ifndef CLIENT_H

#define CLIENT_H

class Client
{
	public:
		const int fd;
		const int port;
		const string hostname;

		Client( int fd );
		Client( int fd, int port, const string &hostname );
};

bool operator<( const Client& lhs, const Client& rhs );
std::ostream& operator<<( std::ostream& os, const Client& c );
bool operator==( const pollfd& lhs, const Client& rhs );

#endif /* end of include guard: CLIENT_H */
