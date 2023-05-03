#ifndef ANSWER_HPP
#define ANSWER_HPP

//#include<>

//Error reponsee

#define ERR_NONICKNAMEGIVEN(source)		"431 " + source + " :No nickname given\n"
#define ERR_NICKNAMEINUSE(source)		"433 " + source + " " + source  + " :Nickname is already in use\n"
#define ERR_ALREADYREGISTERED(source)		"462 " + source + " :You may not reregister\n"
#define ERR_PASSWDMISMATCH(source)		"464 " + source + " :Password is incorrect\n"

#define ERR_NEEDMOREPARAMS(source, command)	"461 " + source + " " + command + " :Not enough parameters\n"

//log reponse

#define RPL_WELCOME(source)			"001 " + source + " :Welcome " + source + " to the ft_irc network\n"

#endif

//!source = client
