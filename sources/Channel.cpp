#include "Channel.hpp"

Channel::Channel(const std::string &name, Client *op) : _name(name), _inviteOnly(false)
{
	_members.push_back(op);
	_operators.push_back(op);
}

Channel::Channel(const Channel &other) : _name(other._name), _operators(other._operators), _members(other._members), _invited(other._invited), _inviteOnly(other._inviteOnly) {}

Channel	&Channel::operator=(const Channel &other)
{
	if (this == &other)
		return (*this);
	
	_operators = other._operators;
	_members = other._members;
	_invited = other._invited;
	
	return (*this);
}

Channel::~Channel() {}

std::vector<Client *>	&Channel::getOperators() { return (_operators); }
std::vector<Client *>	&Channel::getMembers() { return (_members); }
std::vector<Client *>	&Channel::getInvited() { return (_invited); }

std::string				Channel::getName() const { return (_name); }
void					Channel::setName(const std::string &name) { _name = name; }

bool					Channel::isInviteOnly() const { return (_inviteOnly == true); }
void					Channel::setInviteOnly(bool status) { _inviteOnly = status; }

std::vector<Client *>::iterator	Channel::_findMember(std::vector<Client *> &vec, Client * client)
{
	return (std::find(vec.begin(), vec.end(), client));
}

void	Channel::addMember(Client *client)
{
	if (!isMember(client))
		_members.push_back(client);
}

void	Channel::addRandomOperator()
{
	for (size_t i = 0; i < _members.size(); i++)
	{
		if (!isOperator(_members[i]))
		{
			_operators.push_back(_members[i]);
			return ;
		}
	}
}

void	Channel::removeMember(Client *client)
{
	std::vector<Client *>::iterator it = _findMember(_members, client);
	if (it != _members.end())
		_members.erase(it);

	it = _findMember(_operators, client);
	if (it != _operators.end())
		_operators.erase(it);

	removeInvited(client);
}

bool	Channel::isMember(Client *client)
{
	return (_findMember(_members, client) != _members.end());
}

bool	Channel::isEmpty() const
{
	return (_members.empty());
}

bool	Channel::isOperator(Client *client)
{
	return (_findMember(_operators, client) != _operators.end());
}

void	Channel::addInvited(Client *client)
{
	if (!isInvited(client))
		_invited.push_back(client);
}

bool	Channel::isInvited(Client *client)
{
	return (_findMember(_invited, client) != _invited.end());
}

void	Channel::removeInvited(Client *client)
{
	std::vector<Client *>::iterator it = _findMember(_invited, client);
	if (it != _invited.end())
		_invited.erase(it);
}

size_t	Channel::operatorCount() const { return (_operators.size()); }

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
