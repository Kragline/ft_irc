#include "Server.hpp"

#include "Client.hpp"
#include "Channel.hpp"
#include "ParseRequest.hpp"
#include "Error.hpp"
#include "Reply.hpp"

#include "IrcCommon.hpp" // only if Server.cpp uses CAP_LS / JOIN_451 / JOIN_461 / color macros

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

	_clients.clear();
	_clientsByNicks.clear();
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
	_commands["KICK"] = &Server::_kickHandler;
	_commands["INVITE"] = &Server::_inviteHandler;
	_commands["TOPIC"] = &Server::_topicHandler;
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
	std::map<std::string, Channel>::iterator	it = _channels.find(name);
	if (it != _channels.end())
		return (&it->second);
	return (NULL);
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

	client.sendMessage(msg001);
	client.sendMessage(msg002);
	client.sendMessage(msg003);
	client.sendMessage(msg004);
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

std::string	Server::_getNick(const std::string &token)
{
    size_t	start = 5;
	size_t	end = token.find("\r");

	if (end == std::string::npos)
		end = token.length();

	return (token.substr(start, end - start));
}

bool	Server::_nickExists(const std::string &nick, int excludeFd)
{
	std::map<std::string, Client *>::iterator	it = _clientsByNicks.find(nick); 

	if (it != _clientsByNicks.end() && it->second->getFd() != excludeFd)
		return (true);

	return (false);
}

