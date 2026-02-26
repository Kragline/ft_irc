#pragma once

#include "irc.hpp"
#include <algorithm>

class Channel
{
private:
	std::string				_name;
	std::vector<Client *>	_operators;
	std::vector<Client *>	_members;

public:
	Channel(const std::string &name, Client *op);
	Channel(const Channel &other);

	Channel	&operator=(const Channel &other);

	~Channel();

	std::vector<Client *>	&getOperators();
	std::vector<Client *>	&getMembers();

    std::string				getName(void) const ;
	void					setName(const std::string &name);

	void	addMember(Client *client);
	void	addRandomOperator();
	void	removeMember(Client *client);
	bool	isMember(Client *client);
	bool	isOperator(Client *client);
	bool	isEmpty() const ;

	size_t	operatorCount() const ;

	void	broadcast(const std::string &message, Client *exclude = NULL);
private:
	std::vector<Client *>::iterator	_findMember(std::vector<Client *> &vec, Client * client);
};
