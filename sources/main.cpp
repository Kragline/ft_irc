#include "irc.hpp"

int	main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << RED << "Error: " WHITE "Wrong number of arguments\n" BLUE "Usage: " GREEN "./ircserv " PURPLE "<port> <password>" WHITE << std::endl;
		return (1);
	}
	
	if (!(argv[1][0]) || !(argv[2][0]))
	{
		std::cerr << RED << "Error: " WHITE "Empty arguments aren't allowed" << std::endl;
		return (1);
	}

	std::stringstream	portStream(argv[1]);
	int					port;
	
	portStream >> port;
	if (portStream.fail())
	{
		std::cerr << RED << "Error: " WHITE "Wrong value for port" << std::endl;
		return (1);
	}

	try
	{
		Server	server(port, argv[2]);
		server.serverLoop();
	}
	catch(const std::exception& e)
	{
		std::cerr << RED << "Error: " WHITE << e.what() << std::endl;
		return (1);
	}
	return (0);
}