void	Server::_broadcastNickChange(Client &client, const std::string &oldNick, const std::string &newNick)
{
	std::string	msg = ":" + oldNick + "!" + client.getUser() + "@" +
						client.getHostname() + " NICK :" + newNick + "\r\n";

	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		it->second.sendMessage(msg);
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

void	Server::_capLSHandler(Client &client, const std::string &line)
{
	(void)line;
	client.sendMessage(CAP_LS);
}

void	Server::_passHandler(Client &client, const std::string &line)
{
	if (client.isRegistered()) { Error::_alreadyRegistered(client); return ; }

	if (line.length() <= 5)	{ Error::_needMoreParams(client, "PASS"); return ; }

    std::string temp = line.substr(5);
    std::string parameter = temp.substr(0, temp.find("\r\n"));

	if (parameter.empty() || parameter != _password) { Error::_passwordMismatch(client); return ; }

	client.setPassOk(true);
	_tryRegister(client);
}

void	Server::_nickHandler(Client &client, const std::string &line)
{
	if (line.length() <= 5)	{ Error::_needMoreParams(client, "NICK"); return ; }

	std::string newNick = _getNick(line);
	if (newNick.empty()) { Error::_noNicknameGiven(client);	return ; }

	if (!_isValidNick(newNick))	{ Error::_erroneousNickname(client, newNick); return ; }

	// if nickname is used, irc client will automatucally generate and send a new one
	if (_nickExists(newNick, client.getFd())) { Error::_nicknameInUse(client, newNick); return ; }

	std::string	oldNick = client.getNick();
	if (oldNick == newNick)
		return ;

	if (!oldNick.empty())
		_clientsByNicks.erase(oldNick);
	_clientsByNicks[newNick] = &client;

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
	if (client.isRegistered()) { Error::_alreadyRegistered(client); return ; }

	std::stringstream	ss(line);
	std::string			cmd, user, host, server, real;

	ss >> cmd >> user >> host >> server >> real;

	if (user.empty() || host.empty() ||
		server.empty() || real.empty())	{ Error::_needMoreParams(client, "USER"); return ; }

    client.setUser(user);
    client.setHostname(host);
    client.setServername(server);
	client.setRealName(real);

	client.setUserOk(true);
	_tryRegister(client);
}

void Server::_applyChannelModes(Client &client, Channel *channel, const std::string &modes, std::stringstream &ss)
{
	bool		adding = true;
	std::string	modeChanges;

	if (modes.find_first_not_of("+-itkol") != std::string::npos)
		return ;

	for (size_t i = 0; i < modes.size(); i++)
	{
		char c = modes[i];

		if (c == '+') { adding = true; modeChanges += c; }
		else if (c == '-') { adding = false; modeChanges += c; }

		else if (c == 'i')
		{
			channel->setInviteOnly(adding);
			modeChanges += 'i';
		}
		else if (c == 't')
		{
			channel->setTopicRestricted(adding);
			modeChanges += 't';
		}
		else if (c == 'k')
		{
			std::string	key;
			ss >> key;

			if (adding)
			{
				channel->setKey(key);
				modeChanges += 'k';
			}
			else
			{
				channel->removeKey();
				modeChanges += 'k';
			}
		}
		else if (c == 'l')
		{
			if (adding)
			{
				size_t	limit;
				if ((ss >> limit))
					channel->setLimit(limit);
			}
			else
				channel->removeLimit();

			modeChanges += 'l';
		}
		else if (c == 'o')
		{
			std::string	nick;
			ss >> nick;

			Client	*it = _findClient(nick);
			if (it)
			{
				if (adding)
					channel->addOperator(it);
				else
					channel->removeOperator(it);
			}

			modeChanges += 'o';
		}
	}

	std::string	msg = ":" + client.getNick() + "!" +
		client.getUser() + "@" + client.getHostname() +
		" MODE " + channel->getName() + " " + modeChanges + "\r\n";

	channel->broadcast(msg);
}

void	Server::_modeHandler(Client &client, const std::string &line)
{
	if (!client.isRegistered()) { Error::_notRegistered(client); return ; }

	std::stringstream	ss(line);
	std::string			cmd, target, modes, param;

	ss >> cmd >> target >> modes;
	if (target.empty()) { Error::_needMoreParams(client, "MODE"); return ; }

	if (target[0] != '#') { Reply::_uModeIs(client, modes); return ; }

	Channel	*channel = _findChannel(target);
	if (!channel) { Error::_noSuchChannel(client, target); return ; }

	if (modes.empty()) { Reply::_channelModeIs(client, target, channel->getModeString()); return ; }

	if (!channel->isOperator(&client)) { Error::_chanOpPrivsNeeded(client, target); return ; }

	_applyChannelModes(client, channel, modes, ss);
}

void	Server::_pingHandler(Client &client, const std::string &line)
{
	(void)line;
	client.sendMessage("PONG ircserv\r\n");
}

void	Server::_joinHandler(Client &client, const std::string &line)
{
	if (line == "JOIN :")
		return ;
	
	if (line.length() <= 5) { Error::_needMoreParams(client, "JOIN"); return ; }

	std::stringstream	ss(line);
    std::string			command, channelName, key;
    
	ss >> command >> channelName >> key;
	Channel	*channel = _findChannel(channelName);
	if (channel)
	{
		if (channel->isMember(&client))
			return ;
			
		if (channel->hasKey() && key != channel->getKey() ){ Error::_badChanKey(client, channelName); return ; }

		if (channel->isInviteOnly()	&& !channel->isInvited(&client)) { Error::_inviteOnlyChan(client, channelName); return ; }

		channel->addMember(&client);
		channel->removeInvited(&client);
	}
	else
	{
		std::pair<std::map<std::string, Channel>::iterator, bool> result =
			_channels.insert(std::make_pair(channelName, Channel(channelName, &client)));

		channel = &result.first->second;
	}

	std::string	joinMsg = ":" + client.getNick() + "!" +
		client.getUser() + "@" + client.getHostname() +
		" JOIN :" + channelName + "\r\n";

    channel->broadcast(joinMsg);
}

void	Server::_privmsgHandler(Client &client, const std::string &line)
{
	if (!client.isRegistered()) { Error::_notRegistered(client); return ; }

	std::stringstream	ss(line);
	std::string			command, target, message;

	ss >> command >> target;

	if (target.empty()) { Error::_needMoreParams(client, "PRIVMSG"); return ; }

	size_t	msgPos = line.find(" :");
	if (msgPos == std::string::npos) { Error::_needMoreParams(client, "PRIVMSG"); return ; }

	message = line.substr(msgPos + 2);
	if (message.empty())
		return ;

	std::string	fullMsg = ":" + client.getNick() + "!" + client.getUser() +
		"@" + client.getHostname() + " PRIVMSG " + target + " :" + message + "\r\n";

	if (target[0] == '#')
	{
		Channel	*channel = _findChannel(target);

		if (!channel) { Error::_noSuchChannel(client, target); return ; }

		if (!channel->isMember(&client)) { Error::_notOnChannel(client, target); return ; }

		channel->broadcast(fullMsg, &client);
	}
}

void	Server::_quitHandler(Client &client, const std::string &line)
{
	std::string	reason = "Client Quit";

	size_t	pos = line.find(" :");
	if (pos != std::string::npos)
		reason = line.substr(pos + 2);

	std::string	quitMsg = ":" + client.getNick() + "!" +
		client.getUser() + "@" + client.getHostname() +
		" QUIT :" + reason + "\r\n";

	std::map<std::string, Channel>::iterator	it = _channels.begin();
	while (it != _channels.end())
	{
		Channel	*channel = &it->second;

		if (channel->isMember(&client))
		{
			channel->broadcast(quitMsg, &client);
			channel->removeMember(&client);
			
			if (channel->isEmpty())
			{
				_channels.erase(it++);
				continue ;
			}
		}
		++it;
	}

	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, client.getFd(), NULL);
	close(client.getFd());

	_clients.erase(client.getFd());
	if (!client.getNick().empty())
		_clientsByNicks.erase(client.getNick());
}

