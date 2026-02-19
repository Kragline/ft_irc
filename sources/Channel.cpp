#include "Channel.hpp"

Channel::Channel(const std::string &name, Client *op) : _name(name), _operator(op)
{
	_members.push_back(op);
}

Channel::Channel(const Channel &other) : _name(other._name), _operator(other._operator), _members(other._members) {}

Channel	&Channel::operator=(const Channel &other)
{
	if (this == &other)
		return (*this);
	
	_operator = other._operator;
	_members = other._members;
	
	return (*this);
}

Channel::~Channel() {}

Client					&Channel::getOperator() const { return (*_operator); }
std::vector<Client *>	&Channel::getMembers() { return (_members); }

std::string				Channel::getName() const { return (_name); }
void					Channel::setName(const std::string &name) { _name = name; }

void	Channel::addMember(Client *client)
{
	if (!isMember(client))
		_members.push_back(client);
}

void	Channel::removeMember(Client *client)
{
	for (std::vector<Client *>::iterator it = _members.begin(); it != _members.end(); it++)
	{
		if (*it == client)
		{
			_members.erase(it);
			return ;
		}
	}
}

bool	Channel::isMember(Client *client) const
{
	for (size_t i = 0; i < _members.size(); i++)
	{
		if (_members[i] == client)
			return (true);
	}
	return (false);
}

bool	Channel::isOperator(Client *client) const
{
	return (client == _operator);
}

void Channel::broadcast(const std::string &msg, Client *exclude)
{
	for (size_t i = 0; i < _members.size(); i++)
	{
		if (_members[i] != exclude)
		{
			send(_members[i]->getFd(), msg.c_str(), msg.size(), 0);
		}
	}
}
