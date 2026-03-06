#pragma once

#include <iostream>
#include <vector>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <map>

#include "IrcCommon.hpp"
#include "Channel.hpp"

class Client
{
	int         _fd;

	bool		_nickOk;
	bool		_userOk;
	bool		_passOk;
	bool		_registered;

	std::string _nick;
	std::string _user;
	std::string _hostname;
	std::string _servername;
	std::string _realName;

    std::string _buffer;

	std::map<std::string, Channel *>	_channels;
public:
	Client();
	Client(int fd);
	Client(const Client &other);

	Client	&operator=(const Client &other);

	~Client();

	int		getFd() const;
	void	setFd(int newFd);

	bool	hasPass() const ;
	bool	hasNick() const ;
	bool	hasUser() const ;
	bool	isRegistered() const ;

	void	setPassOk(bool status);
	void	setNickOk(bool status);
	void	setUserOk(bool status);
	void	setRegistered(bool status);

	void	sendMessage(const std::string &message) const ;

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

    const std::string   &getBuffer() const;
    void                addToBuffer(const std::string &buffer);
    void                cleanBuffer(void);

	void								addChannel(Channel *channel);
	void								removeChannel(Channel *channel);
	std::map<std::string, Channel *>	&getChannels();
};
