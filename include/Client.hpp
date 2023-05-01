#ifndef CLIENT_H

#define CLIENT_H

#include <string>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;
using std::string;

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

#endif /* end of include guard: CLIENT_H */
