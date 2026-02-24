#include "Server.hpp"

Server::Server() : _fd(-1), _epoll_fd(-1), _port(5555), _password("bismillah")
{
	std::memset(&_clientInfo, 0, sizeof(_clientInfo));
	_initServer();
	_initCommands();
}

Server::Server(int port, const std::string &password) : _fd(-1), _epoll_fd(-1), _port(port), _password(password)
{
	std::memset(&_clientInfo, 0, sizeof(_clientInfo));
	_initServer();
	_initCommands();
}

Server::Server(const Server &other) : _fd(other._fd), _epoll_fd(other._epoll_fd), _port(other._port), _clientInfo(other._clientInfo), _clients(other._clients), _channels(other._channels) {}

Server	&Server::operator=(const Server &other)
{
	if (this != &other)
	{
		_fd = other._fd;
        _epoll_fd = other._epoll_fd;
        _port = other._port;
		_clientInfo = other._clientInfo;
		_clients = other._clients;
		_channels = other._channels;
	}
	return (*this);
}

Server::~Server()
{
	if (_fd != -1)
		close(_fd);
	
	for (size_t i = 0; i < _channels.size(); i++)
		delete _channels[i];
	
	for (size_t i = 0; i < _clients.size(); i++)
		delete _clients[i];
}

void	Server::_initServer()
{
	int	opt = 1;
    int epoll_fd;
    struct epoll_event  event;
	struct sockaddr_in  serverInfo;
	std::memset(&serverInfo, 0, sizeof(serverInfo));
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_addr.s_addr = 0;
	serverInfo.sin_port = htons(_port);

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
		throw std::runtime_error("socket");
	
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		close(fd);
		throw std::runtime_error("setsockopt");
	}
	
	if (bind(fd, (struct sockaddr *)&serverInfo, sizeof(serverInfo)) == -1)
	{
		close(fd);
		throw std::runtime_error("bind");
	}

    if (_setNonblocking(fd) == -1)
    {
        close(fd);
        throw std::runtime_error("_setNonblocking");
    }
    
	if (listen(fd, 0) == -1)
	{
		close(fd);
		throw std::runtime_error("listen");
	}

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        close(fd);
        throw std::runtime_error("epoll_create1");
    }
    event.events = EPOLLIN;
    event.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1)
    {
        close(fd);
        close(epoll_fd);
        throw std::runtime_error("epoll_ctl");
    }
	_fd = fd;
    _epoll_fd = epoll_fd;
}

void	Server::_initCommands()
{
	_commands["CAP"] = &Server::_capLSHandler;
	_commands["PASS"] = &Server::_passHandler;
	_commands["NICK"] = &Server::_nickHandler;
	_commands["USER"] = &Server::_userHandler;
	_commands["JOIN"] = &Server::_joinHandler;
	_commands["PING"] = &Server::_pingHandler;
	_commands["MODE"] = &Server::_modeHandler;
	_commands["PRIVMSG"] = &Server::_privmsgHandler;
	_commands["QUIT"] = &Server::_quitHandler;
}

int     Server::_setNonblocking(int fd)
{
    int flags;

    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return (-1);
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        return (-1);
    return (1);
}

Channel	*Server::_findChannel(const std::string &name)
{
	for (size_t i = 0; i < _channels.size(); i++)
	{
		if (_channels[i]->getName() == name)
			return (_channels[i]);
	}
	return (NULL);
}

Channel	*Server::_createChannel(const std::string &name, Client *creator)
{
	Channel	*newChannel = new Channel(name, creator);
	_channels.push_back(newChannel);
	return (newChannel);
}

void	Server::_addUser(const char *buf, Client &client)
{
    std::string temp;
	int		i = 5;
    
    for (; buf[i] != ' '; i++)
        temp.append(1, buf[i]);
    client.setUser(temp);
    temp.clear();

    i++;
    for (; buf[i] != ' '; i++)
        temp.append(1, buf[i]);
    client.setHostname(temp);
    temp.clear();

    i++;
    for (; buf[i] != ' '; i++)
        temp.append(1, buf[i]);
    client.setServername(temp);
    temp.clear();

    i += 2;
    for (; buf[i] != '\0'; i++)
		temp.append(1, buf[i]);
	client.setRealName(temp);
}

