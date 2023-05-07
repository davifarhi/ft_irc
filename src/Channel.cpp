#include "Channel.hpp"

Channel::Channel( const string& name ) : name(Channel::trim_channel_name(name))
{
	topic = "";
	password = "";
	invite_only = false;
	user_limit = default_user_limit();
	has_password = false;
	status_topic = 0;
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
	chan_ops.erase(&client);
	for (list<Client*>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		if (*it == &client)
			it = --clients.erase(it);
	}
	if (chan_ops.empty() && !clients.empty())
		chan_ops.insert(clients.front());
}

void Channel::send_topic_to_client( Client& client, IRCServer& server ) const
{
	if (topic.size())
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

bool Channel::user_is_invited( Client & client ) const
{
	if (invite_only && invited.find(&client) == invited.end())
		return false;
	return true;
}

bool Channel::is_there_space_for_newuser( void ) const
{
	if (clients.size() < user_limit + 1)
		return true;
	return false;
}

size_t Channel::default_user_limit( void ) const
{
	return clients.max_size();
}

bool operator<( const Channel& lhs, const Channel& rhs )
{
	return (lhs.name < rhs.name);
}

string Channel::trim_channel_name( const string& str )
{
	if (str[0] == '#')
		return str.substr(1);
	return str;
}

void Channel::change_topic_of_channel( const string str, Client& client )
{
	if (status_topic == 0)
		topic = this->name + " " + str;
	else
	{
		if (get_chan_ops( client ))
			topic = this->name + " " + str;
	}
}

bool Channel::get_chan_ops( Client& client )
{
	set<Client*>::iterator it = chan_ops.find(&client);
	if (it == chan_ops.end())
		return false;
	else
		return true;
}

void Channel::change_privilege_topic( int temp )
{
	status_topic = temp;
}

void Channel::add_new_chan_ops( Client& client )
{
	chan_ops.insert(&client);
}

void Channel::kick_user_of_chan_ops( Client& client )
{
	if (chan_ops.size() > 1)
		chan_ops.erase(&client);
}
