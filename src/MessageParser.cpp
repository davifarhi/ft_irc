#include "MessageParser.hpp"
#include "debug.hpp"
#include <string>

client_cmd::client_cmd( string cmd, void(MessageParser::*exec)( Client&, string& ) ) : cmd(cmd), exec(exec) {}

MessageParser::MessageParser( IRCServer& server ) : server(server)
{
	this->init();
}

void MessageParser::init( void )
{
	cmd_list.push_back(client_cmd( string("CAP"), &MessageParser::execCAP ));
	cmd_list.push_back(client_cmd( string("PASS"), &MessageParser::execPASS ));
	cmd_list.push_back(client_cmd( string("NICK"), &MessageParser::execNICK ));
	cmd_list.push_back(client_cmd( string("USER"), &MessageParser::execUSER ));
	cmd_list.push_back(client_cmd( string("PING"), &MessageParser::execUSER ));
	cmd_list.push_back(client_cmd( string("QUIT"), &MessageParser::execUSER ));
}

void MessageParser::parse( Client& client, string& line )
{
	for (vector<client_cmd>::iterator it = cmd_list.begin(); it != cmd_list.end(); it++)
	{
		if (line.rfind( it->cmd, 0 ) != line.npos)
			(this->*(it->exec))(client, line);
	}
}

bool MessageParser::find_text( string& line, string to_find ) const
{
	return (line.find(to_find) != line.npos);
}

string MessageParser::get_argument( string& line ) const
{
	return line.substr(line.find(':')).erase(0, 1);
}

vector<string> MessageParser::split_line( const string& line ) const
{
	vector<string> words;
	size_t pos_start = 0, pos_end = 0;
	while ((pos_end = line.find( ' ', pos_start )) != line.npos)
	{
		words.push_back(line.substr( pos_start, pos_end - pos_start ));
		pos_start = pos_end + 1;
	}
	if (pos_start < line.size())
		words.push_back(line.substr(pos_start));
	return words;
}

void MessageParser::execCAP( Client& client, string& line )
{
	if (find_text(line, "LS"))
	{
		client.on_cap_negotiation = true;
		server.send_message_to_client( client, "CAP * LS :\n" );
	}
	else if (find_text(line, "LIST"))
	{
		server.send_message_to_client( client, "CAP * LIST :\n" );
	}
	else if (find_text(line, "REQ"))
	{
		client.on_cap_negotiation = true;
		server.send_message_to_client( client, "CAP * NAK :" + get_argument(line) + '\n' );
	}
	else if (find_text(line, "END"))
	{
		client.on_cap_negotiation = false;
		if (client.registered)
		{
			server.send_message_to_client(client, RPL_WELCOME(client.nickname));
			if (DEBUG_PING_CLIENT)
				server.send_message_to_client( client, "PING :just a test\n" );
		}
	}
}

void MessageParser::execPASS( Client& client, string& line )
{
	vector<string> words = split_line(line);
	if (words.size() < 2)
	{
		server.send_message_to_client( client, ERR_NEEDMOREPARAMS( client.nickname, "PASS" ) );
	}
	else if (client.authenticated)
	{
		server.send_message_to_client( client, ERR_ALREADYREGISTERED(client.nickname) );
	}
	else if (words.back() != server.get_pswd())
	{
		server.send_message_to_client( client, ERR_PASSWDMISMATCH(client.nickname) );
	}
	else
	{
		if (DEBUG_PRINT_AUTHENTICATION)
			cout << client << " has been succesfully authenticated\n";
		client.authenticated = true;
	}
}

void MessageParser::execNICK( Client& client, string& line )
{
	vector<string> words = split_line(line);
	if (words.size() < 2)
	{
		server.send_message_to_client( client, ERR_NONICKNAMEGIVEN(client.nickname) );
	}
	else
	{
		// bad for performance, iterating through set
		for (set<Client>::iterator it = server.clients.begin(); it != server.clients.end(); it++)
		{
			if (it->nickname == words.back())
			{
				server.send_message_to_client( client, ERR_NICKNAMEINUSE(client.nickname) );
				return;
			}
		}
		if (DEBUG_PRINT_NICKNAME)
			cout << client << " has changed nickname to " << words.back() << endl;
		//server.send_message_to_client( client, client.nickname + " NICK " + words.back() + "\n" );
		client.nickname = words.back();
		//TODO (maybe) send message announcing change to all other users
		//https://modern.ircdocs.horse/#nick-message
	}
}

void MessageParser::execUSER( Client& client, string& line )
{
	vector<string> words = split_line(line);
	if (words.size() < 5)
	{
		server.send_message_to_client( client, ERR_NEEDMOREPARAMS( client.nickname, "USER" ) );
	}
	else if (client.registered)
	{
		server.send_message_to_client( client, ERR_ALREADYREGISTERED(client.nickname) );
	}
	else
	{
		client.username = words[1];
		client.realname = get_argument(line);
		client.registered = true;
		if (DEBUG_PRINT_USER_REAL_NAME)
			cout << client << " registered with name " << client.username << " and real name " << client.realname << endl;
		if (!client.on_cap_negotiation)
		{
			server.send_message_to_client(client, RPL_WELCOME(client.nickname));
			if (DEBUG_PING_CLIENT)
				server.send_message_to_client( client, "PING :just a test\n" );
		}
	}
}

void MessageParser::execPING( Client& client, string& line )
{
	vector<string> words = split_line(line);
	string msg("PONG");
	if (words.size() > 1)
		for (vector<string>::iterator it = ++words.begin(); it != words.end(); it++)
			msg += " " + *it;
	server.send_message_to_client( client, msg + '\n' );
}

void MessageParser::execQUIT( Client& client, string& line )
{
	string reason = get_argument(line);
	if (DEBUG_PRINT_CLIENT_QUIT)
		cout << client << " quit the network, reason: " << reason << endl;
}

// example for copy paste
void MessageParser::exec( Client& client, string& line )
{
	(void) client;
	(void) line;
}
