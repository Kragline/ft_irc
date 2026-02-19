#pragma once

#include <stdexcept>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>

#include <algorithm>
#include <map>

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

	std::vector<Client>			_clients;
	std::vector<Channel>		_channels;


	typedef	void (Server::*CommandHandler)(Client &, const std::string &);
	std::map<std::string, CommandHandler>	_commands;
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
	void	_initCommands();
    int     _setNonblocking(int fd);
    void    _handleMessages(int cfd, char *buffer);

	void	_addUser(const char *buf, Client &client);
	void	_welcome(Client &client);
	void	_tryRegister(Client &client);

	void	_dispatchCommand(Client &client, const std::string &line);

	// command handlers
	void	_capLSHandler(Client &client, const std::string &line);
	void	_passHandler(Client &client, const std::string &line);
	void	_nickHandler(Client &client, const std::string &line);
	void	_userHandler(Client &client, const std::string &line);
	void	_modeHandler(Client &client, const std::string &line);
	void	_pingHandler(Client &client, const std::string &line);
	void	_joinHandler(Client &client, const std::string &line);

	// helpers
	std::string	_getNick(const std::string &token);

	bool	_nickExists(const std::string &nick, int excludeFd);
	bool	_isValidNick(const std::string &nick);
	void	_broadcastNickChange(Client &client, const std::string &oldNick, const std::string &newNick);

	// errors
	void	_alreadyRegistered(const Client &client);
	void	_needMoreParams(const Client &client, const std::string &command);
	void	_erroneousNickname(const Client &client, const std::string &nick);
	void	_nicknameInUse(const Client &client, const std::string &nick);
	void	_passwordMismatch(const Client &client);
	void	_noNicknameGiven(const Client &client);
	void	_notRegistered(const Client &client);

	class	FdComparator // Functor (class/object with overloaded "()" operator to compare the values) for std::find_if, there are no lambdas in CPP98to use
	{
	private:
		int	_targetFd;
	public:
		explicit	FdComparator(int targetFd) : _targetFd(targetFd) {}
		bool		operator()(const Client &client) const { return (client.getFd() == _targetFd); }
	};
};
