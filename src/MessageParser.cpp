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
	cmd_list.push_back(client_cmd( string("PING"), &MessageParser::execPING ));
	cmd_list.push_back(client_cmd( string("QUIT"), &MessageParser::execQUIT ));
	cmd_list.push_back(client_cmd( string("JOIN"), &MessageParser::execJOIN ));
	cmd_list.push_back(client_cmd( string("PART"), &MessageParser::execPART ));
	cmd_list.push_back(client_cmd( string("PRIVMSG"), &MessageParser::execPRIVMSG ));
	cmd_list.push_back(client_cmd( string("TOPIC"), &MessageParser::execTopic));
}

void MessageParser::parse( Client& client, string& line )
{
	for (vector<client_cmd>::iterator it = cmd_list.begin(); it != cmd_list.end(); it++)
	{
		if (line.rfind( it->cmd, 0 ) != line.npos)
		{
			(this->*(it->exec))(client, line);
			return;
		}
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
		if (server.get_user( words.back(), 0 ))
		{
			server.send_message_to_client( client, ERR_NICKNAMEINUSE( client.nickname, words.back() ) );
			return;
		}
		if (DEBUG_PRINT_NICKNAME)
			cout << client << " has changed nickname to " << words.back() << endl;
		server.send_msg_to_all(CMD_CONFIRM( client.nickname, client.hostname, "NICK", words.back() ));
		client.nickname = words.back();
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
	for (vector<pollfd>::iterator it = server.pfds.begin(); it != server.pfds.end(); it++)
	{
		if (it->fd == client.fd)
		{
			it->events = POLLHUP;
			break;
		}
	}
	client.leave_all_channels();
	server.send_msg_to_all(CMD_CONFIRM( client.nickname, client.hostname, "QUIT", ":Quit: " + reason ));
	server.send_message_to_client( client, "ERROR" );
}

void MessageParser::execJOIN( Client& client, string& line )
{
	vector<string> words = split_line(line);
	if (words.size() < 2)
	{
		server.send_message_to_client( client, ERR_NEEDMOREPARAMS( client.nickname, "JOIN" ) );
	}
	else
	{
		Channel& chan = server.get_channel(words[1]);
		if (!chan.try_password((words.size() > 2 ? words[2] : "")))
		{
			server.send_message_to_client( client, ERR_BADCHANNELKEY( client.nickname, words[1] ) );
			return;
		}
		if (!chan.join_client(client)) return;
		client.join_channel(chan);
		chan.send_msg_to_all( CMD_CONFIRM( client.nickname, client.hostname, "JOIN", "#" + chan.name ), server );
		chan.send_topic_to_client( client, server );
		chan.send_names_to_client( client, server );
	}
}

void MessageParser::execPART( Client& client, string& line )
{
	vector<string> words = split_line(line);
	if (words.size() < 2)
	{
		server.send_message_to_client( client, ERR_NEEDMOREPARAMS( client.nickname, "PART" ) );
		return;
	}
	Channel* chan;
	if (!server.get_channel( words[1], &chan ))
	{
		server.send_message_to_client( client, ERR_NOSUCHCHANNEL( client.nickname, words[1] ) );
	}
	else if (!client.is_in_channel(*chan))
	{
		server.send_message_to_client( client, ERR_NOTONCHANNEL( client.nickname, words[1] ) );
	}
	else
	{
		chan->send_msg_to_all( CMD_CONFIRM( client.nickname, client.hostname, "PART", "#" + chan->name ), server );
		//reference for KICK
		chan->part_client(client);
		client.part_channel(*chan);
		server.delete_channel_if_empty( chan );
	}
}

void MessageParser::execPRIVMSG( Client& client, string& line )
{
	vector<string> words = split_line(line.substr( 0, line.find(':') ));
	string msg = get_argument(line);
	if (words.size() < 2)
	{
		server.send_message_to_client( client, ERR_NORECIPIENT( client.nickname, "PRIVMSG" ) );
	}
	else if (words.size() > 2)
	{
		server.send_message_to_client( client, ERR_TOOMANYTARGETS( client.nickname, "PRIVMSG" ) );
		cout << words.size() << " args received\n";
	}
	else if (!msg.size())
	{
		server.send_message_to_client( client, ERR_NOTEXTTOSEND(client.nickname) );
	}
	else
	{
		if (words[1][0] == '#')
		{
			Channel* chan;
			if (server.get_channel( words[1], &chan ))
			{
				chan->send_msg_to_all( PRIVMSG_CHAN( client.nickname, client.hostname, chan->name, msg ), server, &client );
				return;
			}
		}
		else
		{
			Client* user;
			if (server.get_user( words[1], &user ))
			{
				server.send_message_to_client( *user, PRIVMSG_USER( client.nickname, client.hostname, user->nickname, msg ) );
				return;
			}
		}
		server.send_message_to_client( client, ERR_NOSUCHCHANNEL( client.nickname, words.back() ) );
	}
}

// example for copy paste
void MessageParser::exec( Client& client, string& line )
{
	(void) client;
	(void) line;
}

void MessageParser::execTopic( Client& client, string& line)
{
	if (!line.find(":"))
	{
		vector<string> words = split_line(line);
		Channel* chan;
		if (!server.get_channel( words[1], &chan ))
		{
			server.send_message_to_client( client, ERR_NOSUCHCHANNEL( client.nickname, words[1] ) );
		}
		else if (!client.is_in_channel(*chan))
		{
			server.send_message_to_client( client, ERR_NOTONCHANNEL( client.nickname, words[1] ) );
		}
		else
		{
			Channel& chan = server.get_channel(words[1]);
			chan.send_topic_to_client( client, server );
		}
	}
	else
	{
		vector<string> words = split_line(line);
		Channel* chan;
		if (!server.get_channel( words[1], &chan ))
		{
			server.send_message_to_client( client, ERR_NOSUCHCHANNEL( client.nickname, words[1] ) );
		}
		else if (!client.is_in_channel(*chan))
		{
			server.send_message_to_client( client, ERR_NOTONCHANNEL( client.nickname, words[1] ) );
		}
		if (words.size() > 2)
		{
			Channel& chan = server.get_channel(words[1]);
			chan.change_topic_of_channel(line.substr((line.find(':')) + 2));
			chan.send_topic_to_client( client, server );//je comprend pas pourquoi ca marche pas sans ca 
		}
	}
}
