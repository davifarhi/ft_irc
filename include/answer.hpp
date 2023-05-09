#ifndef ANSWER_HPP
#define ANSWER_HPP

//Error reponsee

#define ERR_NONICKNAMEGIVEN(source)		"431 " + source + " :No nickname given"
#define ERR_NICKNAMEINUSE(source, new_nick)		"433 " + source + " " + new_nick + " :Nickname is already in use"
#define ERR_ALREADYREGISTERED(source)		"462 " + source + " :You may not reregister"
#define ERR_PASSWDMISMATCH(source)		"464 " + source + " :Password is incorrect"

#define ERR_NEEDMOREPARAMS(source, command)	"461 " + source + " " + command + " :Not enough parameters"

#define ERR_NOSUCHCHANNEL(client, channel)	"403 " + client + " " + channel + " :No such channel"
#define ERR_NOTONCHANNEL(client, channel)	"442 " + client + " " + channel + " :You're not on that channel"
#define ERR_USERNOTINCHANNEL(client, nick, channel)	"441 " + client + " " + nick + " " + channel + " :They aren't on that channel"
#define ERR_BADCHANNELKEY(client, channel)	"475 " + client + " " + channel + " :Cannot join channel (+k)"
#define ERR_INVITEONLYCHAN(client, channel)	"473 " + client + " " + channel + " :Cannot join channel (+i)"
#define ERR_CHANNELISFULL(client, channel)	"471 " + client + " " + channel + " :Cannot join channel (+l)"

#define ERR_NOSUCHNICK(client, channel)		"401 " + client + " " + channel + " :No such nick/channel"
#define ERR_NORECIPIENT(client, command)	"411 " + client + " :No recipient given " + command
#define ERR_TOOMANYTARGETS(client, command)	"407 " + client + " :Too many targets " + command
#define ERR_NOTEXTTOSEND(client)			"412 " + client + " :No text to send"
#define ERR_CHANOPRIVSNEEDED(client, channel)	"482 " + client + " " + channel + " :You're not channel operator"

#define ERR_NOTREGISTERED(client)			"451 " + client + " :You have not registered"
#define ERR_USERONCHANNEL(client, nick, channel)	"443 " + client + " " + nick + " #" + channel + " :is already on channel"

//log reponse

#define RPL_WELCOME(source)			"001 " + source + " :Welcome " + source + " to the ft_irc network"
#define RPL_TOPIC(client, channel, topic)	"332 " + client + " #" + channel + " :" + topic
#define RPL_NOTOPIC(client, channel)		"331 " + client + " #" + channel + " :No topic is set"
#define RPL_NAMREPLY(client, channel, nick)	"353 " + client + " = " + channel + " :" + nick
#define RPL_ENDOFNAMES(client, channel)		"366 " + client + " " + channel + " :End of /NAMES list"
#define RPL_INVITING(client, nick, channel)	"341 " + client + " " + nick + " #" + channel

//server command answer

#define CMD_CONFIRM(nick, host, cmd, arg)	":" + nick + "@" + host + " " + cmd + " " + arg
//#define PRIVMSG_USER(Tnick, Rnick, msg)		":" + Tnick + " PRIVMSG " + Rnick + " :" + msg
#define PRIVMSG_USER(Tnick, host, Rnick, msg)		":" + Tnick + "@" + host + " PRIVMSG " + Rnick + " :" + msg
#define PRIVMSG_CHAN(nick, host, channel, message)	":" + nick + "@" + host + " PRIVMSG #" + channel + " :" + message

#endif
