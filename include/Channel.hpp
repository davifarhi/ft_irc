#ifndef CHANNEL_HPP

#define CHANNEL_HPP

class Channel;

#include "IRCServer.hpp"

class Channel
{
	private:
		list<Client*> clients;
		set<Client*> chan_ops;
//TODO		set<Client*> invited;
		string name;
		string topic;

		bool has_password;
		string password;

		friend class IRCServer;
		friend class MessageParser;

	public:
		Channel( const string& name );

		bool client_is_in_channel( Client& client ) const;
		bool join_client( Client& client );
		void part_client( Client& client );

		void send_topic_to_client( Client& client, IRCServer& server ) const;
		void send_names_to_client( Client& client, IRCServer& server) const;

		void send_msg_to_all( string msg, IRCServer& server, Client* exception = 0 );

		bool try_password( const string& pswd ) const;

		friend bool operator<( const Channel& lhs, const Channel& rhs );

		static string trim_channel_name( const string& str );
};


#endif
