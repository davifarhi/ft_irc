#include "server.hpp"

Server::Server(const std::string &port, const std::string &pass) : _port(port), _pass(pass)
{
	_socket = create_socket();
}

Server::~Server() {}

void	Server::start()
{
	pollfd srv = {_socket, POLLIN, 0};
	_pfds.push_back(srv);
	
	std::cout << "serveur lancer\n";
	
	while(1)
	{
		int p = poll(_pfds.begin().base(), _pfds.size(), 0);
		if (p == -1)
		{
			std::cout << "erreur poll\n";
		}
		for (std::vector<pollfd>::iterator it = _pfds.begin(); it != _pfds.end(); it++)
		{
			if ((it->revents &POLLHUP) == POLLHUP)
			{
				client_disconnecte(it->fd);
				break;
			}
			if ((it->revents &POLLIN) == POLLIN)
			{
				if (it->fd == _socket)
				{
					client_connect();
					break;
				}
			}
//			client_messa(it->fd);
		}
	}
}

void	Server::client_disconnecte(int fd)
{
	//trouver le client et le delet
	Client* client = _client.at(fd);

	
	std::cout << "client is disconnected " << client->get_hostname().c_str() << " " << client->get_port() << "\n";
	_client.erase(fd);

	// trouver le client dans le poll et le delet a son tour sinon gros problem de boucle infinie	
	for (std::vector<pollfd>::iterator it = _pfds.begin(); it != _pfds.end(); it++)
	{
		if (it->fd == fd)
		{
			_pfds.erase(it);
			close(fd);
			break;
		}
	}

	// delete le client vue qu'on la new
	delete client;
}

void	Server::client_connect()
{
	sockaddr_in address;
	int fd;
	socklen_t addrlen = sizeof(address);
	
	fd = accept(_socket, (sockaddr *) &address, &addrlen);
	
	pollfd  pfd = {fd, POLLIN, 0};
	_pfds.push_back(pfd);
	
	char hostname[NI_MAXHOST];
	if (getnameinfo((struct sockaddr *) &address, sizeof(address), hostname, NI_MAXHOST, NULL, 0, NI_NUMERICSERV) != 0)
	{
		std::cout << "erreur getnameinfo\n";
		return;
	}
	Client* client = new Client(fd, ntohs(address.sin_port), hostname);
	_client.insert(std::make_pair(fd, client));

	std::cout << "message " << client->get_hostname().c_str() << " " << client->get_port() << "\n";
}

int	Server::create_socket()
{
	int opt = 1;
	struct sockaddr_in address;
	//creating socket file descriptor
	int sockefd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockefd < 0)
	{
		std::cout << "aie probleme de chausette\n";
		return (1);
	}
	//focefully attaching socket to te port
	if (setsockopt(sockefd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		std::cout << "chaussette pas dans le port\n";
		return (1);
	}
	//attention peut etre que la socket sera bloquante je sais pas ce que c'est
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(atoi(_port.c_str()));
	
	if (bind(sockefd, (struct sockaddr*)&address, sizeof(address)))
	{
		std::cout << "bind failure\n";
		return (1); 
	}
	//mise sur ecoute met le serveur en attente du client
	if (listen(sockefd, 128) < 0)
	{
		std::cout << "erreur dans l'ecoute socket\n";
		return (1);
	}
	return sockefd;
}
