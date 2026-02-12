#pragma once

#include <stdexcept>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>

#include <algorithm>

/* --- NETWORK SHIT ---*/
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
/* --------------------*/

#include "irc.hpp"

#define MAX_EVENTS 10

// compliler needs this because of circular dependencies
// if you dont like this approach just remove this header from irc.hpp and include it separately
struct sockaddr_in;

class Server
{
private:
	int							_fd;
    int                         _epoll_fd;
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
	std::vector<Client>::iterator	findClient(int targetFd);
private:

	Server();
	
	void	_initServer();
    int     _setNonblocking(int fd);
    void    _handleMessages(int cfd, char *buffer);

    bool    _pass(const char *buf);
	void	_addNick(const char *buf, Client &client);
	void	_addUser(const char *buf, Client &client);
    void    _mode(const char *buf, int fd);
	void	_capLs(int fd);
	void	_emptyJoin(int fd);
	void	_welcome(int fd, Client &client);
	void    _pong(int fd);
    void    _motd(int fd, Client &client);

private:
	class	FdComparator // Functor (class/object with overloaded "()" operator to compare the values) for std::find_if, there are no lambdas in CPP98to use
	{
	private:
		int	_targetFd;
	public:
		explicit	FdComparator(int targetFd) : _targetFd(targetFd) {}
		bool		operator()(const Client &client) const { return (client.getFd() == _targetFd); }
	};
};
