#include "client.hpp"

Client::Client(int fd, int port, const std::string &hostname): _fd(fd), _port(port), _hostname(hostname)
{

}

Client::~Client() {}

int	Client::get_fd() const
{
	return (_fd);
}

int	Client::get_port() const
{
	return (_port);
}

std::string	Client::get_hostname() const
{
	return (_hostname);
}
