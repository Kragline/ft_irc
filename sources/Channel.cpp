#include "Channel.hpp"
#include "Client.hpp"

Channel::Channel(const std::string &name, Client *op) : _name(name),
	_inviteOnly(false), _topicRestricted(false),
	_hasKey(false), _hasLimit(false), _limit(std::numeric_limits<size_t>::max())
{
	addMember(op);
	addOperator(op);
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
	_hasKey = other._hasKey;
	_hasLimit = other._hasLimit;

	_limit = other._limit;
	
	return (*this);
}

Channel::~Channel() {}

std::map<int, Client *>	&Channel::getOperators() { return (_operators); }
std::map<int, Client *>	&Channel::getMembers() { return (_members); }
std::map<int, Client *>	&Channel::getInvited() { return (_invited); }

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

void	Channel::addMember(Client *client)
{
	if (!isMember(client))
		_members.insert(std::make_pair(client->getFd(), client));
}

void	Channel::setNewOperator()
{
	for (std::map<int, Client *>::iterator it = _members.begin(); it != _members.end(); ++it)
	{
		if (!isOperator(it->second))
		{
			_operators.insert(std::make_pair(it->second->getFd(), it->second));
			return ;
		}
	}
}

void	Channel::removeMember(Client *client)
{
	std::map<int, Client *>::iterator	it = _members.find(client->getFd());
	if (it != _members.end())
		_members.erase(it);

	it = _operators.find(client->getFd());
	if (it != _operators.end())
		_operators.erase(it);

	removeInvited(client);
}

bool	Channel::isMember(Client *client)
{
	return (_members.find(client->getFd()) != _members.end());
}

bool	Channel::isEmpty() const
{
	return (_members.empty());
}

void	Channel::addInvited(Client *client)
{
	if (!isInvited(client))
		_invited.insert(std::make_pair(client->getFd(), client));
}

bool	Channel::isInvited(Client *client)
{
	return (_invited.find(client->getFd()) != _invited.end());
}

void	Channel::removeInvited(Client *client)
{
	std::map<int, Client *>::iterator	it = _invited.find(client->getFd());
	if (it != _invited.end())
		_invited.erase(it);
}

void	Channel::addOperator(Client *client)
{
	if (!isOperator(client))
		_operators.insert(std::make_pair(client->getFd(), client));
}

bool	Channel::isOperator(Client *client)
{
	return (_operators.find(client->getFd()) != _operators.end());
}

void	Channel::removeOperator(Client *client)
{
	std::map<int, Client *>::iterator	it = _operators.find(client->getFd());
	if (it != _operators.end())
		_operators.erase(it);
}

size_t	Channel::operatorCount() const { return (_operators.size()); }

size_t	Channel::memberCount() const { return (_members.size()); }

void Channel::broadcast(const std::string &msg, Client *exclude)
{
	for (std::map<int, Client *>::iterator it = _members.begin(); it != _members.end(); ++it)
		if (it->second != exclude)
			it->second->sendMessage(msg);
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
