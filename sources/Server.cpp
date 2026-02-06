#include "Server.hpp"

Server::Server() : _fd(-1), _port(5555), _password("bismillah")
{
	std::memset(&_clientInfo, 0, sizeof(_clientInfo));
	_initServer();
}

Server::Server(int port, const std::string &password) : _fd(-1), _port(port), _password(password)
{
	std::memset(&_clientInfo, 0, sizeof(_clientInfo));
	_initServer();
}

Server::Server(const Server &other) : _fd(other._fd), _clientInfo(other._clientInfo), _clients(other._clients), _channels(other._channels) {}

Server	&Server::operator=(const Server &other)
{
	if (this != &other)
	{
		_fd = other._fd;
		_clientInfo = other._clientInfo;
		_clients = other._clients;
		_channels = other._channels;
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

	if (listen(fd, 0) == -1)
	{
		close(fd);
		throw std::runtime_error("listen");
	}
	_fd = fd;
}

void	Server::_addNick(const char *buf, Client &client)
{
    std::string temp;
	int		i;

	for (i = 5; buf[i] != '\r' && i < 8; i++)
		temp.append(1, buf[i]);
	client.setNick(temp);
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

void	Server::_capLs(int fd)
{
	send(fd, CAP_LS, std::strlen(CAP_LS), 0);
}

void	Server::_emptyJoin(int fd)
{
	send(fd, JOIN_461, strlen(JOIN_461), 0);
}

void	Server::_welcome(int fd, Client &client)
{
    std::string welcome_001("001 " + client.getNick() + " :Welcome to the IRC Network " + client.getNick() + "\r\n");
    std::string welcome_002("002 " + client.getNick() + " :Your host is " + client.getHostname() + "\r\n");
    std::string welcome_003("003 " + client.getNick() + " :This server was created ...\r\n");

	send(fd, welcome_001.c_str(), welcome_001.length(), 0);
	send(fd, welcome_002.c_str(), welcome_002.length(), 0);
	send(fd, welcome_003.c_str(), welcome_003.length(), 0);
}

void    Server::_pong(int fd)
{
    send(fd, "PONG localhost\r\n", 16, 0);
}

void    Server::_motd(int fd, Client &client)
{
    std::string motd_375("375 " + client.getNick() + " :- \r\n");

    std::string motd_372_1("372 " + client.getNick() + " :- ⣿⣿⣿⣿⣿⣿⣿⣿⡿⠿⠛⠛⠛⠋⠉⠈⠉⠉⠉⠉⠛⠻⢿⣿⣿⣿⣿⣿⣿⣿\r\n");
    std::string motd_372_2("372 " + client.getNick() + " :- ⣿⣿⣿⣿⣿⡿⠋⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠛⢿⣿⣿⣿⣿\r\n");
    std::string motd_372_3("372 " + client.getNick() + " :- ⣿⣿⣿⣿⡏⣀⠀⠀⠀⠀⠀⠀⠀⣀⣤⣤⣤⣄⡀⠀⠀⠀⠀⠀⠀⠀⠙⢿⣿⣿\r\n");
    std::string motd_372_4("372 " + client.getNick() + " :- ⣿⣿⣿⢏⣴⣿⣷⠀⠀⠀⠀⠀⢾⣿⣿⣿⣿⣿⣿⡆⠀⠀⠀⠀⠀⠀⠀⠈⣿⣿\r\n");
    std::string motd_372_5("372 " + client.getNick() + " :- ⣿⣿⣟⣾⣿⡟⠁⠀⠀⠀⠀⠀⢀⣾⣿⣿⣿⣿⣿⣷⢢⠀⠀⠀⠀⠀⠀⠀⢸⣿\r\n");
    std::string motd_372_6("372 " + client.getNick() + " :- ⣿⣿⣿⣿⣟⠀⡴⠄⠀⠀⠀⠀⠀⠀⠙⠻⣿⣿⣿⣿⣷⣄⠀⠀⠀⠀⠀⠀⠀⣿\r\n");
    std::string motd_372_7("372 " + client.getNick() + " :- ⣿⣿⣿⠟⠻⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠶⢴⣿⣿⣿⣿⣿⣧⠀⠀⠀⠀⠀⠀⣿\r\n");
    std::string motd_372_8("372 " + client.getNick() + " :- ⣿⣁⡀⠀⠀⢰⢠⣦⠀⠀⠀⠀⠀⠀⠀⠀⢀⣼⣿⣿⣿⣿⣿⡄⠀⣴⣶⣿⡄⣿\r\n");
    std::string motd_372_9("372 " + client.getNick() + " :- ⣿⡋⠀⠀⠀⠎⢸⣿⡆⠀⠀⠀⠀⠀⠀⣴⣿⣿⣿⣿⣿⣿⣿⠗⢘⣿⣟⠛⠿⣼\r\n");
    std::string motd_372_10("372 " + client.getNick() + " :- ⣿⣿⠋⢀⡌⢰⣿⡿⢿⡀⠀⠀⠀⠀⠀⠙⠿⣿⣿⣿⣿⣿⡇⠀⢸⣿⣿⣧⢀⣼\r\n");
    std::string motd_372_11("372 " + client.getNick() + " :- ⣿⣿⣷⢻⠄⠘⠛⠋⠛⠃⠀⠀⠀⠀⠀⢿⣧⠈⠉⠙⠛⠋⠀⠀⠀⣿⣿⣿⣿⣿\r\n");
    std::string motd_372_12("372 " + client.getNick() + " :- ⣿⣿⣧⠀⠈⢸⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠟⠀⠀⠀⠀⢀⢃⠀⠀⢸⣿⣿⣿⣿\r\n");
    std::string motd_372_13("372 " + client.getNick() + " :- ⣿⣿⡿⠀⠴⢗⣠⣤⣴⡶⠶⠖⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⡸⠀⣿⣿⣿⣿\r\n");
    std::string motd_372_14("372 " + client.getNick() + " :- ⣿⣿⣿⡀⢠⣾⣿⠏⠀⠠⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠛⠉⠀⣿⣿⣿⣿\r\n");
    std::string motd_372_15("372 " + client.getNick() + " :- ⣿⣿⣿⣧⠈⢹⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣰⣿⣿⣿⣿\r\n");
    std::string motd_372_16("372 " + client.getNick() + " :- ⣿⣿⣿⣿⡄⠈⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣠⣴⣾⣿⣿⣿⣿⣿\r\n");
    std::string motd_372_17("372 " + client.getNick() + " :- ⣿⣿⣿⣿⣧⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣠⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿\r\n");
    std::string motd_372_18("372 " + client.getNick() + " :- ⣿⣿⣿⣿⣷⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣴⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿\r\n");
    std::string motd_372_19("372 " + client.getNick() + " :- ⣿⣿⣿⣿⣿⣦⣄⣀⣀⣀⣀⠀⠀⠀⠀⠘⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿\r\n");
    std::string motd_372_20("372 " + client.getNick() + " :- ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⡄⠀⠀⠀⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿\r\n");
    std::string motd_372_21("372 " + client.getNick() + " :- ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣧⠀⠀⠀⠙⣿⣿⡟⢻⣿⣿⣿⣿⣿⣿⣿⣿⣿\r\n");
    std::string motd_372_22("372 " + client.getNick() + " :- ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠇⠀⠁⠀⠀⠹⣿⠃⠀⣿⣿⣿⣿⣿⣿⣿⣿⣿\r\n");
    std::string motd_372_23("372 " + client.getNick() + " :- ⣿⣿⣿⣿⣿⣿⣿⣿⡿⠛⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⢐⣿⣿⣿⣿⣿⣿⣿⣿⣿\r\n");
    std::string motd_372_24("372 " + client.getNick() + " :- ⣿⣿⣿⣿⠿⠛⠉⠉⠁⠀⢻⣿⡇⠀⠀⠀⠀⠀⠀⢀⠈⣿⣿⡿⠉⠛⠛⠛⠉⠉\r\n");
    std::string motd_372_25("372 " + client.getNick() + " :- ⣿⡿⠋⠁⠀⠀⢀⣀⣠⡴⣸⣿⣇⡄⠀⠀⠀⠀⢀⡿⠄⠙⠛⠀⣀⣠⣤⣤⠄\r\n");

    std::string motd_376("376 " + client.getNick() + ":- End of MOTD.\r\n");

    send(fd, motd_375.c_str(), motd_375.length(), 0);
    send(fd, motd_372_1.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_2.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_3.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_4.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_5.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_6.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_7.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_8.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_9.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_10.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_11.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_12.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_13.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_14.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_15.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_16.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_17.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_18.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_19.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_20.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_21.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_22.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_23.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_24.c_str(), motd_372_1.length(), 0);
    send(fd, motd_372_25.c_str(), motd_372_1.length(), 0);
    send(fd, motd_376.c_str(), motd_376.length(), 0);
}

void	Server::_handleRegistration(int cfd, char *buffer)
{
	std::vector<std::string>  tokens;
	Client	newClient(cfd);

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
			else if (tokens[i].find("NICK") != std::string::npos)
				_addNick(tokens[i].c_str(), newClient);
			else if (tokens[i].find("USER") != std::string::npos)
			{
				_addUser(tokens[i].c_str(), newClient); 
				_clients.push_back(newClient);
				_welcome(cfd, newClient);
                _motd(cfd, newClient);
			}
			else if (tokens[i].find("PING") != std::string::npos)
				_pong(cfd);
		}
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
