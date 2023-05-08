#include "IRCServer.hpp"

#ifndef CLIENT_H

#define CLIENT_H

class Client
{
	private:
		static unsigned int guest_n;

		const int fd;
		const int port;
		const string hostname;

		bool on_cap_negotiation;
		bool authenticated;
		bool registered;
		string nickname;
		string username;
		string realname;

		set<Channel*> channels;

		friend class IRCServer;
		friend class Channel;
		friend class MessageParser;

	public:
		Client( int fd );
		Client( int fd, int port, const string &hostname );

		bool is_in_channel( Channel& channel ) const;

		void join_channel( Channel& channel );
		void part_channel( Channel& channel );

		void leave_all_channels( void );
		bool is_registration_done( void );

		friend bool operator<( const Client& lhs, const Client& rhs );
		friend std::ostream& operator<<( std::ostream& os, const Client& c );
		friend bool operator==( const pollfd& lhs, const Client& rhs );
};


#endif /* end of include guard: CLIENT_H */