void	Server::_welcome(Client &client)
{
	std::string nick = client.getNick();

	std::string msg001 = ":ircserv 001 " + nick +
		" :Welcome to the Internet Relay Network " +
		nick + "!" + client.getUser() + "@" + client.getHostname() + "\r\n";

	std::string msg002 = ":ircserv 002 " + nick +
		" :Your host is ircserv, running version 1.0\r\n";

	std::string msg003 = ":ircserv 003 " + nick +
		" :This server was created today\r\n";

	std::string msg004 = ":ircserv 004 " + nick + " ircserv 1.0 o o\r\n";

	send(client.getFd(), msg001.c_str(), msg001.size(), 0);
	send(client.getFd(), msg002.c_str(), msg002.size(), 0);
	send(client.getFd(), msg003.c_str(), msg003.size(), 0);
	send(client.getFd(), msg004.c_str(), msg004.size(), 0);
}

void	Server::_tryRegister(Client &client)
{
	if (client.isRegistered())
		return ;
	
	if (client.hasPass() && client.hasNick() && client.hasUser())
	{
		client.setRegistered(true);
		_welcome(client);
	}
}

void	Server::_alreadyRegistered(const Client &client)
{
	std::string msg = ":ircserv 462 " + (client.getNick().empty() ? "*" : client.getNick()) +
		" :You may not reregister\r\n";

	send(client.getFd(), msg.c_str(), msg.size(), 0);
}

std::string	Server::_getNick(const std::string &token)
{
    size_t	start = 5;
	size_t	end = token.find("\r");

	if (end == std::string::npos)
		end = token.length();

	return token.substr(start, end - start);
}

bool	Server::_nickExists(const std::string &nick, int excludeFd)
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i]->getFd() != excludeFd &&
			_clients[i]->getNick() == nick)
			return (true);
	}
	return (false);
}

void	Server::_broadcastNickChange(Client &client, const std::string &oldNick, const std::string &newNick)
{
	std::string msg = ":" + oldNick + "!" + client.getUser() + "@" +
						client.getHostname() + " NICK :" + newNick + "\r\n";

	for (size_t i = 0; i < _clients.size(); i++)
		send(_clients[i]->getFd(), msg.c_str(), msg.size(), 0);
}

void	Server::_needMoreParams(const Client &client, const std::string &command)
{
	std::string nick = client.getNick().empty() ? "*" : client.getNick();

	std::string msg = ":ircserv 461 " + nick + " " + command +
						" :Not enough parameters\r\n";

	send(client.getFd(), msg.c_str(), msg.size(), 0);
}

bool	Server::_isValidNick(const std::string &nick)
{
	if (nick.empty() || nick.length() > 9)
		return (false);

	std::string	special = "[]\\`_^{|}";

	if (!std::isalpha(nick[0]) && special.find(nick[0]) == std::string::npos)
		return (false);

	for (size_t i = 1; i < nick.length(); i++)
	{
		if (!std::isalnum(nick[i]) &&
			special.find(nick[i]) == std::string::npos &&
			nick[i] != '-')
			return (false);
	}
	return (true);
}

void	Server::_erroneousNickname(const Client &client, const std::string &nick)
{
	std::string msg = ":ircserv 432 " + nick + " :Erroneous nickname\r\n";

	send(client.getFd(), msg.c_str(), msg.length(), 0);
}

void	Server::_nicknameInUse(const Client &client, const std::string &nick)
{
	std::string msg = ":ircserv 433 * " + nick + " :Nickname is already in use\r\n";

	send(client.getFd(), msg.c_str(), msg.size(), 0);
}

void	Server::_passwordMismatch(const Client &client)
{
	send(client.getFd(), ":ircserv 464 * :Password incorrect\r\n", 39, 0);
}

void	Server::_noNicknameGiven(const Client &client)
{
	send(client.getFd(), ":ircserv 431 * :No nickname given\r\n", 35, 0);
}

void	Server::_notRegistered(const Client &client)
{
	send(client.getFd(), ":ircserv 451 * :You have not registered\r\n", 43, 0);
}

void	Server::_noSuchChannel(const Client &client, const std::string &name)
{
	std::string msg = ":ircserv 403 " + name + " :No such channel\r\n";

	send(client.getFd(), msg.c_str(), msg.size(), 0);
}

