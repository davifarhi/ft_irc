#ifndef CHANNEL_HPP

#define CHANNEL_HPP

class Channel;

#include "IRCServer.hpp"

class Channel
{
	private:
		list<Client*> clients;
		string name;
		string topic;

		friend class IRCServer;
		friend class MessageParser;

	public:
		Channel( const string& name );

		bool client_is_in_channel( Client& client ) const;
		bool join_client( Client& client );
		void part_client( Client& client );

		void send_topic_to_client( Client& client, IRCServer& server ) const;
		void send_names_to_client( Client& client, IRCServer& server) const;

		friend bool operator<( const Channel& lhs, const Channel& rhs );

		static string trim_channel_name( const string& str );
};


#endif
