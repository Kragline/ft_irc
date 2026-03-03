#include "Channel.hpp"

Channel::Channel(const std::string &name, Client *op) : _name(name),
	_inviteOnly(false), _topicRestricted(false),
	_hasKey(false), _hasLimit(false), _limit(std::numeric_limits<size_t>::max())
{
	_members.push_back(op);
	_operators.push_back(op);
}

Channel::Channel(const Channel &other) : _name(other._name), _key(other._key), _topic(other._topic),
	_operators(other._operators), _members(other._members),
	_invited(other._invited), _inviteOnly(other._inviteOnly),
	_topicRestricted(other._topicRestricted), _hasKey(other._hasKey),
	_hasLimit(other._hasLimit) {}

Channel	&Channel::operator=(const Channel &other)
{
	if (this == &other)
		return (*this);
	
	_operators = other._operators;
	_members = other._members;
	_invited = other._invited;
	
	_name = other._name;
	_key = other._key;
	_topic = other._topic;

	_inviteOnly = other._inviteOnly;
	_topicRestricted = other._topicRestricted;
	_hasKey = other._hasLimit;
	_hasLimit = other._hasLimit;

	_limit = other._limit;
	
	return (*this);
}

Channel::~Channel() {}

std::vector<Client *>	&Channel::getOperators() { return (_operators); }
std::vector<Client *>	&Channel::getMembers() { return (_members); }
std::vector<Client *>	&Channel::getInvited() { return (_invited); }

std::string	Channel::getName() const { return (_name); }
void	Channel::setName(const std::string &name) { _name = name; }

bool	Channel::isInviteOnly() const { return (_inviteOnly == true); }
void	Channel::setInviteOnly(bool status) { _inviteOnly = status; }

bool	Channel::isTopicRestricted() const { return (_topicRestricted == true); }
void	Channel::setTopicRestricted(bool status) { _topicRestricted = status; }

void		Channel::setKey(const std::string &newKey) { _key = newKey; _hasKey = true; }
void		Channel::removeKey() { _key.clear(); _hasKey = false; }
bool		Channel::hasKey() const { return (_hasKey == true); }
std::string	Channel::getKey() const { return (_key); }

void		Channel::setTopic(const std::string &newTopic) { _topic = newTopic; }
std::string	Channel::getTopic() const { return (_topic); }

void	Channel::setLimit(size_t newLimit) { _limit = newLimit; _hasLimit = true; }
void	Channel::removeLimit() { _limit = std::numeric_limits<size_t>::max(); _hasLimit = false; }
bool	Channel::hasLimit() const { return (_hasLimit == true); }
size_t	Channel::getLimit() const { return (_limit); }

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

void	Channel::addOperator(Client *client)
{
	if (!isOperator(client))
		_operators.push_back(client);
}

bool	Channel::isOperator(Client *client)
{
	return (_findMember(_operators, client) != _operators.end());
}

void	Channel::removeOperator(Client *client)
{
	std::vector<Client *>::iterator it = _findMember(_operators, client);
	if (it != _operators.end())
		_operators.erase(it);
}

size_t	Channel::operatorCount() const { return (_operators.size()); }

size_t	Channel::memberCount() const { return (_members.size()); }

void Channel::broadcast(const std::string &msg, Client *exclude)
{
	for (size_t i = 0; i < _members.size(); i++)
		if (_members[i] != exclude)
			_members[i]->sendMessage(msg);
}

std::string	Channel::getModeString() const
{
	std::string	modes = "+";

	if (_inviteOnly) modes += "i";
	if (_topicRestricted) modes += "t";
	if (_hasKey) modes += "k";
	if (_hasLimit) modes += "l";

	return (modes);
}