void	Server::_capLSHandler(Client &client, const std::string &line)
{
	(void)line;
	send(client.getFd(), CAP_LS, std::strlen(CAP_LS), 0);
}

void	Server::_passHandler(Client &client, const std::string &line)
{
	if (client.isRegistered())
	{
		_alreadyRegistered(client);
		return ;
	}

	if (line.length() <= 5)
	{
		_needMoreParams(client, "PASS");
		return ;
	}

    std::string temp = line.substr(5);
    std::string parameter = temp.substr(0, temp.find("\r\n"));

	if (parameter.empty() || parameter != _password)
	{
		_passwordMismatch(client);
		return ;
	}

	client.setPassOk(true);
	_tryRegister(client);
}

void	Server::_nickHandler(Client &client, const std::string &line)
{
	if (line.length() <= 5)
	{
		_needMoreParams(client, "NICK");
		return ;
	}

	std::string newNick = _getNick(line);
	if (newNick.empty())
	{
		_noNicknameGiven(client);
		return ;
	}

	if (!_isValidNick(newNick))
	{
		_erroneousNickname(client, newNick);
		return ;
	}

	// if nickname is used, irc client will automatucally generate and send a new one
	if (_nickExists(newNick, client.getFd()))
	{
		_nicknameInUse(client, newNick);
		return ;
	}

	std::string	oldNick = client.getNick();
	if (oldNick == newNick)
		return ;

	client.setNick(newNick);
	if (!client.isRegistered())
	{
		client.setNickOk(true);
		_tryRegister(client);
	}
	else
	{
		_broadcastNickChange(client, oldNick, newNick);
	}
}

void	Server::_userHandler(Client &client, const std::string &line)
{
	if (client.isRegistered())
	{
		_alreadyRegistered(client);
		return ;
	}

	std::istringstream	iss(line);
	std::string			cmd, user, host, server, real;

	iss >> cmd >> user >> host >> server;

	if (!(iss >> real))
	{
		_needMoreParams(client, "USER");
		return ;
	}

	_addUser(line.c_str(), client);
	client.setUserOk(true);
	_tryRegister(client);
}

void	Server::_modeHandler(Client &client, const std::string &line)
{
	if (!client.isRegistered())
	{
		_notRegistered(client);
		return ;
	}

	std::string temp = line.substr(5);
    std::string parameter = temp.substr(0, temp.find("\r\n"));

    send(client.getFd(), std::string("221 " + temp + " " + parameter + "\r\n").c_str(), temp.length() + parameter.length() + 3, 0);
}

void	Server::_pingHandler(Client &client, const std::string &line)
{
	(void)line;
	send(client.getFd(), "PONG localhost\r\n", 16, 0);
}

void	Server::_joinHandler(Client &client, const std::string &line)
{
	if (line.length() <= 5)
	{
		_needMoreParams(client, "JOIN");
		return ;
	}

	std::stringstream	iss(line);
    std::string			command, channelName;
    
	iss >> command >> channelName;
	Channel	*channel = _findChannel(channelName);
	if (channel)
		channel->addMember(&client);
	else
		channel = _createChannel(channelName, &client);

	std::string joinMsg = ":" + client.getNick() + "!" +
		client.getUser() + "@" + client.getHostname() +
		" JOIN :" + channelName + "\r\n";

    channel->broadcast(joinMsg);
}

void	Server::_privmsgHandler(Client &client, const std::string &line)
{
	if (!client.isRegistered())
	{
		_notRegistered(client);
		return ;
	}

	std::stringstream	iss(line);
	std::string			command, target, message;

	iss >> command >> target;

	if (target.empty())
	{
		_needMoreParams(client, "PRIVMSG");
		return ;
	}

	size_t	msgPos = line.find(" :");
	if (msgPos == std::string::npos)
	{
		_needMoreParams(client, "PRIVMSG");
		return ;
	}

	message = line.substr(msgPos + 2);
	if (message.empty())
		return ;

	std::string	fullMsg = ":" + client.getNick() + "!" + client.getUser() +
		"@" + client.getHostname() + " PRIVMSG " + target + " :" + message + "\r\n";

	if (target[0] == '#')
	{
		Channel *channel = _findChannel(target);

		if (!channel)
		{
			_noSuchChannel(client, target);
			return ;
		}

		channel->broadcast(fullMsg, &client);
	}
}

