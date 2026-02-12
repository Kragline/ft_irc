#include "Client.hpp"

Client::Client() : _fd(-1), _authorized(false) {}

Client::Client(int fd) : _fd(fd), _authorized(false) {}

Client::Client(const Client &other) : _fd(other._fd), _authorized(other._authorized), _nick(other._nick), _user(other._user), _hostname(other._hostname), _servername(other._servername), _realName(other._realName) {}

Client	&Client::operator=(const Client &other)
{
	if (this == &other)
		return (*this);
	
	_fd = other._fd;
    _authorized = other._authorized;
	_nick = other._nick;
    _user = other._user;
    _hostname = other._hostname;
    _servername = other._servername;
	_realName = other._realName;

	return (*this);
}

Client::~Client() {}

int		Client::getFd() const { return (_fd); }
void	Client::setFd(int newFd) { _fd = newFd; }

bool    Client::getAuthorized() const {return (_authorized); }
void    Client::setAuthorized(bool newStatus) {_authorized = newStatus;}

const std::string	&Client::getNick() const { return (_nick); }
void				Client::setNick(const std::string &newNick) { _nick = newNick; }

const std::string	&Client::getUser() const { return (_user); }
void				Client::setUser(const std::string &newUser) { _user = newUser; }

const std::string	&Client::getRealName() const { return (_realName); }
void				Client::setRealName(const std::string &newRealName) { _realName = newRealName; }

const std::string	&Client::getHostname() const { return (_hostname); }
void				Client::setHostname(const std::string &newHostname) { _hostname = newHostname; }

const std::string	&Client::getServername() const { return (_servername); }
void				Client::setServername(const std::string &newServername) { _servername = newServername; }
