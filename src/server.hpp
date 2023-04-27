#ifndef SERVER_HPP
#define SERVER_HPP

#include <poll.h>
#include <vector>
#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <iostream>
#include <map>


#include "client.hpp"


class Server
{		
	public:
		Server(const std::string &port, const std::string &pass);
		~Server();

		void	start();

	private:
		const std::string	_port;
		const std::string	_pass;
		int	_socket;

		int	create_socket();
		void	client_connect();
		
		std::vector<pollfd>     _pfds;
		std::map<int, Client*>	_client;
};

#endif
