#include "Server.hpp"

Server::Server() : _fd(-1)
{
	std::memset(&_clientInfo, 0, sizeof(_clientInfo));
	_initServer();
}

Server::Server(const Server &other) : _fd(other._fd), _clientInfo(other._clientInfo), _clients(other._clients) {}

Server	&Server::operator=(const Server &other)
{
	if (this != &other)
	{
		_fd = other._fd;
		_clientInfo = other._clientInfo;
		_clients = other._clients;
	}
	return (*this);
}

Server::~Server() { if (_fd != -1) close(_fd); }

void	Server::_initServer()
{
	int	opt = 1;
	struct sockaddr_in  serverInfo;
	std::memset(&serverInfo, 0, sizeof(serverInfo));
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_addr.s_addr = 0;
	serverInfo.sin_port = htons(5555);

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

	if (listen(fd, 0) == -1)
	{
		close(fd);
		throw std::runtime_error("listen");
	}
	_fd = fd;
}

void	Server::_addNick(const char *buf, ClientState &client)
{
    std::string temp;
	int		i;

	for (i = 5; buf[i] != '\r' && i < 8; i++)
		temp.append(1, buf[i]);
	client.setNick(temp);
}

void	Server::_addUser(const char *buf, ClientState &client)
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

void	Server::_capLs(int fd)
{
	send(fd, CAP_LS, std::strlen(CAP_LS), 0);
}

void	Server::_emptyJoin(int fd)
{
	send(fd, JOIN_461, strlen(JOIN_461), 0);
}

void	Server::_welcome(int fd)
{
	send(fd, WELCOME_001, strlen(WELCOME_001), 0);
	send(fd, WELCOME_002, strlen(WELCOME_002), 0);
	send(fd, WELCOME_003, strlen(WELCOME_003), 0);
	send(fd, WELCOME_004, strlen(WELCOME_004), 0);
}

void    Server::_pong(int fd)
{
    send(fd, "PONG localhost\r\n", 16, 0);
}

void	Server::_handleRegistration(int cfd, char *buffer)
{
	std::vector<std::string>  tokens;
	ClientState	newClient(cfd);

	while (recv(cfd, buffer, 512, 0) > 0)
	{
		std::string request(buffer);

		_parser.parseLine(request);
		tokens = _parser.getTokens();

		for (size_t i = 0; i < tokens.size(); i++)
		{
			if (tokens[i].find("CAP LS") != std::string::npos)
				_capLs(cfd);
			else if (tokens[i].find("JOIN :") != std::string::npos)
				_emptyJoin(cfd);
			else if (tokens[i].find("CAP END") != std::string::npos)
				_welcome(cfd);
			else if (tokens[i].find("NICK") != std::string::npos)
				_addNick(tokens[i].c_str(), newClient);
			else if (tokens[i].find("USER") != std::string::npos)
			{
				_addUser(tokens[i].c_str(), newClient); 
				_clients.push_back(newClient);
				std::cout << "Nick: " << _clients.back().getNick() << std::endl;
				std::cout << "User: " << _clients.back().getUser() << std::endl;
				std::cout << "Hostname: " << _clients.back().getHostname() << std::endl;
				std::cout << "Servername: " << _clients.back().getServername() << std::endl;
				std::cout << "Real Name: " << _clients.back().getRealName() << std::endl;
			}
			else if (tokens[i].find("PING") != std::string::npos)
				_pong(cfd);
		}
		_parser.resetParser();
	}
}

void	Server::serverLoop()
{
	int			cfd;
	char		buffer[512];
	socklen_t	clientSize = 0;

	while (true)
	{
		cfd = accept(_fd, (struct sockaddr *)&_clientInfo, &clientSize);
		if (cfd == -1)
            std::runtime_error("accept");
		_handleRegistration(cfd, buffer); 
        recv(cfd, buffer, 512, 0);
	}
}
