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
	cout << "Starting IRC server at port " << port << " with password: " << pswd << endl;
	this->has_started = true;
}

bool IRCServer::get_has_started( void ) const
{
	return this->has_started;
}
