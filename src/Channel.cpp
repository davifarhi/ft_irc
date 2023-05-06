#include "Channel.hpp"

Channel::Channel( const string& name ) : name(Channel::trim_channel_name(name))
{
	topic = this->name + " channel has no topic set";
	password = "";
	has_password = false;
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
	if (!clients.size())
		chan_ops.insert(&client);
	clients.push_back(&client);
	return true;
}

void Channel::part_client( Client& client )
{
	//TODO remove from chan_ops
	//if last chan_ops, first clients is chan_ops
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

void Channel::send_names_to_client( Client& client, IRCServer& server) const
{
	for (list<Client*>::const_iterator it = clients.begin(); it != clients.end(); it++)
		server.send_message_to_client( client, RPL_NAMREPLY( client.nickname, name, (*it)->nickname ) );
	server.send_message_to_client( client, RPL_ENDOFNAMES( client.nickname, name ) );
}

void Channel::send_msg_to_all( string msg, IRCServer& server, Client* exception )
{
	for (list<Client*>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		if (*it != exception)
			server.send_message_to_client( **it, msg );
	}
}

bool Channel::try_password( const string& pswd ) const
{
	if (!has_password)
		return true;
	return pswd == password;
}

string Channel::trim_channel_name( const string& str )
{
	if (str[0] == '#')
		return str.substr(1);
	return str;
}

void Channel::change_topic_of_channel( const string str )
{
	topic = this->name + " " + str;
}

bool Channel::get_chan_ops( client& client)
{
	set<Client*>::iterator it = client.find(name);
	if (it == client.end())
		return false;
	else
		return true;
}