void	Server::_kickHandler(Client &client, const std::string &line)
{
	std::stringstream	ss(line);
	std::string			cmd, channelName, targetNick, reason;

	ss >> cmd >> channelName >> targetNick;
	if (channelName.empty() || targetNick.empty()) { Error::_needMoreParams(client, "KICK"); return ; }
	
	size_t	pos = line.find(" :");
	reason = targetNick;
	if (pos != std::string::npos && line[pos + 2] != '\0')
		reason = line.substr(pos + 2);

	Channel	*channel = _findChannel(channelName);
	if (!channel) { Error::_noSuchChannel(client, channelName); return ; }

	if (!channel->isMember(&client)) { Error::_notOnChannel(client, channelName); return ; }

	if (!channel->isOperator(&client)) { Error::_chanOpPrivsNeeded(client, channelName); return ; }

	Client	*targetClient = _findClient(targetNick);
	if (!targetClient || !channel->isMember(targetClient))
		return ;
	
	std::string kickMsg = ":" + client.getNick() + "!" +
        client.getUser() + "@" + client.getHostname() +
        " KICK " + channelName + " " + targetNick +
        " :" + reason + "\r\n";

	channel->broadcast(kickMsg);
	channel->removeMember(targetClient);

	if (channel->isEmpty())
	{
		_channels.erase(channelName);
		return ;
	}
	
	if (!channel->operatorCount())
		channel->setNewOperator();
}

void	Server::_inviteHandler(Client &client, const std::string &line)
{
	if (!client.isRegistered()) { Error::_notRegistered(client); return ; }

	std::stringstream	ss(line);
	std::string			cmd, targetNick, channelName;

	ss >> cmd >> targetNick >> channelName;
	if (targetNick.empty() || channelName.empty()) { Error::_needMoreParams(client, "INVITE"); return ; }

	Channel	*channel = _findChannel(channelName);
	if (!channel) { Error::_noSuchChannel(client, channelName); return ; }

	if (!channel->isMember(&client)) { Error::_notOnChannel(client, channelName); return ; }

	if (channel->isInviteOnly() && !channel->isOperator(&client)) { Error::_chanOpPrivsNeeded(client, channelName); return ; }

	Client	*targetClient = _findClient(targetNick);
	if (!targetClient) { Error::_noSuchNick(client, channelName); return ; }

	if (channel->isMember(targetClient)) { Error::_userOnChannel(client, targetNick, channelName); return ; }

	channel->addInvited(targetClient);

	// Send invite to target
	std::string	inviteMsg =	":" + client.getNick() + "!" +
		client.getUser() + "@" + client.getHostname() +
		" INVITE " + targetNick + " :" + channelName + "\r\n";

	targetClient->sendMessage(inviteMsg);
	Reply::_inviting(client, targetNick, channelName);
}

