#ifndef CHANNEL_HPP

#define CHANNEL_HPP

class Channel;

#include "IRCServer.hpp"

class Channel
{
	private:
		list<Client*> clients;
		set<Client*> chan_ops;
		set<Client*> invited;

		string name;
		string topic;
		
		bool protected_topic;

		bool invite_only;
		size_t user_limit;

		bool has_password;
		string password;

		friend class IRCServer;
		friend class MessageParser;

	public:
		Channel( const string& name );

		bool client_is_in_channel( Client& client ) const;
		bool join_client( Client& client );
		void part_client( Client& client );

		void send_topic_to_client( const Client& client, IRCServer& server ) const;
		void send_names_to_client( Client& client, IRCServer& server);

		void send_msg_to_all( string msg, IRCServer& server, Client* exception = 0 );

		bool try_password( const string& pswd ) const;
		bool user_is_invited ( Client & client ) const;
		bool is_there_space_for_newuser( void ) const;

		size_t default_user_limit( void ) const;

		friend bool operator<( const Channel& lhs, const Channel& rhs );

		static string trim_channel_name( const string& str );
	
		void change_topic_of_channel( const string str, Client& client);
		bool get_chan_ops( Client& client);
		void change_privilege_topic(bool temp);
		void add_new_chan_ops( Client& client );
		void kick_user_of_chan_ops( Client& client );
		void change_channel_keys(string str, int rep);
		void change_status_invit_channel( bool info );
		void change_limit_of_channel( string str, int rep );
};


#endif
