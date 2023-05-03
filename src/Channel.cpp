#include "Channel.hpp"

Channel::Channel( const string& name ) : name(Channel::trim_channel_name(name))
{
	topic = this->name + " channel has no topic set";
}

bool operator<( const Channel& lhs, const Channel& rhs )
{
	return (lhs.name < rhs.name);
}

bool Channel::client_is_in_channel( Client& client ) const
{
	return client.is_in_channel( const_cast<Channel&>(*this) );
}

bool Channel::join_client( Client& client )
{
	if (client_is_in_channel(client))
		return false;
	clients.push_back(&client);
	return true;
}

void Channel::part_client( Client& client )
{
	for (list<Client*>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		if (*it == &client)
			it = --clients.erase(it);
	}
}

void Channel::send_topic_to_client( Client& client, IRCServer& server ) const
{
	server.send_message_to_client( client, RPL_TOPIC( client.nickname, name, topic ) );
}

string Channel::trim_channel_name( const string& str )
{
	if (str[0] == '#')
		return str.substr(1);
	return str;
}
