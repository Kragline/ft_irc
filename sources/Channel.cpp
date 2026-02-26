#include "Channel.hpp"

Channel::Channel(const std::string &name, Client *op) : _name(name)
{
	_members.push_back(op);
	_operators.push_back(op);
}

Channel::Channel(const Channel &other) : _name(other._name), _operators(other._operators), _members(other._members) {}

Channel	&Channel::operator=(const Channel &other)
{
	if (this == &other)
		return (*this);
	
	_operators = other._operators;
	_members = other._members;
	
	return (*this);
}

Channel::~Channel() {}

std::vector<Client *>	&Channel::getOperators() { return (_operators); }
std::vector<Client *>	&Channel::getMembers() { return (_members); }

std::string				Channel::getName() const { return (_name); }
void					Channel::setName(const std::string &name) { _name = name; }

std::vector<Client *>::iterator	Channel::_findMember(std::vector<Client *> &vec, Client * client)
{
	return (std::find(vec.begin(), vec.end(), client));
}

void	Channel::addMember(Client *client)
{
	if (!isMember(client))
		_members.push_back(client);
}

void	Channel::removeMember(Client *client)
{
	std::vector<Client *>::iterator it = _findMember(_members, client);
	if (it != _members.end())
		_members.erase(it);

	it = _findMember(_operators, client);
	if (it != _operators.end())
		_operators.erase(it);
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
