#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <vector>
#include <string>
#include <iostream>


class Client 
{
	private:
        
		int             _fd;
		int             _port;

        	std::string     _hostname;

	public:
		Client(int fd, int port, const std::string &hostname);
		~Client();

		int	get_fd() const;
		int	get_port() const;

		std::string	get_hostname() const;
};

#endif
