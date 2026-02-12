#pragma once

#include <iostream>
#include <vector>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>

class Client
{
	int         _fd;
    bool        _authorized;

	std::string _nick;
	std::string _user;
	std::string _hostname;
	std::string _servername;
	std::string _realName;
public:
	Client();
	Client(int fd);
	Client(const Client &other);

	Client	&operator=(const Client &other);

	~Client();

	int		getFd() const;
	void	setFd(int newFd);

    bool    getAuthorized() const;
    void    setAuthorized(bool newStatus);

    bool    getCapListed() const;
    void    setCapListed(bool newStatus);

	const std::string	&getNick() const;
	void				setNick(const std::string &newNick);

	const std::string	&getUser() const;
	void				setUser(const std::string &newUser);

	const std::string	&getHostname() const;
	void				setHostname(const std::string &newHostname);

	const std::string	&getServername() const;
	void				setServername(const std::string &newServername);

	const std::string	&getRealName() const;
	void				setRealName(const std::string &newRealName);
};
