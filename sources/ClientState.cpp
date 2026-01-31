#include "ClientState.hpp"

ClientState::ClientState() : _fd(-1), _role(REGULAR) {}

ClientState::ClientState(int fd, role_e role) : _fd(fd), _role(role) {}

ClientState::ClientState(const ClientState &other) : _fd(other._fd), _role(other._role), _nick(other._nick), _user(other._user), _hostname(other._hostname), _servername(other._servername), _realName(other._realName) {}

ClientState	&ClientState::operator=(const ClientState &other)
{
	if (this == &other)
		return (*this);
	
	_fd = other._fd;
	_role = other._role;
	_nick = other._nick;
    _user = other._user;
    _hostname = other._hostname;
    _servername = other._servername;
	_realName = other._realName;

	return (*this);
}

ClientState::~ClientState() {}

int		ClientState::getFd() const { return (_fd); }
void	ClientState::setFd(int newFd) { _fd = newFd; }

role_e	ClientState::getRole() const { return (_role); }
void	ClientState::setRole(role_e newRole) { _role = newRole; }

const std::string	&ClientState::getNick() const { return (_nick); }
void				ClientState::setNick(const std::string &newNick) { _nick = newNick; }

const std::string	&ClientState::getUser() const { return (_user); }
void				ClientState::setUser(const std::string &newUser) { _user = newUser; }

const std::string	&ClientState::getRealName() const { return (_realName); }
void				ClientState::setRealName(const std::string &newRealName) { _realName = newRealName; }

const std::string	&ClientState::getHostname() const { return (_hostname); }
void				ClientState::setHostname(const std::string &newHostname) { _hostname = newHostname; }

const std::string	&ClientState::getServername() const { return (_servername); }
void				ClientState::setServername(const std::string &newServername) { _servername = newServername; }
