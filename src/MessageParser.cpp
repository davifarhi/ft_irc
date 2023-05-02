#include "MessageParser.hpp"

client_cmd::client_cmd( string cmd, void(MessageParser::*exec)( Client&, string& ) ) : cmd(cmd), exec(exec) {}

MessageParser::MessageParser( IRCServer& server ) : server(server)
{
	this->init();
}

void MessageParser::init( void )
{
	cmd_list.push_back(client_cmd( string("CAP"), &MessageParser::execCAP ));
}

void MessageParser::parse( Client& client, string& line )
{
	for (vector<client_cmd>::iterator it = cmd_list.begin(); it != cmd_list.end(); it++)
	{
		if (line.rfind( it->cmd, 0 ) != line.npos)
			(this->*(it->exec))(client, line);
	}
}

void MessageParser::execCAP( Client& client, string& line )
{
	(void)client;
	if (line.find("LS") != line.npos)
	{
		client.on_cap_negotiation = true;
		server.send_message_to_client(client, "CAP * LS :\n");
	}
	else if (line.find("LIST") != line.npos)
	{
		server.send_message_to_client(client, "CAP * LIST :\n");
	}
	else if (line.find("REQ") != line.npos)
	{
		client.on_cap_negotiation = true;
		string msg("CAP * NAK ");
		msg += line.substr(line.find(':'));
		msg += '\n';
		server.send_message_to_client(client, msg);
	}
	else if (line.find("END") != line.npos)
	{
		client.on_cap_negotiation = false;
	}
}
