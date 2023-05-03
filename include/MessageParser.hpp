#include "IRCServer.hpp"

#ifndef MESSAGEPARSER_H

#define MESSAGEPARSER_H
#include "answer.hpp"

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

		bool find_text( string& line, string to_find ) const;
		string get_argument( string& line ) const;
		vector<string> split_line( const string& line ) const;

		void execCAP( Client& client, string& line );
		void execPASS( Client& client, string& line );
		void execNICK( Client& client, string& line );
		void execUSER( Client& client, string& line );
		void execPING( Client& client, string& line );
		void execQUIT( Client& client, string& line );
		void execJOIN( Client& client, string& line );
		void execPART( Client& client, string& line );

		//example for copy paste
		void exec( Client& client, string& line );
};

#endif /* end of include guard: MESSAGEPARSER_H */
