#include "Server.hpp"

Server::Server() : _fd(-1), _epoll_fd(-1), _port(5555), _password("bismillah")
{
	std::memset(&_clientInfo, 0, sizeof(_clientInfo));
	_initServer();
}

Server::Server(int port, const std::string &password) : _fd(-1), _epoll_fd(-1), _port(port), _password(password)
{
	std::memset(&_clientInfo, 0, sizeof(_clientInfo));
	_initServer();
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

Server::~Server() { if (_fd != -1) close(_fd); }

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

int     Server::_setNonblocking(int fd)
{
    int flags;

    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return -1;
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        return -1;
    return 1;
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

void    Server::_mode(const char *buf, int cfd)
{
    std::string temp(buf + 5);
    std::string parameter;

    parameter = temp.substr(0, temp.find("\r\n"));
    send(cfd, std::string("221 " + temp + " " + parameter + "\r\n").c_str(), temp.length() + parameter.length() + 3, 0);
}

bool    Server::_pass(const char *buf)
{
    if (std::strlen(buf) <= 5)
        return false;

    std::string temp(buf + 5);
    std::string parameter;
    
    parameter = temp.substr(0, temp.find("\r\n"));
    if (parameter.empty())
        return false;
    return parameter == _password;
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
    std::string motd_375("375 " + client.getNick() + " :- Message of the day - \r\n");

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

    std::string motd_376("376 " + client.getNick() + ":- End of /MOTD command\r\n");

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

void	Server::_handleMessages(int cfd, char *buffer)
{
    std::string request;
	std::vector<std::string>  tokens;
	Client	newClient(cfd);
    ssize_t count;

    bzero(buffer, std::strlen(buffer));
	while ((count = recv(cfd, buffer, 512, 0)) > 0)
	{
		request += buffer;
		bzero(buffer, std::strlen(buffer));
	}
    
    _parser.parseLine(request);
	tokens = _parser.getTokens();
	for (size_t i = 0; i < tokens.size(); i++)
	{
        if (tokens[i].find("PASS") != std::string::npos)
        {
            if (_pass(tokens[i].c_str()) == false)
            {
                close(cfd);
                break ;
            }
        }
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
            //_motd(cfd, newClient);
		}
        else  if (tokens[i].find("MODE") != std::string::npos)
            _mode(tokens[i].c_str(), cfd);
        else if (tokens[i].find("PING") != std::string::npos)
            _pong(cfd);
    }

    if (count == 0)
    {
        std::cout << "Client " << cfd << " clossed the connection" << std::endl;
        close(cfd);
    }
    else if (count == -1)
    {
        if (errno != EAGAIN)
        {
            close(cfd);
            perror("recv");
        }
    }
}

void	Server::serverLoop()
{
	int			        cfd;
    int                 n_events;
	char		        buffer[512];
    struct epoll_event  event;
    struct epoll_event  events[MAX_EVENTS];
	socklen_t	        clientSize = 0;

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
            if (events[i].data.fd == _fd)
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
                            perror("accept");
                            break ;
                        }
                    }
                    if (_setNonblocking(cfd) == -1)
                    {
                         close(cfd);
                         perror("_setNonblocking");
                         break ;
                    }
                    event.events = EPOLLIN | EPOLLET;
                    event.data.fd = cfd;
                    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, cfd, &event) == -1)
                    {
                        close(cfd);
                        perror("epoll_ctl");
                    }
                }
            }
            else
            {
                cfd = events[i].data.fd;
                _handleMessages(cfd, buffer); 
            }
        }
	}
}
