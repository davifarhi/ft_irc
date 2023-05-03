#include "Channel.hpp"

Channel::Channel( const string& name ) : name(Channel::trim_channel_name(name))
{
}

bool operator<( const Channel& lhs, const Channel& rhs )
{
	return (lhs.name < rhs.name);
}

bool Channel::join_client( Client& client )
{
	if (std::find( clients.begin(), clients.end(), &client ) != clients.end())
		return false;
	clients.push_back(&client);
	return true;
}
