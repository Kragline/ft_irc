#include "irc.hpp"

std::vector<ClientState> clients;

int	init_server(void)
{
	int	opt = 1;
	struct sockaddr_in  serverInfo;
	std::memset(&serverInfo, 0, sizeof(serverInfo));
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_addr.s_addr = 0;
	serverInfo.sin_port = htons(5555);

	int	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
	{
		std::cerr << "socket" << std::endl;
		return -1;
	}
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		std::cerr << "setsockopt" << std::endl;
		close(fd);
		return -1;
	}
	if (bind(fd, (struct sockaddr *)&serverInfo, sizeof(serverInfo)) == -1)
	{
		std::cerr << "bind" << std::endl;
		close(fd);
		return -1;
	}
	if (listen(fd, 0) == -1)
	{
		std::cerr << "listen" << std::endl;
		close(fd);
		return -1;
	}
	return fd;
}

ssize_t	recv_line(int fd, char *buf)
{
	size_t i = 0;
	char c;
	ssize_t n;

	while (i < 512)
	{
		n = read(fd, &c, 1);
		if (n <= 0)
			return n; 
		buf[i] = c;
		if (c == '\n' && buf[i - 1] == '\r')
		{
			buf[i + 1] = '\0';
			return i;
		}
		i++;
	}
	buf[i] = '\0';
	return i;
}

void	add_nick(const char *buf, ClientState &client)
{
    std::string temp;
	int		i;

	for (i = 5; buf[i] != '\r' && i < 8; i++)
		temp.append(1, buf[i]);
	client.setNick(temp);
}

void	add_user(const char *buf, ClientState &client)
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

void	cap_ls(int fd)
{
	send(fd, CAP_LS, std::strlen(CAP_LS), 0);
}

void	empty_join(int fd)
{
	send(fd, JOIN_461, strlen(JOIN_461), 0);
}

void	welcome(int fd)
{
	send(fd, WELCOME_001, strlen(WELCOME_001), 0);
	send(fd, WELCOME_002, strlen(WELCOME_002), 0);
	send(fd, WELCOME_003, strlen(WELCOME_003), 0);
	send(fd, WELCOME_004, strlen(WELCOME_004), 0);
}

void    pong(int fd)
{
    send(fd, "PONG localhost\r\n", 16, 0);
}

void	handle_registration(int cfd, char *buffer)
{
  std::vector<std::string>  tokens;
  ClientState	newClient(cfd);

  while (recv(cfd, buffer, 512, 0) > 0)
  {
    std::string request(buffer);
	  ParseRequest parser(request);
    tokens = parser.getTokens();
    for (size_t i = 0; i < tokens.size(); i++)
    {
      if (tokens[i].find("CAP LS") != std::string::npos)
        cap_ls(cfd);
      else if (tokens[i].find("JOIN :") != std::string::npos)
        empty_join(cfd);
      else if (tokens[i].find("CAP END") != std::string::npos)
        welcome(cfd);
      else if (tokens[i].find("NICK") != std::string::npos)
        add_nick(tokens[i].c_str(), newClient);
      else if (tokens[i].find("USER") != std::string::npos)
      {
        add_user(tokens[i].c_str(), newClient); 
	    clients.push_back(newClient);
        std::cout << "Nick: " << clients[0].getNick() << std::endl;
        std::cout << "User: " << clients[0].getUser() << std::endl;
        std::cout << "Hostname: " << clients[0].getHostname() << std::endl;
        std::cout << "Servername: " << clients[0].getServername() << std::endl;
        std::cout << "Real Name: " << clients[0].getRealName() << std::endl;
      }
      else if (tokens[i].find("PING") != std::string::npos)
          pong(cfd);
    }
  }
}

int	main()
{
	int		fd;                               // Main connection socket file descriptor
	int		cfd;                              // Temp client socket file descriptor
	char	buffer[512];                     // Buffer for client messages
	bzero(buffer, sizeof(buffer)); 

	struct sockaddr_in  clientInfo; // Don't touch it
	std::memset(&clientInfo, 0, sizeof(clientInfo));
	socklen_t clientSize = 0;

	fd = init_server();
	if (fd == -1)
		return -1;

	while (true) // Main loop
	{
		cfd = accept(fd, (struct sockaddr *)&clientInfo, &clientSize);
		if (cfd == -1)
		{
            std::cerr << "accept" << std::endl;
			close(fd);
			return -1;
		}
		handle_registration(cfd, buffer); 
        recv(cfd, buffer, 512, 0);
	}
	close(fd);
	return 0;
}
