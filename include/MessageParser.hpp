#include "IRCServer.hpp"

#ifndef MESSAGEPARSER_H

#define MESSAGEPARSER_H

struct client_cmd
{
	string cmd;
	void(MessageParser::*exec)( Client& , string& );
	client_cmd( string cmd, void(MessageParser::*exec)( Client&, string& ) );
};

class MessageParser
{
	private:
		IRCServer& server;
		vector<client_cmd> cmd_list;
	public:
		MessageParser( IRCServer& server );

		void parse( Client& client, string& line );
	private:
		void init( void );

		void execCAP( Client& client, string& line );
};

#endif /* end of include guard: MESSAGEPARSER_H */
