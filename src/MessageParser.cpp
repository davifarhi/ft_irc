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
	cmd_list.push_back(client_cmd( string("TOPIC"), &MessageParser::execTOPIC));
	cmd_list.push_back(client_cmd( string("MODE"), &MessageParser::execMODE ));
	cmd_list.push_back(client_cmd( string("INVITE"), &MessageParser::execINVITE ));
	cmd_list.push_back(client_cmd( string("KICK"), &MessageParser::execKICK ));
	cmd_list.push_back(client_cmd( string("NAMES"), &MessageParser::execNAMES ));
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
	size_t pos = line.find(':');
	if (pos == line.npos)
		return "";
	return line.substr(pos).erase(0, 1);
}

bool MessageParser::check_registration( Client& client )
{
	if (client.is_registration_done()) return true;
	server.send_message_to_client( client, ERR_NOTREGISTERED(client.nickname) );
	return false;
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
	client.leave_all_channels();
	server.send_msg_to_all(CMD_CONFIRM( client.nickname, client.hostname, "QUIT", ":Quit: " + reason ));
	server.send_message_to_client( client, "ERROR" );
	server.fds_to_disconnect.push(client.fd);
}

void MessageParser::execJOIN( Client& client, string& line )
{
	if (!check_registration(client)) return;
	vector<string> words = split_line(line);
	if (words.size() < 2 || !words[1].size())
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
		if (!chan.user_is_invited(client))
		{
			server.send_message_to_client( client, ERR_INVITEONLYCHAN( client.nickname, words[1]) );
			return;
		}
		if (!chan.is_there_space_for_newuser())
		{
			server.send_message_to_client( client, ERR_CHANNELISFULL( client.nickname, words[1]) );
			return;
		}
		if (!chan.join_client(client)) return;
		client.join_channel(chan);
		chan.send_msg_to_all( CMD_CONFIRM( client.nickname, client.hostname, "JOIN", "#" + chan.name ), server );
		if (chan.topic.size()) chan.send_topic_to_client( client, server );
		chan.send_names_to_client( client, server );
	}
}

void MessageParser::execPART( Client& client, string& line )
{
	if (!check_registration(client)) return;
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
		chan->part_client(client);
		client.part_channel(*chan);
		server.delete_channel_if_empty( chan );
	}
}

