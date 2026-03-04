#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>

#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define MAX_EVENTS 10

class Client;
class Channel;

class Server
{
private:
	int							_fd;
	int							_epoll_fd;
	int							_port;
	std::string					_password;

	struct sockaddr_in			_clientInfo;

	std::vector<Client *>		_clients;
	std::vector<Channel *>		_channels;

	typedef	void (Server::*CommandHandler)(Client &, const std::string &);
	std::map<std::string, CommandHandler>	_commands;

public:
	Server(int port, const std::string &password);
	Server(const Server &other);

	Server	&operator=(const Server &other);
	~Server();

	void	serverLoop();

private:
    Server();

	void	_initServer();
	void	_initCommands();
	int		_setNonblocking(int fd);
	void	_handleMessages(int cfd, char *buffer);

	void	_welcome(Client &client);
	void	_tryRegister(Client &client);

	void	_dispatchCommand(Client &client, const std::string &line);

	std::vector<Client *>::iterator	_findClient(int targetFd);
	std::vector<Client *>::iterator	_findClient(const std::string &targetNick);

	// command handlers
	void	_capLSHandler(Client &client, const std::string &line);
	void	_passHandler(Client &client, const std::string &line);
	void	_nickHandler(Client &client, const std::string &line);
	void	_userHandler(Client &client, const std::string &line);
	void	_modeHandler(Client &client, const std::string &line);
	void	_pingHandler(Client &client, const std::string &line);
	void	_joinHandler(Client &client, const std::string &line);
	void	_privmsgHandler(Client &client, const std::string &line);
	void	_quitHandler(Client &client, const std::string &line);
	void	_kickHandler(Client &client, const std::string &line);
	void	_inviteHandler(Client &client, const std::string &line);
	void	_topicHandler(Client &client, const std::string &line);

	// channels
	Channel	*_findChannel(const std::string &name);
	Channel	*_createChannel(const std::string &name, Client *creator);

	// helpers
	std::string	_getNick(const std::string &token);
	void		_applyChannelModes(Client &client, Channel *channel, const std::string &modes, std::stringstream &ss);

	bool	_nickExists(const std::string &nick, int excludeFd);
	bool	_isValidNick(const std::string &nick);
	void	_broadcastNickChange(Client &client, const std::string &oldNick, const std::string &newNick);
};
