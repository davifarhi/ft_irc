#ifndef IRCSERVER_H

#define IRCSERVER_H

#include <string>
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using std::string;

class IRCServer
{
	private:
		const int port;
		const string pswd;
		bool has_started;
	public:
		IRCServer( int port, string pswd );

		bool get_has_started( void ) const;
};

#endif /* end of include guard: IRCSERVER_H */
