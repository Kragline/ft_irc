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

void	handle_nick(char *buf, ClientState &client)
{
	char	temp[9];
	int		i;

	for (i = 0; buf[i] != '\r' && i < 8; i++)
		temp[i] = buf[i];
	temp[i] = '\0';
	client.setNick(temp);
}

void	handle_user(char *buf, ClientState &client)
{
	char	temp[9];
	int		i;

	for (i = 0; buf[i] != ' '; i++)
		temp[i] = buf[i];
	temp[i] = '\0';
	client.setUser(temp);

	for (i = 0; buf[i] != ' '; i++)
    	temp[i] = buf[i];
	temp[i] = '\0';
	client.setRealName(temp);
}

void	handle_registration(int cfd, char *buffer, ClientState &client)
{
	recv_line(cfd, buffer);
	if (strncmp(buffer, "CAP LS", 6) == 0)
		send(cfd, "CAP * LS :\r\n", 13, 0);

  while (strcmp("JOIN :\r\n", buffer) != 0)
		recv_line(cfd, buffer);
  send(cfd, JOIN_451, strlen(JOIN_451), 0);
	while (strcmp("CAP END\r\n", buffer) != 0)
		recv_line(cfd, buffer);

	recv_line(cfd, buffer);
	if (strncmp("NICK", buffer, 4) == 0)
		handle_nick(buffer + 5, client);

	recv_line(cfd, buffer);
	if (strncmp("USER", buffer, 4) == 0)
		handle_user(buffer + 5, client);

	send(cfd, WELCOME_001, strlen(WELCOME_001), 0);
	send(cfd, WELCOME_002, strlen(WELCOME_002), 0);
	send(cfd, WELCOME_003, strlen(WELCOME_003), 0);
	send(cfd, WELCOME_004, strlen(WELCOME_004), 0);
 
  recv(cfd, buffer, 512, 0);
  recv(cfd, buffer, 512, 0);
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
			perror("accept");
			close(fd);
			return -1;
		}

		ClientState	newClient(cfd);
		clients.push_back(newClient);

		handle_registration(cfd, buffer, clients.back()); 
	}
	close(fd);
	return 0;
}