void Server::_quitHandler(Client &client, const std::string &line)
{
	std::string	reason = "Client Quit";

	size_t	pos = line.find(" :");
	if (pos != std::string::npos)
		reason = line.substr(pos + 2);

	std::string	quitMsg = ":" + client.getNick() + "!" +
		client.getUser() + "@" + client.getHostname() +
		" QUIT :" + reason + "\r\n";

	for (size_t i = 0; i < _channels.size(); )
	{
		Channel	*channel = _channels[i];

		if (channel->isMember(&client))
		{
			channel->broadcast(quitMsg, &client);
			channel->removeMember(&client);

			if (channel->isEmpty())
			{
				delete channel;
				_channels.erase(_channels.begin() + i);
				continue ;
			}
		}
		i++;
	}

	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, client.getFd(), NULL);
	close(client.getFd());

	std::vector<Client*>::iterator it = _findClient(client.getFd());
	if (it != _clients.end())
	{
		delete *it;
		_clients.erase(it);
	}
}

void	Server::_dispatchCommand(Client &client, const std::string &line)
{
	std::stringstream	iss(line);
	std::string			cmd;
	iss >> cmd;

	std::map<std::string, CommandHandler>::iterator it = _commands.find(cmd);
	if (it != _commands.end())
	{
		CommandHandler	handler = it->second;
		(this->*handler)(client, line);
	}
}

void	Server::_handleMessages(int cfd, char *buffer)
{
	ParseRequest					parser;
	std::string						request;
	std::vector<std::string>		tokens;
	std::vector<Client *>::iterator	client = _findClient(cfd);
	ssize_t							count;

	std::memset(buffer, 0x0, 512);
	while ((count = recv(cfd, buffer, 512, 0)) > 0)
	{
		request += buffer;
		std::memset(buffer, 0x0, 512);
	}

	parser.parseLine(request);
	tokens = parser.getTokens();
	for (size_t i = 0; i < tokens.size(); i++)
    {
        std::cout << tokens[i] << std::endl;
        _dispatchCommand(*(*client), tokens[i]);
    }

	if (count == 0)
	{
		std::cout << "Client " << ((*client)->getNick().empty() ? "*" : (*client)->getNick()) << " (" << cfd << ") clossed the connection" << std::endl;
		close(cfd);
		_clients.erase(client);
	}
	else if (count == -1)
	{
		if (errno != EAGAIN)
		{
			close(cfd);
			_clients.erase(client);
			throw std::runtime_error("recv");
		}
	}
}

std::vector<Client *>::iterator	Server::_findClient(int targetFd)
{
	std::vector<Client *>::iterator it = std::find_if(
		_clients.begin(),
		_clients.end(),
		FdComparator(targetFd)
	);

	return (it);
}

void	Server::serverLoop()
{
	int					cfd;
	int					n_events;
	char				buffer[512];
	struct epoll_event	event;
	struct epoll_event	events[MAX_EVENTS];
	socklen_t			clientSize = 0;
	Client				*newClient;

	while (true)
	{
		n_events = epoll_wait(_epoll_fd, events, MAX_EVENTS, -1);
		if (n_events == -1)
		{
			close(_fd);
			close(_epoll_fd);
			throw std::runtime_error("epoll_wait");
		}
		for (int i = 0; i < n_events; i++)
		{
			if (events[i].data.fd == _fd)   // New Client
			{
				while (true)
				{
					cfd = accept(_fd, (struct sockaddr *)&_clientInfo, &clientSize);
					if (cfd == -1)
					{
						if (errno == EAGAIN || errno == EWOULDBLOCK)
							break ;
						else
						{
							close(cfd);
							throw std::runtime_error("accept");
						}
					}
					if (_setNonblocking(cfd) == -1)
					{
						close(cfd);
						throw std::runtime_error("_setNonblocking");
					}
					event.events = EPOLLIN | EPOLLET;
					event.data.fd = cfd;
					if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, cfd, &event) == -1)
					{
						close(cfd);
						throw std::runtime_error("epoll_ctl");
					}
					newClient = new Client();
					newClient->setFd(cfd);
					_clients.push_back(newClient);
				}
			}
			else        // Existing Client
				_handleMessages(events[i].data.fd, buffer); 
		}
	}
}
