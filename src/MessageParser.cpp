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

#define PASSWDERROR(client) (client ": Password doesn't match")
		//cout << PASSWDERROR("test") << endl;

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
		//server.send_message_to_client( client, "001: dfarhi :Welcome to the network\n" );
		if (DEBUG_PING_CLIENT)
			server.send_message_to_client( client, "PING :just a test\n" );
	}
}

void MessageParser::execPASS( Client& client, string& line )
{
	vector<string> words = split_line(line);
	if (words.size() < 2)
	{
		cout << "ERR_NEEDMOREPARAMS\n";
		//ERR_NEEDMOREPARAMS
	}
	else if (client.authenticated)
	{
		cout << "ERR_ALREADYREGISTERED\n";
		//ERR_ALREADYREGISTERED
	}
	else if (words.back() != server.get_pswd())
	{
		cout << "'" << words.back() << "'\n'" << server.get_pswd() << "'\n";
		cout << (int)words.back().at(4) << endl;
		cout << "ERR_PASSWDMISMATCH\n";
		if (words.back().size() != server.get_pswd().size())
			cout << words.back().size() << " " << server.get_pswd().size() << " diferent sizes\n";
		//ERR_PASSWDMISMATCH
	}
	else
	{
		if (DEBUG_PRINT_AUTHENTICATION)
			cout << client << " has been succesfully authenticated\n";
		client.authenticated = true;
		cout << RPL_WELCOME(client.nickname) << "\n";
		server.send_message_to_client(client, RPL_WELCOME(client.nickname));
	}
}

void MessageParser::execNICK( Client& client, string& line )
{
	vector<string> words = split_line(line);
	if (words.size() < 2)
	{
		cout << "ERR_NONICKNAMEGIVEN\n";
	}
	else
	{
		// bad for performance, iterating through set
		for (set<Client>::iterator it = server.clients.begin(); it != server.clients.end(); it++)
		{
			if (it->nickname == words.back())
			{
				cout << "ERR_NICKNAMEINUSE\n";
				return;
			}
		}
		if (DEBUG_PRINT_NICKNAME)
			cout << client << " has changed nickname to " << words.back() << endl;
		server.send_message_to_client( client, client.nickname + " NICK " + words.back() + "\n" );
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
		cout << "ERR_NEEDMOREPARAMS\n";
	}
	else if (client.registered)
	{
		cout << "ERR_ALREADYREGISTERED\n";
	}
	else
	{
		client.username = words[1];
		client.realname = get_argument(line);
		client.registered = true;
		if (DEBUG_PRINT_USER_REAL_NAME)
			cout << client << " registered with name " << client.username << " and real name " << client.realname << endl;
	}
}
