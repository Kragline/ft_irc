#pragma once

#include <stdexcept>
#include "irc.hpp"

// compliler needs this because of circular dependencies
// if you dont like this approach just remove this header from irc.hpp and include it separately
struct sockaddr_in;

class Server
{
private:
	int							_fd;
	int							_port;
	std::string					_password;

	struct sockaddr_in			_clientInfo;

	ParseRequest				_parser;
	std::vector<Client>			_clients;
	std::vector<Channel>		_channels;
public:
	Server(int port, const std::string &password);
	Server(const Server &other);

	Server	&operator=(const Server &other);
	~Server();

	void	serverLoop();
private:

	Server();
	
	void	_initServer();
	void	_handleRegistration(int cfd, char *buffer);

	void	_addNick(const char *buf, Client &client);
	void	_addUser(const char *buf, Client &client);
	void	_capLs(int fd);
	void	_emptyJoin(int fd);
	void	_welcome(int fd, Client &client);
	void    _pong(int fd);
    void    _motd(int fd, Client &client);
};
