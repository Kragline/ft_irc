#pragma once

#include "irc.hpp"

class Channel
{
private:
	std::string				_name;
	Client					*_operator;
	std::vector<Client *>	_members;

public:
	Channel(const std::string &name, Client *op);
	Channel(const Channel &other);

	Channel	&operator=(const Channel &other);

	~Channel();

	Client					&getOperator() const ;
	std::vector<Client *>	&getMembers();

    std::string				getName(void) const ;
    void					setName(const std::string &name);

	void	addMember(Client *client);
	void	removeMember(Client *client);
	bool	isMember(Client *client) const ;
	bool	isOperator(Client *client) const ;
	bool	isEmpty() const ;

	void	broadcast(const std::string &message, Client *exclude = NULL);
};
