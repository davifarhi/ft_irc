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

		friend bool operator<( const Channel& lhs, const Channel& rhs );

		static string trim_channel_name( const string& str );
};


#endif