void MessageParser::execPRIVMSG( Client& client, string& line )
{
	if (!check_registration(client)) return;
	vector<string> words = split_line(line.substr( 0, line.find(':') ));
	string msg = get_argument(line);
	if (words.size() < 2)
	{
		server.send_message_to_client( client, ERR_NORECIPIENT( client.nickname, "PRIVMSG" ) );
	}
	else if (words.size() > 2)
	{
		server.send_message_to_client( client, ERR_TOOMANYTARGETS( client.nickname, "PRIVMSG" ) );
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
				if (chan->client_is_in_channel(client))
					chan->send_msg_to_all( PRIVMSG_CHAN( client.nickname, client.hostname, chan->name, msg ), server, &client );
				else
					server.send_message_to_client( client, ERR_NOTONCHANNEL( client.nickname, chan->name ) );
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

void MessageParser::execINVITE( Client& client, string& line )
{
	if (!check_registration(client)) return;
	vector<string> words = split_line(line);
	Client *target;
	Channel *chan;
	if (words.size() < 3)
	{
		server.send_message_to_client( client, ERR_NEEDMOREPARAMS( client.nickname, "INVITE" ) );
	}
	else if (!server.get_user( words[1], &target ))
	{
		server.send_message_to_client( client, ERR_NOSUCHNICK( client.nickname, words[1] ) );
	}
	else if (!server.get_channel( words[2], &chan ))
	{
		server.send_message_to_client( client, ERR_NOSUCHCHANNEL( client.nickname, words[2] ) );
	}
	else if (!chan->client_is_in_channel(client))
	{
		server.send_message_to_client( client, ERR_NOTONCHANNEL( client.nickname, words[2] ) );
	}
	else if (chan->client_is_in_channel(*target))
	{
		server.send_message_to_client( client, ERR_USERONCHANNEL( client.nickname, target->nickname, words[2] ) );
	}
	else if (chan->invite_only && !chan->get_chan_ops(client))
	{
		server.send_message_to_client( client, ERR_CHANOPRIVSNEEDED( client.nickname, words[2] ) );
	}
	else
	{
		chan->invited.insert(target);
		server.send_message_to_client( client, RPL_INVITING( client.nickname, target->nickname, chan->name ) );
		server.send_message_to_client( *target, CMD_CONFIRM( client.nickname, target->hostname, "INVITE", target->nickname + " #" + chan->name ) );
	}
}

void MessageParser::execTOPIC( Client& client, string& line )
{
	if (!check_registration(client)) return;
	vector<string> words = split_line(line);
	if (words.size() == 1)
	{
		server.send_message_to_client( client, ERR_NEEDMOREPARAMS( client.nickname, "TOPIC" ) );
		return;
	}
	if (line.find(":") == line.npos)
	{
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
			return;
		}
	}
	else
	{
		Channel* chan;
		if (!server.get_channel( words[1], &chan ))
		{
			server.send_message_to_client( client, ERR_NOSUCHCHANNEL( client.nickname, words[1] ) );
		}
		else if (!client.is_in_channel(*chan))
		{
			server.send_message_to_client( client, ERR_NOTONCHANNEL( client.nickname, words[1] ) );
		}
		else if (words.size() > 2)
		{
			Channel& chan = server.get_channel(words[1]);
			if (chan.protected_topic && !chan.get_chan_ops( client ))
			{
				server.send_message_to_client( client, ERR_CHANOPRIVSNEEDED( client.nickname, words[1] ) );
				return;
			}
			chan.change_topic_of_channel( get_argument(line), client );
			chan.send_msg_to_all( CMD_CONFIRM( client.nickname, client.hostname, "TOPIC", "#" + chan.name + " :" + chan.topic ), server );
		}
	}
}

void MessageParser::execMODE( Client& client, string& line )
{
	if (!check_registration(client)) return;
	vector<string> words = split_line(line);
	Channel* chann;
	if (words.size() == 1)
	{
		server.send_message_to_client( client, ERR_NEEDMOREPARAMS( client.nickname, "MODE" ) );
		return;
	}
	if (words.size() < 3 || !find_text(line, "#") || (!find_text(line, "-") && !find_text(line, "+")))
	{
		//mode request
		return;
	}
	if (!server.get_channel( words[1], &chann ))
	{
		server.send_message_to_client( client, ERR_NOSUCHCHANNEL( client.nickname, words[1] ) );
		return;
	}
	Channel& chan = server.get_channel(words[1]);
	if (!chan.get_chan_ops( client ))
	{
		server.send_message_to_client( client, ERR_CHANOPRIVSNEEDED( client.nickname, words[1] ) );
		return;
	}
	if (find_text(words[2], "+t") || find_text(words[2], "-t"))
	{
		if (words[2].find("+") != words[2].npos)
		{
			chan.change_privilege_topic(1);
			chan.send_msg_to_all( CMD_CONFIRM( client.nickname, client.hostname, "MODE", "#" + chan.name + " :" + "+t" ), server ); 
			return;
		}
		else
		{
			chan.change_privilege_topic(0);
			chan.send_msg_to_all( CMD_CONFIRM( client.nickname, client.hostname, "MODE", "#" + chan.name + " :" + "-t" ), server );
			return;
		}
	}
	if (find_text(words[2], "+o") || find_text(words[2], "-o"))
	{
		Client* user;
		if (server.get_user( words[3], &user ))
		{
			if (words[2].find("+") != words[2].npos)
			{
				chan.add_new_chan_ops( *user );
				chan.send_msg_to_all( CMD_CONFIRM( client.nickname, client.hostname, "MODE", "#" + chan.name + " +o " + user->nickname ), server );
				return;
			}
			else
			{
				chan.kick_user_of_chan_ops( *user );
				chan.send_msg_to_all( CMD_CONFIRM( client.nickname, client.hostname, "MODE", "#" + chan.name + " -o " + user->nickname ), server );
				return;
			}
		}
	}
	if (find_text(words[2], "+k") || find_text(words[2], "-k"))
	{
		if (words[2].find("+") != words[2].npos)
		{
			chan.change_channel_keys(words[3], 1);
			chan.send_msg_to_all( CMD_CONFIRM( client.nickname, client.hostname, "MODE", "#" + chan.name + " :" + "+k" ), server );
			return;
		}
		else
		{
			chan.change_channel_keys("", 0);
			chan.send_msg_to_all( CMD_CONFIRM( client.nickname, client.hostname, "MODE", "#" + chan.name + " :" + "-k" ), server );
			return;
		}
	}
	if (find_text(words[2], "+i") || find_text(words[2], "-i"))
	{
		if (words[2].find("+") != words[2].npos)
		{
			chan.change_status_invit_channel(true);
			chan.send_msg_to_all( CMD_CONFIRM( client.nickname, client.hostname, "MODE", "#" + chan.name + " :" + "+i" ), server );
			return;
		}
		else
		{
			chan.change_status_invit_channel(false);
			chan.send_msg_to_all( CMD_CONFIRM( client.nickname, client.hostname, "MODE", "#" + chan.name + " :" + "-i" ), server );
			return;
		}
	}
	if (find_text(words[2], "+l") || find_text(words[2], "-l"))
	{
		if (words[2].find("+l") != words[2].npos)
		{
			chan.change_limit_of_channel(words[3], 1);
			chan.send_msg_to_all( CMD_CONFIRM( client.nickname, client.hostname, "MODE", "#" + chan.name + " :" + "+l" ), server );
			return;
		}
		else
		{
			chan.change_limit_of_channel("random", 0);
			chan.send_msg_to_all( CMD_CONFIRM( client.nickname, client.hostname, "MODE", "#" + chan.name + " :" + "-l" ), server );
			return;
		}
	}
}

void MessageParser::execKICK( Client& client, string& line )
{
	if (!check_registration(client)) return;
	vector<string> words = split_line(line);
	Channel* chann;
	if (words.size() < 3)
	{
		server.send_message_to_client( client, ERR_NEEDMOREPARAMS( client.nickname, "KICK" ) );
		return;
	}
	if (!server.get_channel( words[1], &chann ))
	{
		server.send_message_to_client( client, ERR_NOSUCHCHANNEL( client.nickname, words[1] ) );
		return;
	}
	if (!chann->client_is_in_channel(client))
	{
		server.send_message_to_client( client, ERR_NOTONCHANNEL( client.nickname, words[1] ) );
		return;
	}
	Channel& chan = server.get_channel(words[1]);
	if (!chan.get_chan_ops( client ))
	{
		server.send_message_to_client( client, ERR_CHANOPRIVSNEEDED( client.nickname, words[1] ) );
		return;
	}
	Client* user;
	if (server.get_user( words[2], &user ))
	{
		if (user->is_in_channel(chan))
		{
			chan.send_msg_to_all( CMD_CONFIRM( client.nickname, client.hostname, "KICK", "#" + chan.name + " " + user->nickname ), server );
			chan.part_client(*user);
			user->part_channel(chan);
		}
		else
			server.send_message_to_client( client, ERR_USERNOTINCHANNEL( client.nickname, user->nickname, words[1] ) );
	}
	else
		server.send_message_to_client( client, ERR_NOSUCHNICK( client.nickname, words[1] ) );
}

void MessageParser::execNAMES( Client& client, string& line )
{
	if (!check_registration(client)) return;
	vector<string> words = split_line(line);
	Channel *chan;
	if (words.size() < 2)
	{
		server.send_message_to_client( client, ERR_NEEDMOREPARAMS( client.nickname, "NAMES" ) );
	}
	else if (!server.get_channel( words[1], &chan ))
	{
		server.send_message_to_client( client, ERR_NOSUCHCHANNEL( client.nickname, words[1] ) );
	}
	else if (!chan->client_is_in_channel(client))
	{
		server.send_message_to_client( client, ERR_NOTONCHANNEL( client.nickname, words[1] ) );
	}
	else
	{
		chan->send_names_to_client( client, server );
	}
}