void	Server::_topicHandler(Client &client, const std::string &line)
{
	if (!client.isRegistered()) { Error::_notRegistered(client); return; }

	std::stringstream	ss(line);
	std::string			cmd, channelName;

	ss >> cmd >> channelName;
	if (channelName.empty()) { Error::_needMoreParams(client, "TOPIC"); return; }

	Channel	*channel = _findChannel(channelName);
	if (!channel) { Error::_noSuchChannel(client, channelName); return; }

	if (!channel->isMember(&client)) { Error::_notOnChannel(client, channelName); return; }

	size_t	pos = line.find(" :");
	if (pos == std::string::npos)
	{
		if (channel->getTopic().empty())
			Reply::_noTopic(client, channel);
		else
			Reply::_topic(client, channel);
		return ;
	}

	if (channel->isTopicRestricted() && !channel->isOperator(&client)) { Error::_chanOpPrivsNeeded(client, channelName); return; }

	std::string	newTopic = line.substr(pos + 2);
	channel->setTopic(newTopic);

	std::string	topicMsg = ":" + client.getNick() + "!" + client.getUser() + "@" + client.getHostname() +
							" TOPIC " + channelName + " :" + newTopic + "\r\n";

	channel->broadcast(topicMsg);
}

void	Server::_dispatchCommand(Client &client, const std::string &line)
{
	std::stringstream	ss(line);
	std::string			cmd;
	ss >> cmd;

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
	Client							*client = _findClient(cfd);
	ssize_t							count;

	std::memset(buffer, 0x0, 512);
	while ((count = recv(cfd, buffer, 512, 0)) > 0)
	{
		request += buffer;
        std::cout << "Buffer: ";
        for (int i = 0; buffer[i]; i++)
        {
            if (buffer[i] == '\r')
                std::cout << "\\r";
            else if (buffer[i] == '\n')
                std::cout << "\\n";
            else
                std::cout << buffer[i];
        }
        std::cout << std::endl;
		std::memset(buffer, 0x0, 512);
	}

    std::cout << "Full request: ";
    for (int i = 0; request[i]; i++)
    {
        if (request[i] == '\r')
            std::cout << "\\r";
        else if (request[i] == '\n')
            std::cout << "\\n";
         else
            std::cout << request[i];
    }
    std::cout << std::endl;
    if (request.find("\r\n") == std::string::npos)
    {
        client->addToBuffer(request);
        return ;
    }
    else
    {
        client->addToBuffer(request);
        request = client->getBuffer();
        client->cleanBuffer();
    }
	parser.parseLine(request);
	tokens = parser.getTokens();
	for (size_t i = 0; i < tokens.size(); i++)
	{
		std::cout << "Token: " << tokens[i] << std::endl;
		_dispatchCommand(*client, tokens[i]);
	}

	if (count == 0)
	{
		std::cout << "Client " << (client->getNick().empty() ? "*" : client->getNick()) << " (" << cfd << ") clossed the connection" << std::endl;

		if (!client->getNick().empty())
			_clientsByNicks.erase(client->getNick());

		_clients.erase(cfd);
		close(cfd);
	}
	else if (count == -1)
	{
		if (errno != EAGAIN)
		{
			if (!client->getNick().empty())
				_clientsByNicks.erase(client->getNick());

			_clients.erase(cfd);
			close(cfd);
			throw std::runtime_error("recv");
		}
	}
}

Client	*Server::_findClient(int targetFd)
{
	std::map<int, Client>::iterator	it = _clients.find(targetFd);
	if (it != _clients.end())
		return (&it->second);

	return (NULL);
}

Client	*Server::_findClient(const std::string &targetNick)
{
	std::map<std::string, Client*>::iterator	it = _clientsByNicks.find(targetNick);
	if (it != _clientsByNicks.end())
		return (it->second);

	return (NULL);
}

void	Server::serverLoop()
{
	int					cfd;
	int					n_events;
	char				buffer[512];
	struct epoll_event	event;
	struct epoll_event	events[MAX_EVENTS];
	socklen_t			clientSize = 0;

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
					_clients.insert(std::make_pair(cfd, Client(cfd)));
				}
			}
			else        // Existing Client
				_handleMessages(events[i].data.fd, buffer); 
		}
	}
}
