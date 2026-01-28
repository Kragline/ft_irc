#pragma once

#include <iostream>
#include <vector>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>

/* --- NETWORK SHIT ---*/
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>
/* --------------------*/

typedef enum Role
{
	OPERATOR,
	REGULAR
}	role_e;

class ClientState
{
	int         _fd;
	role_e      _role;

	std::string _nick;
	std::string _user;
	std::string _realName;
public:
	ClientState();
	ClientState(int fd, role_e role = REGULAR);
	ClientState(const ClientState &other);

	ClientState	&operator=(const ClientState &other);

	~ClientState();

	int		getFd() const;
	void	setFd(int newFd);

	role_e	getRole() const;
	void	setRole(role_e newRole);

	const std::string	&getNick() const;
	void				setNick(const std::string &newNick);

	const std::string	&getUser() const;
	void				setUser(const std::string &newUser);

	const std::string	&getRealName() const;
	void				setRealName(const std::string &newRealName);
};
